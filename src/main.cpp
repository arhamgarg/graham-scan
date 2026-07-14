#include "../include/avl.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

namespace {

std::vector<Point> baselineGrahamScan(const std::vector<Point>& input) {
    if (input.empty()) {
        return {};
    }

    Point pivot = *std::min_element(input.begin(), input.end(), [](const Point& lhs, const Point& rhs) {
        return std::tie(lhs.y, lhs.x) < std::tie(rhs.y, rhs.x);
    });

    std::vector<Point> ordered;
    ordered.reserve(input.size());
    ordered.push_back(pivot);

    std::vector<Point> others;
    others.reserve(input.size() - 1);
    for (const Point& point : input) {
        if (point == pivot) {
            continue;
        }
        others.push_back(point);
    }

    std::sort(others.begin(), others.end(), [&](const Point& lhs, const Point& rhs) {
        const Point lhsRel{lhs.x - pivot.x, lhs.y - pivot.y};
        const Point rhsRel{rhs.x - pivot.x, rhs.y - pivot.y};
        const bool lhsUpper = lhsRel.y > 0 || (lhsRel.y == 0 && lhsRel.x > 0);
        const bool rhsUpper = rhsRel.y > 0 || (rhsRel.y == 0 && rhsRel.x > 0);
        if (lhsUpper != rhsUpper) {
            return lhsUpper;
        }
        const auto crossValue = (__int128)lhsRel.x * rhsRel.y - (__int128)lhsRel.y * rhsRel.x;
        if (crossValue != 0) {
            return crossValue > 0;
        }
        return (__int128)lhsRel.x * lhsRel.x + (__int128)lhsRel.y * lhsRel.y <
               (__int128)rhsRel.x * rhsRel.x + (__int128)rhsRel.y * rhsRel.y;
    });

    ordered.insert(ordered.end(), others.begin(), others.end());
    if (ordered.size() < 3) {
        return ordered;
    }

    std::vector<Point> stack;
    stack.push_back(ordered.front());
    for (std::size_t i = 1; i < ordered.size(); ++i) {
        const Point& candidate = ordered[i];
        while (stack.size() >= 2) {
            const Point& first = stack[stack.size() - 2];
            const Point& second = stack.back();
            const auto orientation = (__int128)(second.x - first.x) * (candidate.y - first.y) -
                                    (__int128)(second.y - first.y) * (candidate.x - first.x);
            if (orientation > 0) {
                break;
            }
            stack.pop_back();
        }
        stack.push_back(candidate);
    }

    return stack;
}

bool sameHull(const std::vector<Point>& lhs, const std::vector<Point>& rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }

    std::vector<Point> leftCopy = lhs;
    std::vector<Point> rightCopy = rhs;
    std::sort(leftCopy.begin(), leftCopy.end(), [](const Point& a, const Point& b) {
        if (a.x != b.x) {
            return a.x < b.x;
        }
        return a.y < b.y;
    });
    std::sort(rightCopy.begin(), rightCopy.end(), [](const Point& a, const Point& b) {
        if (a.x != b.x) {
            return a.x < b.x;
        }
        return a.y < b.y;
    });

    for (std::size_t i = 0; i < leftCopy.size(); ++i) {
        if (leftCopy[i] != rightCopy[i]) {
            return false;
        }
    }
    return true;
}

void runSelfTest() {
    const std::vector<std::vector<Point>> testCases = {
        {{0, 0}, {2, 0}, {3, 1}, {2, 2}, {1, 3}, {0, 2}, {-1, 1}},
        {{5, 5}, {1, 1}, {3, 3}, {0, 0}, {4, 2}, {2, 4}},
        {{0, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 0}, {0, 5}},
        {{-2, 0}, {0, 2}, {2, 0}, {0, -2}, {1, 1}, {-1, 1}}
    };

    for (std::size_t index = 0; index < testCases.size(); ++index) {
        DynamicHull hull;
        for (const Point& point : testCases[index]) {
            hull.insert(point);
        }

        const std::vector<Point> expected = baselineGrahamScan(testCases[index]);
        const std::vector<Point> actual = hull.hull();
        if (!sameHull(expected, actual)) {
            std::cerr << "Self-test case " << index << " failed.\n";
            std::cerr << "Expected hull:";
            for (const Point& point : expected) {
                std::cerr << " (" << point.x << ", " << point.y << ")";
            }
            std::cerr << "\nActual hull:";
            for (const Point& point : actual) {
                std::cerr << " (" << point.x << ", " << point.y << ")";
            }
            std::cerr << "\n";
            throw std::runtime_error("failed");
        }
    }

    std::cout << "Self-test passed.\n";
}

struct BenchStats {
    std::vector<int64_t> samples;

    void record(int64_t ns) {
        samples.push_back(ns);
    }

    int64_t total() const {
        int64_t sum = 0;
        for (int64_t s : samples) {
            sum += s;
        }
        return sum;
    }

    double mean() const {
        return samples.empty() ? 0.0 : static_cast<double>(total()) / samples.size();
    }

    double stdDev() const {
        if (samples.size() < 2) {
            return 0.0;
        }
        const double m = mean();
        double var = 0.0;
        for (int64_t s : samples) {
            const double diff = s - m;
            var += diff * diff;
        }
        return std::sqrt(var / (samples.size() - 1));
    }

    int64_t min() const {
        return samples.empty() ? 0 : *std::min_element(samples.begin(), samples.end());
    }

    int64_t max() const {
        return samples.empty() ? 0 : *std::max_element(samples.begin(), samples.end());
    }

    int64_t percentile(double p) const {
        if (samples.empty()) {
            return 0;
        }
        std::vector<int64_t> sorted = samples;
        std::sort(sorted.begin(), sorted.end());
        const std::size_t idx = static_cast<std::size_t>((p / 100.0) * (sorted.size() - 1));
        return sorted[idx];
    }

    int64_t median() const {
        return percentile(50.0);
    }

    int64_t p75() const {
        return percentile(75.0);
    }

    int64_t p95() const {
        return percentile(95.0);
    }

    int64_t p99() const {
        return percentile(99.0);
    }
};

void printBenchmarkHeader() {
    std::cout << "\nbenchmark                runs    total           mean +- sd           min      median     max       p75       p95       p99\n";
    std::cout << "------------------------  -----  ------         --------           ---      ------     ---       ---       ---       ---\n";
}

void printBenchmarkRow(const std::string& name, const BenchStats& stats) {
    const int64_t totalNs = stats.total();
    const double meanNs = stats.mean();
    const double stdNs = stats.stdDev();

    auto formatNs = [](int64_t ns) {
        if (ns < 1000) {
            char buf[32];
            snprintf(buf, sizeof(buf), "%ldns", ns);
            return std::string(buf);
        }
        if (ns < 1000000) {
            char buf[32];
            snprintf(buf, sizeof(buf), "%ld.%03ldus", ns / 1000, (ns % 1000));
            return std::string(buf);
        }
        if (ns < 1000000000) {
            char buf[32];
            snprintf(buf, sizeof(buf), "%ld.%03ldms", ns / 1000000, (ns % 1000000) / 1000);
            return std::string(buf);
        }
        char buf[32];
        snprintf(buf, sizeof(buf), "%ld.%03lds", ns / 1000000000, (ns % 1000000000) / 1000000);
        return std::string(buf);
    };

    printf("%-24s %5zu  %13s  %8.3fms +- %.3fms  %10s  %10s  %10s  %10s  %10s  %10s\n",
           name.c_str(),
           stats.samples.size(),
           formatNs(totalNs).c_str(),
           meanNs / 1e6,
           stdNs / 1e6,
           formatNs(stats.min()).c_str(),
           formatNs(stats.median()).c_str(),
           formatNs(stats.max()).c_str(),
           formatNs(stats.p75()).c_str(),
           formatNs(stats.p95()).c_str(),
           formatNs(stats.p99()).c_str());
}

void runBenchmark() {
    const int DATASET_SIZE = 10000;
    const int WARM_UPS = 3;
    const int MEASURED_RUNS = 101;

    std::mt19937 rng(12648430);
    std::uniform_int_distribution<int> dist(-100000, 100000);

    std::vector<Point> points;
    points.reserve(DATASET_SIZE);
    for (int i = 0; i < DATASET_SIZE; ++i) {
        int x = dist(rng);
        int y = dist(rng);
        points.push_back({x, y});
    }

    std::cout << "\nDataset size: " << DATASET_SIZE << " unique random points\n";
    std::cout << "Seed: " << 12648430 << "\n";
    std::cout << "Warm-ups: " << WARM_UPS << "\n";
    std::cout << "Measured runs: " << MEASURED_RUNS << "\n";
    std::cout << "Compiler: GCC\n";
    std::cout << "Optimization: enabled\n";
    std::cout << "Platform: Linux x64\n";

    printBenchmarkHeader();

    // Batch Graham scan (baseline)
    BenchStats batchStats;
    for (int w = 0; w < WARM_UPS; ++w) {
        baselineGrahamScan(points);
    }
    for (int r = 0; r < MEASURED_RUNS; ++r) {
        const auto start = std::chrono::steady_clock::now();
        const auto result = baselineGrahamScan(points);
        const auto end = std::chrono::steady_clock::now();
        const auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        batchStats.record(elapsed);
    }
    printBenchmarkRow("Batch Graham scan", batchStats);

    // AVL build + hull
    BenchStats buildStats;
    for (int w = 0; w < WARM_UPS; ++w) {
        DynamicHull h;
        for (const Point& p : points) {
            h.insert(p);
        }
        h.hull();
    }
    for (int r = 0; r < MEASURED_RUNS; ++r) {
        const auto start = std::chrono::steady_clock::now();
        DynamicHull h;
        for (const Point& p : points) {
            h.insert(p);
        }
        const auto result = h.hull();
        const auto end = std::chrono::steady_clock::now();
        const auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        buildStats.record(elapsed);
    }
    printBenchmarkRow("AVL build + hull", buildStats);

    // AVL hull query
    DynamicHull prebuilt;
    for (const Point& p : points) {
        prebuilt.insert(p);
    }
    BenchStats queryStats;
    for (int w = 0; w < WARM_UPS; ++w) {
        prebuilt.hull();
    }
    for (int r = 0; r < MEASURED_RUNS; ++r) {
        const auto start = std::chrono::steady_clock::now();
        const auto result = prebuilt.hull();
        const auto end = std::chrono::steady_clock::now();
        const auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        queryStats.record(elapsed);
    }
    printBenchmarkRow("AVL hull query", queryStats);

    // AVL normal insert
    DynamicHull forInsert;
    const int INSERT_BASE = 100;
    for (int i = 0; i < INSERT_BASE && i < static_cast<int>(points.size()); ++i) {
        forInsert.insert(points[i]);
    }
    BenchStats insertStats;
    for (int w = 0; w < WARM_UPS; ++w) {
        for (int i = INSERT_BASE; i < INSERT_BASE + 10 && i < static_cast<int>(points.size()); ++i) {
            forInsert.insert(points[i]);
        }
    }
    int insertIdx = INSERT_BASE;
    for (int r = 0; r < MEASURED_RUNS; ++r) {
        const auto start = std::chrono::steady_clock::now();
        if (insertIdx < static_cast<int>(points.size())) {
            forInsert.insert(points[insertIdx++]);
        }
        const auto end = std::chrono::steady_clock::now();
        const auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        insertStats.record(elapsed);
    }
    printBenchmarkRow("AVL normal insert", insertStats);

    // AVL normal delete
    DynamicHull forDelete;
    const std::vector<Point> deleteSubset(points.begin(), points.begin() + (INSERT_BASE + 101));
    for (const Point& p : deleteSubset) {
        forDelete.insert(p);
    }
    BenchStats deleteStats;
    const std::vector<Point> deleteItems(deleteSubset.begin() + INSERT_BASE, deleteSubset.end());
    for (int w = 0; w < WARM_UPS; ++w) {
        for (int i = 0; i < 10 && i < static_cast<int>(deleteItems.size()); ++i) {
            forDelete.remove(deleteItems[i]);
        }
    }
    int deleteIdx = 0;
    for (int r = 0; r < MEASURED_RUNS; ++r) {
        const auto start = std::chrono::steady_clock::now();
        if (deleteIdx < static_cast<int>(deleteItems.size())) {
            forDelete.remove(deleteItems[deleteIdx++]);
        }
        const auto end = std::chrono::steady_clock::now();
        const auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        deleteStats.record(elapsed);
    }
    printBenchmarkRow("AVL normal delete", deleteStats);

    // AVL pivot insert
    DynamicHull forPivotInsert;
    Point minPoint = points[0];
    for (const Point& p : points) {
        if (std::tie(p.y, p.x) < std::tie(minPoint.y, minPoint.x)) {
            minPoint = p;
        }
    }
    for (const Point& p : points) {
        if (p != minPoint) {
            forPivotInsert.insert(p);
            break;
        }
    }
    BenchStats pivotInsertStats;
    std::vector<Point> pivotCandidates;
    for (const Point& p : points) {
        if (std::tie(p.y, p.x) < std::tie(minPoint.y, minPoint.x)) {
            pivotCandidates.push_back(p);
        }
    }
    for (int w = 0; w < WARM_UPS; ++w) {
        if (!pivotCandidates.empty()) {
            forPivotInsert.insert(pivotCandidates[0]);
        }
    }
    int pivotIdx = 0;
    for (int r = 0; r < MEASURED_RUNS; ++r) {
        const auto start = std::chrono::steady_clock::now();
        if (pivotIdx < static_cast<int>(pivotCandidates.size())) {
            forPivotInsert.insert(pivotCandidates[pivotIdx]);
        }
        const auto end = std::chrono::steady_clock::now();
        const auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        pivotInsertStats.record(elapsed);
        pivotIdx++;
        if (pivotIdx >= static_cast<int>(pivotCandidates.size())) {
            pivotIdx = 0;
        }
    }
    printBenchmarkRow("AVL pivot insert", pivotInsertStats);

    // AVL pivot delete
    DynamicHull forPivotDelete;
    for (const Point& p : points) {
        forPivotDelete.insert(p);
    }
    BenchStats pivotDeleteStats;
    for (int w = 0; w < WARM_UPS; ++w) {
        DynamicHull temp;
        for (const Point& p : points) {
            temp.insert(p);
        }
        if (!temp.empty()) {
            temp.remove(temp.points()[0]);
        }
    }
    for (int r = 0; r < MEASURED_RUNS; ++r) {
        const auto start = std::chrono::steady_clock::now();
        if (!forPivotDelete.empty()) {
            const Point pivotToDelete = forPivotDelete.points()[0];
            forPivotDelete.remove(pivotToDelete);
            if (!forPivotDelete.empty()) {
                forPivotDelete.insert(pivotToDelete);
            } else {
                for (const Point& p : points) {
                    forPivotDelete.insert(p);
                    break;
                }
            }
        }
        const auto end = std::chrono::steady_clock::now();
        const auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        pivotDeleteStats.record(elapsed);
    }
    printBenchmarkRow("AVL pivot delete", pivotDeleteStats);

    std::cout << "\n";
}

}  // namespace

int main(int argc, char** argv) {
    const std::string mode = argc > 1 ? argv[1] : "--self-test";
    if (mode == "--self-test") {
        runSelfTest();
        return 0;
    }
    if (mode == "--benchmark") {
        runBenchmark();
        return 0;
    }

    std::cerr << "Usage: hull [--self-test|--benchmark]\n";
    return 1;
}
