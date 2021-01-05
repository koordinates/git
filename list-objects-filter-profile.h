#ifndef GIT_LIST_FILTER_PROFILE_H
#define GIT_LIST_FILTER_PROFILE_H

/**
 * The List Filter Profile API can be used to develop filter plugins used for
 * git-upload-pack/git-rev-list/etc.
 *
 * See contrib/profile-filters/README.md for more details and examples.
 *
 * The API defines three functions to implement a filter operation. Note that
 * this is not a stable binary interface, and plugins must be recompiled with
 * git. There is some plumbing in the Makefile to help with this via
 * FILTER_PROFILES.
 *
 * Filter plugins are dynamically loaded via dlopen()/dlsym() at each use.
 *
 * 1. Filter options are parsed, and the config variable
 *    `uploadpackfilter.profile:<name>.plugin` is checked to validate the
 *    plugin name.
 * 2. The plugin library is loaded via `dlopen()` using the config variable
 *    path, as either an absolute path or relative to GIT_EXEC_PATH.
 * 3. git_filter_profile_init() is called.
 * 4. git_filter_profile_object() is called for each object at least once.
 * 5. git_filter_profile_free() is called.
 */

#include "git-compat-util.h"
#include "list-objects-filter.h"
#include "object.h"

/*
 * This is a corollary to `list_objects_filter__init()` and constructs the
 * filter, parsing and validating any user-provided `filter_arg` (via
 * `--filter=profile:<name>=<arg>`). Use `context` for any filter-allocated context data.
 *
 * Return 0 on success and non-zero on error.
 */
int git_filter_profile_init(
	const struct repository *r,
	const char *filter_arg,
	void **context
);

/*
 * This is a corollary to `list_objects_filter__free()`, destroying the filter
 * and any filter-allocated context data.
 */
void git_filter_profile_free(
	const struct repository *r,
	void **context
);

/*
 * This is a corollary to `list_objects_filter__filter_object()`, and
 * decides how to handle the object `obj`.
 *
 * omit provides a flag determining whether to explicitly add or remove
 * the object from any current omitset.
 */
enum list_objects_filter_result git_filter_profile_object(
	const struct repository *r,
	const enum list_objects_filter_situation filter_situation,
	struct object *obj,
	const char *pathname,
	const char *filename,
	enum list_objects_filter_omit *omit,
	void **context
);

#endif /* GIT_LIST_FILTER_PROFILE_H */
