# TOFIX

Findings from a code scan on 2026-10-04.

## High

- `lib/except.c:65-68` - the `ioctl` wrapper is declared variadic but calls `p_ioctl(d,request)` without the third argument (the code itself says `BUG!!!`), so every `ioctl` that takes an argument pointer (most of them) is passed garbage once the library is preloaded/linked. Fetch the argument with `va_list`/`va_arg(ap, void*)` and forward it.
- `scripts/build_except.py:35-38` - `test/test_link.elf` and `test/test_linkcc.elf` link `-L. -lexcept[cc]` with no rpath, so they cannot run at all (`./test/test_link.elf` fails with "libexcept.so: cannot open shared object file", verified). Add `-Wl,-rpath,'$ORIGIN/..'` (or equivalent) so the built tests are runnable.

## Medium

- `rsconstruct.toml:27-37` - the four test binaries are built but never executed, and the two `*_nolink` tests are byte-identical to the `*_link` ones (`test/test_nolink.c` == `test/test_link.c`) and only mean anything when run with `LD_PRELOAD=./libexcept.so` (`doc/TODO.txt:6`). Add a step that runs the tests (link tests directly, nolink tests under `LD_PRELOAD`) and checks the expected "malloc"/"ioctl" error exit, so the library is actually exercised in CI.
- `lib/except.c:23` and `lib/except.c:55-58` - the "throw" mode is dead and broken: `throw` is a compile-time `false`, and if flipped, `libexcept.so` (linked only with `-ldl`, `scripts/build_except.py:26`) would reference `except_throw`, which lives only in `libexceptcc.so`, and would throw a C++ exception through C frames compiled without `-fexceptions`. Either remove the throw path from the C library or make it a real, tested feature (build except.c with `-fexceptions`, link libexcept against libexceptcc).
- `lib/except.c:75-81` - the `malloc` interposer calls `p_malloc` which is NULL until the constructor at `lib/except.c:38-43` runs; any allocation from an earlier constructor (or from `dlsym` itself) segfaults. Guard for `p_malloc == NULL` (resolve lazily on first call). `calloc`/`realloc` are also not wrapped, so the "exceptions for malloc failure" behaviour only covers one of the allocation entry points.

## Low

- `lib/except.h:16-19` - the public header is empty (only include guards), so a user who `#include "except.h"` gets no declarations; either declare the API (`except_init`, the wrapped functions) or drop the header and its include in `lib/except.c:9`.
- `lib/except.c:45-53` - commented-out `except_fini` destructor is dead code; delete it.
- `doc/TODO.txt:4-6` - refers to "make run" targets and `sudo make install`, but the Makefile is gone (build is `scripts/build_except.py` via rsconstruct); update or remove the stale items.
- `doc/libexplain-1.4.tar.gz` and `doc/libelf-by-example.pdf` - 5.3 MB of vendored third-party archives committed as "docs"; replace with links in `doc/links.txt` and remove the binaries.
- `doc/links.txt:7-9` - dead links: fedorahosted.org was shut down in 2017 and the sourceforge `apps/trac` URL no longer exists; update or drop them.
- `README.md:1-7` - no build or usage instructions (how to build, `LD_PRELOAD` vs. linking, what gets wrapped); add a short usage section matching `scripts/build_except.py`.
