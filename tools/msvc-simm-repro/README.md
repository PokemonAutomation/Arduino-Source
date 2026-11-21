# MSVC SIMM repro

`msvc_simm_repro.cpp` is a standalone minimal repro of the CI failure

```
Kernels::ScaleInvariantMatrixMatch - x86_SSE(w=512, h=513, weights)
Message: compute_scale: default=-0.00225342088 isa=0.000351705559
```

It computes `sum(A*T) / sum(A*A)` over a 512x513 float matrix three ways:

- `ref`: double precision scalar (ground truth),
- `deflt`: the `Default` kernel (`Context_Default`, float scalar lanes),
- `sse`: the x86 SSE kernel (`Context_x86_SSE41`, 128-bit lanes),

and exits 0 when `deflt` and `sse` are within the CI test's tolerance
(4096 ULPs or 1e-4 x magnitude) of `ref`.

## Fidelity to the CI code

- The `SumATA2` template, both contexts, the `PartialWordAccess_x64_SSE41`
  loads and `reduce32_x64_SSE` are verbatim copies from
  `SerialPrograms/Source/Kernels/ScaleInvariantMatrixMatch/`.
- The data is generated exactly like the failing CI test
  (`Kernels_ScaleInvariantMatrixMatch_TestInfra.cpp`): the same seed
  (the weighted (512, 513) instance), the same `std::mt19937` /
  `std::uniform_real_distribution` stream, with the W draw per element
  skipped. On MSVC the data is therefore bit-identical to the data of
  the failing CI test, and `deflt` should print the same value CI prints
  for `default` (in one observed run: `-0.00225342088`). On gcc/clang the
  data differs (the standard library's `uniform_real_distribution`
  mapping is not standardized), which only shifts the absolute values.
- The rows are 64-byte aligned at runtime, as in CI (`GuardPageBuffer`
  places this size on a 64-byte boundary), so the partial-row paths are
  dead at runtime - but the row pointers are rerouted through a `volatile`
  so the compiler cannot see the alignment, and the partial-row paths
  stay live in the binary, as they do in the CI build.

## Build and run

MSVC, with the CI's flags (RelWithDebInfo, no debug info):

```
cl /nologo /std:c++23 /O2 /Ob1 /MD /DNDEBUG /W4 /WX /MP /utf-8 ^
   tools\msvc-simm-repro\msvc_simm_repro.cpp /Fe:repro.exe
repro.exe
```

To bisect the flag set, rebuild without `/MP`:

```
cl /nologo /std:c++23 /O2 /Ob1 /MD /DNDEBUG /W4 /WX /utf-8 ^
   tools\msvc-simm-repro\msvc_simm_repro.cpp /Fe:repro_nom.exe
repro_nom.exe
```

A conforming compiler prints `PASS` and exits 0. The MSVC build that
misbehaves in CI is expected to print `FAIL`, naming which of `deflt` /
`sse` diverges from `ref`.

Sanity check on a non-MSVC toolchain (must PASS):

```
g++ -std=c++23 -O2 tools/msvc-simm-repro/msvc_simm_repro.cpp -o repro
./repro
```

## Running it in CI (optional)

Add a scratch workflow (`.github/workflows/msvc-simm-repro.yml`) that
checks out the repo, runs the "Set up MSVC environment (Windows)" step
from `.github/workflows/cpp-ci-serial-programs-base.yml`, and then runs
the two `cl` commands above, one per step. The first step of that
environment setup prints the `cl` version, which is part of what this
investigation needs.

## Reading the result

- FAIL on the exact-flags build, PASS without `/MP`: the `/MP`
  parallel-compile flag is involved.
- FAIL on both: the miscompilation does not need `/MP`; the repro (or a
  reduced version of it) is the basis for an MSVC bug report.
- PASS on both: the divergence needs more of the CI context (the other
  kernel instantiations in the same TU, the weighted paths, the other
  TUs); extend the repro accordingly.
