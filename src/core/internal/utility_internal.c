#include "anvil/memory/internal/utility_internal.h"
#include <assert.h>
#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/mman.h>
#include <unistd.h>

void log_and_crash(const char *expr, const char *file, int line, const char *msg) {
	// Get the current time
	time_t now = time(NULL);
	struct tm *tm_info = localtime(&now);
	char time_buf[20];
	strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_info);

#ifdef LOG_FILE
	FILE *log = fopen(LOG_FILE, "a");
	if (log) {
		fprintf(log, "[%s] INVARIANT failed: %s at %s:%d\nMessage: %s\n\n", time_buf, expr, file, line, msg);
		fclose(log);
	}
#else
	fprintf(stderr, "INVARIANT failed: %s at %s:%d\nMessage: %s\n", expr, file, line, msg);
#endif /* ifdef LOG_FILE */

	abort();
}

bool is_power_of_two(const size_t x) {
	return (x != 0) && ((x & (x - 1)) == 0);
}

void *safe_aligned_alloc(size_t size, size_t alignment, const char *error_msg) {
	INVARIANT(size != 0, "Cannot allocate zero bytes");
	INVARIANT(is_power_of_two(alignment), "Alignment must be a power of two");
	INVARIANT(alignment <= (1 << 16), "Alignment exceeds limit");

	size_t page_size = (size_t)sysconf(_SC_PAGESIZE);
	size_t total_size = size + alignment + sizeof(Metadata);

	total_size = (total_size + page_size - 1) & ~(page_size - 1);

	void *base = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	INVARIANT(base != MAP_FAILED, error_msg);

	uintptr_t addr = (uintptr_t)base + sizeof(Metadata);
	uintptr_t aligned_addr = (addr + alignment - 1) & ~(alignment - 1);

	Metadata *metadata = (Metadata *)(aligned_addr - sizeof(Metadata));
	metadata->base = base;
	metadata->total_size = total_size;

	return (void *)aligned_addr;
}

void safe_free(void *ptr) {

	free((*(void **)ptr));
}

void safe_aligned_free(void *ptr) {
	if (!ptr)
		return;

	Metadata *metadata = (Metadata *)((uintptr_t)ptr - sizeof(Metadata));

	if (metadata->base != NULL) {
		munmap(metadata->base, metadata->total_size);
	}
}
