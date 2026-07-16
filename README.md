# Graham Scan: Dynamic Convex Hull Backends

A C++17 comparison of three data structures for a mutable set of 2D integer
points ordered for Graham scan:

- a sorted `std::vector`
- an AVL tree
- a red-black tree

Each backend supports point insertion and deletion, exposes the current polar
ordering, and computes a convex hull on demand. The executable also contains
correctness checks and a reproducible benchmark suite for comparing runtime and
allocation behavior.

## Requirements

- Clang with C++17 and `__int128` support, GNU Make, and macOS or Linux

The Makefile builds an optimized native binary with `-O3`, `-march=native`,
link-time optimization, warnings, and assertions disabled.

## Quick Start

```bash
make
make test
make benchmark
```

The binary is written to `build/graham-scan`.

| Command | Purpose |
| ------- | ------- |
| `make` | Build the optimized executable |
| `make run` | Run the executable without selecting a workload |
| `make test` | Build and execute all embedded self-tests |
| `make benchmark` | Build and execute the benchmark suite |
| `make clean` | Remove generated build files |

The equivalent direct invocations are:

```bash
build/graham-scan --self-test
build/graham-scan --benchmark
```

Both flags may be combined. Unknown arguments are ignored; no flags produce no
output.

## Public API

The three implementations have the same interface under the `arr`, `avl`, and
`rbt` namespaces:

```cpp
bool insert(Point point);
bool erase(Point point);
std::vector<Point> ordered_points() const;
std::vector<Point> hull(bool include_collinear = false) const;
bool valid() const;
std::size_t size() const;
```

- `insert` adds a unique point and returns `false` for a duplicate.
- `erase` removes a point and returns `false` when it is absent.
- `ordered_points` returns the pivot followed by all other points in polar order.
- `hull` performs a linear scan over that order; collinear boundary points are
  excluded by default and retained when requested.
- `valid` checks backend-specific structural and ordering invariants.
- `size` returns the number of stored points.

`Point` stores signed 64-bit-style integer coordinates as two `long long`
members. Tree implementations own raw nodes and are deliberately non-copyable;
the vector implementation uses normal value semantics.

## How It Works

The pivot is the point with the smallest `(y, x)` pair. Every other point is
represented relative to it by `dx`, `dy`, and squared distance, then ordered by:

1. upper versus lower half-plane
2. cross-product sign
3. squared distance for points on the same ray

Cross products and squared distances use `__int128`, avoiding floating-point
angles and their rounding errors. Hull construction walks the ordered points and
removes the previous point while the newest turn is clockwise, or non-left when
collinear points are excluded.

If an insertion introduces a lower pivot, or the current pivot is deleted, the
backend rebuilds around the new pivot. Ordinary mutations preserve the existing
pivot and update only the selected container.

This project maintains the polar-ordered point set, not the hull edges
incrementally: every `hull()` call scans all stored points.

## Backends

### Sorted Array

`arr::DynamicHull` stores non-pivot elements in a sorted `std::vector`.
`std::lower_bound` locates insertion, deletion, and duplicate positions. It has
compact contiguous storage and cheap traversal, but shifting elements makes
ordinary mutations linear.

### AVL Tree

`avl::DynamicHull` stores height metadata and restores a balance factor in
`[-1, 1]` with single or double rotations after insertion and deletion. In-order
traversal produces the Graham-scan order.

### Red-Black Tree

`rbt::DynamicHull` uses a shared black sentinel, parent links, node colors,
rotations, recoloring, and standard insertion/deletion fix-ups. In-order
traversal produces the same ordering as the other backends.

## Complexity

| Operation | Sorted array | AVL tree | Red-black tree |
| --- | ---: | ---: | ---: |
| Initial build | `O(n²)` | `O(n log n)` | `O(n log n)` |
| Ordinary insert | `O(n)` | `O(log n)` | `O(log n)` |
| Ordinary erase | `O(n)` | `O(log n)` | `O(log n)` |
| Pivot insert/delete | `O(n²)` | `O(n log n)` | `O(n log n)` |
| Ordered-point export | `O(n)` | `O(n)` | `O(n)` |
| Hull query | `O(n)` | `O(n)` | `O(n)` |
| Invariant validation | `O(n)` | `O(n)` | `O(n)` |
| Storage | `O(n)` | `O(n)` | `O(n)` |

The standalone batch Graham scan used as the correctness oracle and benchmark
baseline is `O(n log n)` because it sorts a copy of the input.

## Correctness Checks

`make test` runs embedded checks covering:

- insertion, deletion, duplicates, missing points, and size tracking
- pivot-changing insertion and deletion
- exclusive and inclusive collinear hull behavior
- array ordering, AVL heights/balance, and red-black coloring/black height
- agreement between every backend and the independent batch implementation
- deterministic dataset generation and all 19 benchmark workloads
- summary statistics, report metadata, and allocation tracking

A successful run prints:

```text
All self-tests passed successfully!
```

## Benchmarks

The benchmark uses 10,000 unique pseudorandom points, seed `12648430`
(`0xC0FFEE`), 3 warm-ups, 101 measured samples, and mutation batches of 256
operations. Correctness is verified before results are printed.

The 19 workloads are one batch Graham scan plus six per backend:

- build and hull
- hull query
- ordinary insert
- ordinary delete
- pivot-changing insert
- pivot delete

Each row reports median (`p50`) and nearest-rank `p95` time per operation,
arithmetic mean with sample standard deviation, allocations per operation, and
allocated bytes per operation. Durations automatically use nanoseconds,
microseconds, milliseconds, or seconds. The report also records dataset settings,
compiler, optimization status, operating system, and CPU architecture.

Allocation metrics come from executable-wide overrides of C++ `new` and `new[]`
that count only while a measured operation is running. They do not represent all
possible process or allocator activity.

## Repository Layout

| Path | Responsibility |
| ---- | -------------- |
| `include/geometry.hpp` | Point type, exact orientation test, shared hull scan |
| `include/arr.hpp`, `src/arr.cpp` | Sorted-vector backend |
| `include/avl.hpp`, `src/avl.cpp` | AVL backend |
| `include/rbt.hpp`, `src/rbt.cpp` | Red-black-tree backend |
| `src/main.cpp` | Batch oracle, datasets, tests, benchmarks, reporting, CLI |
| `Makefile` | Optimized build and task automation |

## Scope and Limitations

- Coordinates and intermediate offsets must fit in `long long`; cross-product and
  squared-distance multiplication then use `__int128`.
- The structures are single-threaded and provide no synchronization.
- Pivot changes rebuild the entire backend.
- Hull queries allocate and return a new vector.
- The executable has fixed benchmark settings and no point-input CLI.
- `-march=native` optimizes for the build machine and may reduce binary portability.
