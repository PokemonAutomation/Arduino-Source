# MSVC SIMM repros

Repros for the CI-only failure

```
Kernels::ScaleInvariantMatrixMatch - x86_SSE(w=512, h=513, ...)
Message: compute_scale: default=-0.00225342088 isa=0.000351705559
(1959531328 ulps, abs 0.002605 vs tolerance 0.000100)
```

Both files use the exact data stream of the failing test (512 x 513,
the weighted instance's seed, per element the A, T and W values drawn
in the test's order - the W draw is discarded by the unweighted
repros but must happen, to keep the `std::mt19937` stream aligned). On
MSVC the A/T values are bit-identical to the CI test's.

**Status:** the instrumented suite probe (see the PR "Kernels:
instrumented SIMM probe as the only test") localized the failure: on
the `windows-2025-vs2026` runner image (MSVC 19.52, VS 2026 18.0) the
**scalar Default kernel** miscompiles - every row diverges - while the
SSE kernel's results are correct and bit-identical to MSVC 19.43's
(VS 2022).

## msvc_simm_default_repro.cpp

The smallest repro: verbatim copies of `Context_Default`, the
unweighted `SumATA2` and the unweighted `compute_scale<>` (from
`Core_Default.cpp` / `Routines.h`), the CI data, and a
double-precision reference. Prints the compiler version, the
diverging rows (if any) and the full-matrix result; exits 1 if the
Default result diverges beyond the CI tolerance. Expected to PASS on
gcc/clang and MSVC 19.43, and to FAIL on MSVC 19.52 with
`total: default=-0.00225342088` - the exact value from the original
CI failure message. This is the basis for the MSVC bug report.

## msvc_simm_repro.cpp

The fuller probe: Default and SSE kernels (verbatim, with the
partial-row paths kept live), a double reference, per-row and
full-matrix values for both, and the SSE row-reduced lane dump. It
prints which kernel diverges. Under MSVC 19.52 it is expected to FAIL
naming the **default** kernel.

## Build and run

MSVC, with the CI's flags:

```
cl /nologo /std:c++23 /O2 /Ob1 /MD /DNDEBUG /W4 /WX /MP /utf-8 /external:anglebrackets ^
   tools\msvc-simm-repro\msvc_simm_default_repro.cpp /Fe:default_repro.exe
default_repro.exe
```

and, for flag bisection, the same without `/MP`.

Sanity check on a non-MSVC toolchain (must PASS):

```
g++ -std=c++23 -O2 tools/msvc-simm-repro/msvc_simm_default_repro.cpp -o default_repro
./default_repro
```

`msvc-simm-repro.yml` (copy to `.github/workflows/` to trigger
manually) builds and runs both repros in CI with the exact flags, so
they can be run against a specific runner pool without a local install
of VS 2026.
