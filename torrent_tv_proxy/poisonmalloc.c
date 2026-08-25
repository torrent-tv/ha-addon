/*
 * poisonmalloc.c — self-arming trap for heap corruption (diagnostic).
 *
 * Roadmap item 1 / research/worker-thread-drain-crash-2026-08-22.md: six
 * identical SIGSEGV deaths whose teardown faults in uv_timer_stop over memory
 * some module had already freed or overwritten earlier, during normal work.
 * Absence-based experiments cannot name the guilty module; this interceptor
 * makes the guilty instruction name itself.
 *
 * Policy:
 *   - Small allocations (<= FENCE_MAX bytes) and large ones (>= FENCE_MIN)
 *     are placed on their own pages. On free() the pages become PROT_NONE
 *     and the block sits in a bounded FIFO quarantine. ANY later access —
 *     read or write — faults IMMEDIATELY at the offending instruction, and
 *     the core dump maps that PC to a shared object through NT_FILE.
 *   - Everything else passes through to libc, poisoned (0xA5-filled) on
 *     free so use-after-free reads at least see recognizable garbage.
 *
 * Cost: quarantined pages are held out of reuse (QUARANTINE_BYTES cap),
 * fenced classes pay page granularity and two mprotect syscalls per free.
 * This is a diagnostic build for the field host, not a general allocator.
 *
 * Build: gcc -shared -fPIC -O2 -o poisonmalloc.so poisonmalloc.c -lpthread
 * Use:   LD_PRELOAD=/usr/lib/poisonmalloc.so <program>
 */

#define _GNU_SOURCE
#include <dlfcn.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>

#define PAGE_SIZE 4096u
#define HDR_MAGIC 0x5015F0EDu

/* Classes: [1 .. FENCE_MAX] fenced-small, [FENCE_MIN .. inf) fenced-large,
 * the gap passes through poisoned. */
#define FENCE_MAX 1024u
#define FENCE_MIN (64u * 1024u)

/* Freed fenced blocks kept PROT_NONE before the oldest is really freed. */
#define QUARANTINE_BYTES (128u * 1024u * 1024u)
#define QUARANTINE_SLOTS 8192u

#define POISON_BYTE 0xA5
#define ALIGNMENT 16u

struct header {
	uint32_t magic;
	uint32_t size;   /* requested size */
	void *base;      /* pointer returned by the real allocator */
};

struct quarantined {
	void *base;
	void *payload;
	size_t span;
};

static void *(*real_malloc)(size_t);
static void (*real_free)(void *);
static void *(*real_calloc)(size_t, size_t);
static void *(*real_realloc)(void *, size_t);

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static struct quarantined ring[QUARANTINE_SLOTS];
static size_t ring_head;      /* next slot to fill */
static size_t ring_count;     /* entries currently held */
static size_t ring_bytes;
static int resolved;

/* Bootstrap arena for the handful of allocations that may happen before the
 * real symbols are resolved (musl's dlsym does not allocate, but be safe). */
static char boot_arena[64 * 1024];
static size_t boot_used;

static void die(const char *what)
{
	static const char msg[] = "poisonmalloc: fatal\n";
	(void)write(2, msg, sizeof(msg) - 1);
	(void)what;
	_exit(134);
}

static size_t align_up(size_t value, size_t alignment)
{
	return (value + alignment - 1) & ~(alignment - 1);
}

static void resolve(void)
{
	real_malloc = dlsym(RTLD_NEXT, "malloc");
	real_free = dlsym(RTLD_NEXT, "free");
	real_calloc = dlsym(RTLD_NEXT, "calloc");
	real_realloc = dlsym(RTLD_NEXT, "realloc");
	if (!real_malloc || !real_free || !real_calloc || !real_realloc) {
		die("resolve");
	}
	resolved = 1;
}

void *malloc(size_t size)
{
	if (!resolved) {
		resolve();
	}
	int fenced = size != 0 && (size <= FENCE_MAX || size >= FENCE_MIN);

	if (!fenced) {
		size_t total = size + ALIGNMENT;
		char *raw = real_malloc(total ? total : 1);
		if (!raw) {
			return NULL;
		}
		uint32_t *slot = (uint32_t *)raw;
		slot[0] = 0;             /* not fenced */
		slot[1] = (uint32_t)size;
		void *payload = raw + ALIGNMENT;
		memset(payload, POISON_BYTE, size);
		return payload;
	}

	size_t span = align_up(sizeof(struct header) + size, PAGE_SIZE);
	size_t total = span + PAGE_SIZE; /* slack so payload can start on a page */
	void *base = real_malloc(total);
	if (!base) {
		return NULL;
	}
	uintptr_t raw = (uintptr_t)base;
	uintptr_t payload = align_up(raw + sizeof(struct header), PAGE_SIZE);
	struct header *hdr = (struct header *)(payload - sizeof(struct header));
	hdr->magic = HDR_MAGIC;
	hdr->size = (uint32_t)size;
	hdr->base = base;
	memset((void *)payload, POISON_BYTE, size);
	return (void *)payload;
}

static void quarantine_push(void *base, void *payload, size_t span)
{
	while (ring_count >= QUARANTINE_SLOTS ||
	       (ring_bytes + span > QUARANTINE_BYTES && ring_count > 0)) {
		struct quarantined *oldest = &ring[(ring_head - ring_count) % QUARANTINE_SLOTS];
		mprotect(oldest->payload, oldest->span, PROT_READ | PROT_WRITE);
		real_free(oldest->base);
		ring_bytes -= oldest->span;
		ring_count--;
	}
	ring[ring_head % QUARANTINE_SLOTS].base = base;
	ring[ring_head % QUARANTINE_SLOTS].payload = payload;
	ring[ring_head % QUARANTINE_SLOTS].span = span;
	ring_head++;
	ring_count++;
	ring_bytes += span;
}

static void fenced_free(void *payload, uint32_t size, void *base)
{
	size_t span = align_up((size_t)size, PAGE_SIZE);
	if (span == 0) {
		span = PAGE_SIZE;
	}
	if (mprotect(payload, span, PROT_NONE) != 0) {
		real_free(base);
		return;
	}
	pthread_mutex_lock(&lock);
	quarantine_push(base, payload, span);
	pthread_mutex_unlock(&lock);
}

void free(void *ptr)
{
	if (!ptr) {
		return;
	}
	if (!resolved) {
		resolve();
	}
	char *raw = (char *)ptr - ALIGNMENT;
	uint32_t kind = ((uint32_t *)raw)[0];
	uint32_t size = ((uint32_t *)raw)[1];

	if (kind == HDR_MAGIC) {
		struct header *hdr = (struct header *)((char *)ptr - sizeof(struct header));
		/* Payload is page-aligned; header sits right before it. */
		fenced_free(ptr, size, hdr->base);
		return;
	}
	if (kind == 0) {
		/* Plain allocation: poison so later reads see the pattern. */
		memset(ptr, POISON_BYTE, size);
		real_free(raw);
		return;
	}
	/* Foreign pointer (allocated before preload engaged) — leave it be. */
	real_free(ptr);
}

void *calloc(size_t count, size_t size)
{
	if (!resolved) {
		resolve();
	}
	size_t total = count * size;
	if (count != 0 && total / count != size) {
		errno = ENOMEM;
		return NULL;
	}
	void *ptr = malloc(total);
	if (ptr) {
		memset(ptr, 0, total);
	}
	return ptr;
}

void *realloc(void *ptr, size_t size)
{
	if (!resolved) {
		resolve();
	}
	if (!ptr) {
		return malloc(size);
	}
	if (size == 0) {
		free(ptr);
		return NULL;
	}
	char *raw = (char *)ptr - ALIGNMENT;
	uint32_t old_size = ((uint32_t *)raw)[1];
	if (((uint32_t *)raw)[0] != HDR_MAGIC && ((uint32_t *)raw)[0] != 0) {
		return real_realloc(ptr, size);
	}
	void *fresh = malloc(size);
	if (!fresh) {
		return NULL;
	}
	memcpy(fresh, ptr, size < old_size ? size : old_size);
	free(ptr);
	return fresh;
}
