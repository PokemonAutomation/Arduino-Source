# MSVC SIMM probe

`msvc_simm_repro.cpp` is a minimal instrumented probe for the CI-only
failure

```
Kernels::ScaleInvariantMatrixMatch - x86_SSE(w=512, h=513, ...)
Message: compute_scale: default=... isa=... (out of tolerance)
```

It computes `s = sum(A*T) / sum(A*A)` over a 512x513 float matrix with
the same data stream as the failing CI test (same seed and
`std::mt19937` / `std::uniform_real_distribution` sequence as the
weighted (512, 513) instance, W draw skipped; on MSVC the A/T values
are bit-identical to the CI test's), and logs the intermediate steps so
a failing run localizes the divergence:

1. `compiler: MSVC _MSC_VER=... _MSC_FULL_VER=...` - the exact compiler
   build of the runner. Failing and passing pool runs must be
   distinguishable here.
2. a data dump (`sum(A)`, `sum(T)`, first values with bit patterns) -
   check the stream is what the platform's `<random>` produces, and is
   stable across runs. Also the row addresses and their 16/64-byte
   alignment (0 in CI).
3. every row's `ref` (double) / `def` (scalar kernel) / `sse` (SSE
   kernel), each from one-row calls of the verbatim kernels - the first
   diverging row (if any).
4. the SSE kernel's row-reduced `sum_at` / `sum_as` lanes for the first
   rows, the diverging rows and the last row - lane `j` holds the
   contributions of the elements whose index is `j mod 16`, so a wrong
   lane localizes the corruption to a quarter of the row stride.
5. the full-matrix result, exactly like CI, as the verdict (exit 0/1).

The kernels are verbatim copies of the CI sources
(`Kernels_ScaleInvariantMatrixMatch_Routines.h`, `Core_Default.cpp`,
`Core_x86_SSE.cpp`, `Kernels_x64_SSE41.h`,
`Kernels_PartialWordAccess_x64_SSE41.h`). The rows are 64-byte aligned
at runtime, as in CI, so the partial-row paths are dead; the row
pointers are rerouted through a `volatile` so the compiler keeps those
paths live in the binary, as in the CI build. The lane dump uses a
second copy of the loop with the same body (instrumentation only: its
values localize the bug, the verdict uses the verbatim kernel).

## Build and run

MSVC, with the CI's flags:

```
cl /nologo /std:c++23 /O2 /Ob1 /MD /DNDEBUG /W4 /WX /MP /utf-8 /external:anglebrackets ^
   tools\msvc-simm-repro\msvc_simm_repro.cpp /Fe:probe.exe
probe.exe
```

and, for flag bisection, the same without `/MP`. A conforming compiler
prints PASS and exits 0; a misbehaving build prints the diverging rows
and lanes and FAILs (exit 1), which also keeps the CI job red with the
log captured.

Sanity check on a non-MSVC toolchain (must PASS):

```
g++ -std=c++23 -O2 tools/msvc-simm-repro/msvc_simm_repro.cpp -o probe
./probe
```

## Running it in CI

Copy `msvc-simm-repro.yml` to `.github/workflows/msvc-simm-repro.yml`
and trigger it manually (workflow_dispatch). It sets up the MSVC
environment (printing the runner's `cl` version), then builds and runs
the probe with the exact CI flags and again without `/MP`.

## Reading the result

- `compiler:` differs between failing and passing CI runs → the pool
  rotation is confirmed and the version is known; the repro (or a
  reduced version of it) is the basis for an MSVC bug report.
- the data dump differs from a passing run → the data generation
  itself is the problem, not the kernel.
- some rows diverge → the bug is in the per-row loop; the lane dump
  says which quarter. All rows agree but the total diverges → the bug
  is in the accumulation across rows (the `sum_AT = vadd(sum_AT, ...)`
  update).
- FAIL with `/MP`, PASS without → `/MP` is involved.
- PASS on both → the divergence needs more CI context (the other
  instantiations in the TU, the weighted paths, other TUs); extend the
  probe accordingly.
