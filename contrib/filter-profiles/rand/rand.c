#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../../../list-objects-filter-profile.h"
#include "../../../hash.h"
#include "../../../trace.h"


#define EXPORT __attribute__((visibility("default")))

static struct trace_key trace_filter = TRACE_KEY_INIT(FILTER);

struct filter_context {
	int percentageMatch;
	int matchCount;
	int blobCount;
	int treeCount;
	struct timespec started_at;
};

EXPORT int git_filter_profile_init(const struct repository *r,
				   const char *filter_arg,
				   void **context)
{
	struct filter_context *ctx = calloc(1, sizeof(struct filter_context));

	ctx->percentageMatch = atoi(filter_arg);
	if (ctx->percentageMatch > 100 || ctx->percentageMatch < 0) {
	fprintf(stderr, "filter-rand: warning: invalid match %%: %s\n",
		filter_arg);
	ctx->percentageMatch = 1;  // default 1%
	}
	fprintf(stderr, "filter-rand: matching %d%%\n", ctx->percentageMatch);
	clock_gettime(CLOCK_MONOTONIC, &ctx->started_at);
	(*context) = ctx;

	return 0;
}

EXPORT enum list_objects_filter_result git_filter_profile_object(
		const struct repository *r,
		const enum list_objects_filter_situation filter_situation,
		struct object *obj,
		const char *pathname,
		const char *filename,
		enum list_objects_filter_omit *omit,
		void **context)
{
	struct filter_context *ctx = (struct filter_context*)(*context);

	if ((ctx->blobCount + ctx->treeCount + 1) % 100000 == 0) {
		fprintf(stderr, "filter-rand: %d...\n",
			(ctx->blobCount + ctx->treeCount + 1));
	}

	switch (filter_situation) {
	default:
		fprintf(stderr,
			"filter-rand: unknown filter_situation: %d\n",
			filter_situation);
		abort();

	case LOFS_BEGIN_TREE:
		ctx->treeCount++;
		/* always include all tree objects */
		return LOFR_MARK_SEEN | LOFR_DO_SHOW;

	case LOFS_END_TREE:
		return LOFR_ZERO;

	case LOFS_BLOB:
		ctx->blobCount++;

		if ((rand() % 100) < ctx->percentageMatch) {
			ctx->matchCount++;
			trace_printf_key(&trace_filter,
				"match: %s %s\n",
				oid_to_hex(&obj->oid),
				pathname
			);
			return LOFR_MARK_SEEN | LOFR_DO_SHOW;
		} else {
			*omit = LOFO_OMIT;
			return LOFR_MARK_SEEN; /* hard omit */
		}
	}
}

EXPORT void git_filter_profile_free(const struct repository *r, void **context)
{
	struct filter_context *ctx = (struct filter_context*)(*context);

	struct timespec ended_at;
	clock_gettime(CLOCK_MONOTONIC, &ended_at);

	int count = ctx->blobCount + ctx->treeCount;
	double elapsed = (ended_at.tv_sec - ctx->started_at.tv_sec)
		+ (ended_at.tv_nsec - ctx->started_at.tv_nsec)/1E9;

	fprintf(stderr, "filter-rand: done: count=%d (blob=%d tree=%d) "
		"matched=%d elapsed=%fs rate=%0.1f/s average=%0.1fus\n",
		count, ctx->blobCount, ctx->treeCount, ctx->matchCount,
		elapsed, count/elapsed, elapsed/count*1E6);

	free(ctx);
}
