#include <iomanip>
#include <iostream>
#include <sstream>

#include <time.h>

extern "C" {
	#include "../../../list-objects-filter-extensions.h"
	#include "adapter_functions.h"
}

struct rand_context {
	int percentageMatch = 0;
	int matchCount = 0;
	int blobCount = 0;
	int treeCount = 0;
	struct timespec started_at;
};

static int rand_init(
	const struct repository *r,
	const char *filter_arg,
	void **context)
{
	struct rand_context *ctx = new rand_context();

	ctx->percentageMatch = atoi(filter_arg);
	if (ctx->percentageMatch > 100 || ctx->percentageMatch < 0) {
		std::cerr << "filter-rand-cpp: warning: invalid match %: " << filter_arg << "\n";
		ctx->percentageMatch = 1;  // default 1%
	}
	std::cerr << "filter-rand-cpp: matching " << ctx->percentageMatch << "%\n";
	clock_gettime(CLOCK_MONOTONIC, &ctx->started_at);
	(*context) = ctx;

	return 0;
}

enum list_objects_filter_result rand_filter_object(
	const struct repository *r,
	const enum list_objects_filter_situation filter_situation,
	struct object *obj,
	const char *pathname,
	const char *filename,
	enum list_objects_filter_omit *omit,
	void *context)
{
	struct rand_context *ctx = static_cast<struct rand_context*>(context);

	if ((ctx->blobCount + ctx->treeCount + 1) % 100000 == 0) {
		std::cerr << "filter-rand-cpp: " << (ctx->blobCount + ctx->treeCount + 1) << "...\n";
	}
	switch (filter_situation) {
	default:
		std::cerr << "filter-rand-cpp: unknown filter_situation: " << filter_situation << "\n";
		abort();

	case LOFS_BEGIN_TREE:
		ctx->treeCount++;
		/* always include all tree objects */
		return static_cast<list_objects_filter_result>(LOFR_MARK_SEEN | LOFR_DO_SHOW);

	case LOFS_END_TREE:
		return LOFR_ZERO;

	case LOFS_BLOB:
		ctx->blobCount++;

		if ((rand() % 100) < ctx->percentageMatch) {
			ctx->matchCount++;
			std::cout << "match: " << obj_to_hex_oid(obj) << pathname << "\n";
			return static_cast<list_objects_filter_result>(LOFR_MARK_SEEN | LOFR_DO_SHOW);
		} else {
			*omit = LOFO_OMIT;
			return LOFR_MARK_SEEN; /* but not LOFR_DO_SHOW (hard omit) */
		}
	}
}

void rand_free(const struct repository *r, void *context) {
	struct rand_context *ctx = static_cast<struct rand_context*>(context);

	struct timespec ended_at;
	clock_gettime(CLOCK_MONOTONIC, &ended_at);

	int count = ctx->blobCount + ctx->treeCount;
	double elapsed = (ended_at.tv_sec - ctx->started_at.tv_sec)
		+ (ended_at.tv_nsec - ctx->started_at.tv_nsec)/1E9;

	std::cerr << "filter-rand-cpp: done: count=" << count
		<< " (blob=" << ctx->blobCount << " tree=" << ctx->treeCount << ")"
		<< " matched=" << ctx->matchCount
		<< " elapsed=" << elapsed << "s"
		<< " rate=" << count/elapsed << "/s"
		<< " average=" << elapsed/count*1E6 << "us\n";

	delete ctx;
}

extern const struct filter_extension filter_extension_rand_cpp = {
	"rand_cpp",
	&rand_init,
	&rand_filter_object,
	&rand_free,
};
