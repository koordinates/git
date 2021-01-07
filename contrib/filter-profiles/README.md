Custom List Filter Profiles
===========================

This API can be used to develop filter plugins used for custom filtering
behaviour with `git-upload-pack` and `git-rev-list`.

**This is not a stable binary interface, and plugins must be compiled alongside
Git**. Filter plugins are dynamically loaded via `dlopen()`/`dlsym()` at each
use. The API defines three required functions to implement a filter operation.

When invoking `--filter=profile:<name>[=<arg>]`, the following steps occur:

1. Filter options are parsed, and the config variable
`uploadpackfilter.profile:<name>.plugin` is checked to validate the
plugin is defined. For a fetch, this validation happens when the filter is
actually being applied (ie: as part of git-upload-pack).
2. The plugin library is loaded via `dlopen()` using the config variable
value as either an absolute path, or relative to `GIT_EXEC_PATH`.
3. `git_filter_profile_init()` is called, parsing and validating any `<arg>`
and initialising any filter context.
4. `git_filter_profile_object()` is called for each object at least once.
5. `git_filter_profile_free()` is called to cleanup any context.

## Examples

**[`rand`](./rand/)** is a filter that matches all trees and a random percentage
of blobs, where the percentage is parsed from the filter arg. It imports and
uses the `oid_to_hex()` and `trace_key_printf()` functions from the Git API.

Build via:

```console
$ echo FILTER_PROFILES=contrib/filter-profiles/rand >> config.mak
$ make
    ...
    SUBDIR contrib/filter-profiles/rand
    ...
```

We can run against git's own repo:

```console
$ ./git config --local uploadpackfilter.profile:rand.plugin $(pwd)/contrib/filter-profiles/rand/rand.so
$ ./git rev-list refs/heads/master --objects --max-count 1 --filter=profile:rand=3 --filter-print-omitted | grep -c '^~'
filter-rand: matching 3%
filter-rand: done: count=3958 (blob=3760 tree=198) matched=114 elapsed=0.007338s rate=539384.0/s average=1.9us
3646
```

**[`rand-cpp`](./rand-cpp/)** is the same as `rand` but implemented in C++17.

## Development

See the examples for a basic implementation. The comments in
[`list-objects-filter.h`](../../list-objects-filter.h) and the built-in filter
implementations in [`list-objects-filter.c`](../../list-objects-filter.c) are
important to understand how filters are implemented - `filter_blobs_limit()`
provides a simple example, and `filter_sparse()` is more complex.

The API differences between the built-in filters and the profile plugins:

1. Profile filters don't handle `omitset`s directly, instead setting `omit`.
2. Profile filters receive a void pointer they can use for context.

## Building

There is some plumbing in the Git Makefile to help with this via
`FILTER_PROFILES`, setting it to space-separated paths of the profiles to build,
and ensures Git itself is linked correctly for plugin integration. For example,
to build both the example plugins together:

```console
FILTER_PROFILES="contrib/filter-profiles/rand contrib/filter-profiles/rand-cpp" make
```

Filter profile paths don't need to be within the Git source tree. Profiles
expect a Makefile with `all`, `clean`, and `install` targets. Several useful
variables from the main Git Makefile are exported. See the examples for details.

If you want to use any library functions from Git, you will need to set your
plugin linker to resolve symbols from the main `git` executable. The examples
do this.

You can develop plugins with C++, but many Git header files are not compatible
with modern C++, so you'll need to avoid using library functions from Git. At a
minimum you'll need to wrap `extern "C" {}` around
`#include "list-objects-filter-profile.h"`. See [`rand-cpp`](./rand-cpp/) for a
simple example.

For other languages you'll need to port definitions of some internal Git
structs. At a minimum, `object`, `object_id`, `repository`, and `hash_algo`.
