# Faster rebuilds on macOS with ccache

Optional, for developers who build often or keep several worktrees. Get a
normal build working first with the [Mac build guide](CompilingForMac.md).

The `macos-ccache` preset is the `RelWithDebInfo` preset plus
[ccache](https://ccache.dev): compilations are cached in one place for all your
checkouts, so switching branches or setting up another worktree reuses earlier
compiles instead of starting over. It needs ccache 4.8 or newer and Apple Clang.
Other presets and CI are unchanged.

## Set up

```sh
brew install ninja ccache
```

From the repository root, configure with the same options your normal build
uses (for example `CMAKE_PREFIX_PATH` for Qt, or `PKG_CONFIG_PATH` if OpenCV is
installed keg-only), then build:

```sh
cmake -S SerialPrograms --preset macos-ccache \
    -DCMAKE_PREFIX_PATH=/path/to/Qt/6.8.3/macos
cmake --build build/macos-ccache --parallel 4
```

The first build fills the cache and takes as long as a normal build.

## What the preset sets

- **Cache location and size:** `~/Library/Caches/pokemonautomation-ccache`,
  capped at 5 GB; ccache evicts old entries itself. A full build adds about
  250 MB (compressed), so this holds around 20 revisions' worth of compiles.
- **Path rewriting:** `base_dir` is the checkout, so paths inside it are made
  relative and two worktrees produce the same compiler commands.
- **Debug paths:** `-fdebug-prefix-map=<checkout>=.` and
  `-fdebug-compilation-dir=.` keep the checkout's path out of the objects, so
  they can be shared. Debug info then has relative paths: start the debugger
  from the build directory or set its source path.

These are passed on the compiler command line, so no environment variables are
needed. The preset sets `CMAKE_C_FLAGS` and `CMAKE_CXX_FLAGS`; if you need other
flags, add them to these rather than replacing them.

To look at the cache:

```sh
ccache --dir ~/Library/Caches/pokemonautomation-ccache --show-stats
```

## Getting cache hits

Hits need the same compiler, SDK, Qt, dependencies and flags in each worktree.
A change to a widely included header still recompiles everything that includes
it, cached or not.

Measured on Apple Silicon (Apple Clang 21, Qt 6.8.3, ccache 4.14): a second
worktree at the same revision built in 41 s with 1,601 of 1,607 compiles from
the cache, against 13 to 14 minutes uncached.

## Turning it off

Use a normal preset in a separate build directory. Switching presets in the
same directory keeps the launcher and flags stored in `CMakeCache.txt`.
