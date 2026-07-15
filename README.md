# Dynamic Convex Hull with AVL and Red-Black Trees

A C++17 implementation of a dynamic point set maintaining convex hulls using both **AVL** and **Red-Black** trees sorted in Graham-scan order. Points are stored relative to a pivot with exact integer arithmetic (`__int128`) to avoid floating-point rounding errors. 

Both data structures are integrated side-by-side to allow comparative performance profiling and allocation tracking.

## Build

### Make

```bash
make
```

### CMake

```bash
cmake -S . -B build/cmake
cmake --build build/cmake
```

## Run

### Self-Test

Validates insertion, deletion, pivot changes, and hull construction invariants for both AVL and RBT structures:

```bash
build/make/hull --self-test
```

Or run via CMake:

```bash
ctest --test-dir build/cmake --output-on-failure
```

### Benchmark

Benchmarks 13 workload types across batch Graham scan, RBT/AVL build + hull, RBT/AVL hull queries, normal insert/delete, and pivot-changing insert/delete. After three warm-ups, each workload records 101 runs and reports total time, mean with sample standard deviation, min, median, max, p75, p95, and p99, alongside allocation/memory metrics:

```bash
build/make/hull --benchmark
```

## Architecture

- **Pivot Management**: First inserted point becomes pivot; all others stored relative to pivot as (dx, dy) offset
- **Ordering**: Graham-scan polar order with 3-tier comparator: (1) upper-half plane test, (2) cross product sign, (3) squared distance
- **Balancing Schemes**:
  - **AVL Tree**: Uses height tracking, balance factor calculation, and single/double rotations.
  - **Red-Black Tree**: Uses node coloring (RED/BLACK), parent tracking, and restoration rotations during insertions/deletions.
- **Exact Geometry**: `__int128` cross products and squared distances prevent floating-point rounding
- **Lazy Rebuild**: When a new point becomes the pivot (y,x) < current pivot (y,x), all $n$ points are re-inserted in $O(n \log n)$ time.

## Files

- `include/avl.hpp` / `src/avl.cpp`: AVL tree implementation (rotations, rebalancing, insertion, deletion)
- `include/rbt.hpp` / `src/rbt.cpp`: Red-Black tree implementation (rotations, color fixups, insertion, deletion)
- `src/main.cpp`: Self-tests, baseline Graham scan, benchmark harness running both implementations side-by-side
- `Makefile`: Make build producing `build/make/hull`
- `CMakeLists.txt`: CMake build and CTest registration producing `build/cmake/hull`

## Performance Characteristics

- **Insert**: $O(\log n)$ normal, $O(n \log n)$ when pivot changes
- **Delete**: $O(\log n)$ normal, $O(n \log n)$ when deleting pivot
- **Hull Query**: $O(n)$ after points are sorted (retrieval via Graham scan stack)
- **Space**: $O(n)$ for $n$ points. AVL nodes track height, while RBT nodes track parent pointers and color.
