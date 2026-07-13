# Dynamic Convex Hull with Red-Black Tree

A C++17 implementation of a dynamic point set maintaining convex hulls using a Red-Black Tree sorted in Graham-scan order. Points are stored relative to a pivot with exact integer arithmetic (__int128) to avoid floating-point rounding errors.

## Build

### Make

```bash
make
```

### CMake

```bash
cmake -S . -B build/cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build/cmake
```

## Run

### Self-Test
Validates insertion, deletion, pivot changes, hull construction, benchmark statistics, deterministic workload generation, and benchmark correctness:
```bash
build/make/hull --self-test
```

Or run the CMake-registered test:

```bash
ctest --test-dir build/cmake --output-on-failure
```

### Benchmark
Benchmarks 100,000 deterministic unique random points across batch Graham scan, RBT build plus hull, hull query, normal insert/delete, and pivot-changing insert/delete. After three warm-ups, each workload records 101 runs and reports total time, mean with sample standard deviation, min, median, max, p75, p95, p99, and separately measured allocations. Any hull mismatch or invalid red-black tree terminates the run.

```bash
build/make/hull --benchmark
```

## Profiling

### Linux

System-wide performance counters:
```bash
perf stat build/make/hull --benchmark
```

Detailed call graph recording:
```bash
perf record -g build/make/hull --benchmark
perf report
```

Memory usage profiling:
```bash
valgrind --tool=massif build/make/hull --benchmark
ms_print massif.out.* | head -100
```

### macOS

Time profiler (CPU cycles):
```bash
xcrun xctrace record --template 'Time Profiler' --launch -- build/make/hull --benchmark
```

Memory allocation profiler:
```bash
xcrun xctrace record --template Allocations --launch -- build/make/hull --benchmark
```

## Architecture

- **Pivot Management**: First inserted point becomes pivot; all others stored relative to pivot as (dx, dy) offset from pivot
- **Ordering**: Graham-scan polar order with 3-tier comparator: (1) upper-half plane test, (2) cross product sign, (3) squared distance
- **Invariants**: Black-root, red-parent rule, equal black-height across all paths
- **Exact Geometry**: `__int128` cross products and squared distances prevent floating-point rounding
- **Lazy Rebuild**: When new point would become pivot (y,x) < current pivot (y,x), all n points are re-inserted O(n log n)

## Files

- `include/rbt.hpp`: Public interface (Point, DynamicHull class, Color enum)
- `src/rbt.cpp`: RBT implementation (rotations, rebalancing, insertion, deletion, validation)
- `src/main.cpp`: Self-tests, baseline Graham scan, benchmark harness, allocation tracking
- `Makefile`: Make build producing `build/make/hull`
- `CMakeLists.txt`: CMake build and CTest registration producing `build/cmake/hull`

## Performance Characteristics

- **Insert**: O(log n) normal, O(n log n) when pivot changes
- **Delete**: O(log n) normal, O(n log n) when deleting pivot
- **Hull**: O(n) after points are in sorted order
- **Space**: O(n) for n points plus O(1) overhead per node

## Implementation Notes

- Sentinel nil_ node simplifies boundary handling in rotations
- Node stores: Point, dx, dy, squared distance (__int128), color, parent/left/right pointers
- Both baseline and RBT implementations use identical Graham-scan stack algorithm
- Canonicalization (rotation to minimum (y,x) point) enables deterministic hull comparison
