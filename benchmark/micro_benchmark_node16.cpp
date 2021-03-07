// Copyright 2020-2021 Laurynas Biveinis

#include "global.hpp"

#include <benchmark/benchmark.h>

#include "art.hpp"
#include "micro_benchmark_node_utils.hpp"
#include "micro_benchmark_utils.hpp"
#include "mutex_art.hpp"
#include "olc_art.hpp"

namespace {

template <class Db>
void grow_node4_to_node16_sequentially(benchmark::State &state) {
  unodb::benchmark::grow_node_sequentially_benchmark<Db, 4>(state);
}

template <class Db>
void grow_node4_to_node16_randomly(benchmark::State &state) {
  unodb::benchmark::grow_node_randomly_benchmark<Db, 4>(state);
}

template <class Db>
void node16_sequential_add(benchmark::State &state) {
  unodb::benchmark::sequential_add_benchmark<Db, 16>(state);
}

template <class Db>
void node16_random_add(benchmark::State &state) {
  unodb::benchmark::random_add_benchmark<Db, 16>(state);
}

template <class Db>
void minimal_node16_tree_full_scan(benchmark::State &state) {
  unodb::benchmark::minimal_tree_full_scan<Db, 16>(state);
}

template <class Db>
void minimal_node16_tree_random_gets(benchmark::State &state) {
  unodb::benchmark::minimal_tree_random_gets<Db, 16>(state);
}

template <class Db>
void full_node16_tree_full_scan(benchmark::State &state) {
  unodb::benchmark::full_node_scan_benchmark<Db, 16>(state);
}

template <class Db>
void full_node16_tree_random_gets(benchmark::State &state) {
  unodb::benchmark::full_node_random_get_benchmark<Db, 16>(state);
}

template <class Db>
void full_node16_tree_sequential_delete(benchmark::State &state) {
  unodb::benchmark::sequential_delete_benchmark<Db, 16>(state);
}

template <class Db>
void full_node16_tree_random_delete(benchmark::State &state) {
  unodb::benchmark::random_delete_benchmark<Db, 16>(state);
}

template <class Db>
void shrink_node48_to_node16_sequentially(benchmark::State &state) {
  unodb::benchmark::shrink_node_sequentially_benchmark<Db, 16>(state);
}

template <class Db>
void shrink_node48_to_node16_randomly(benchmark::State &state) {
  unodb::benchmark::shrink_node_randomly_benchmark<Db, 16>(state);
}

}  // namespace

BENCHMARK_TEMPLATE(grow_node4_to_node16_sequentially, unodb::db)
    ->Range(20, 16383)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(grow_node4_to_node16_sequentially, unodb::mutex_db)
    ->Range(20, 16383)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(grow_node4_to_node16_sequentially, unodb::olc_db)
    ->Range(20, 16383)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_TEMPLATE(grow_node4_to_node16_randomly, unodb::db)
    ->Range(20, 16383)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(grow_node4_to_node16_randomly, unodb::mutex_db)
    ->Range(20, 16383)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(grow_node4_to_node16_randomly, unodb::olc_db)
    ->Range(20, 16383)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_TEMPLATE(node16_sequential_add, unodb::db)
    ->Range(10, 16383)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(node16_sequential_add, unodb::mutex_db)
    ->Range(10, 16383)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(node16_sequential_add, unodb::olc_db)
    ->Range(10, 16383)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_TEMPLATE(node16_random_add, unodb::db)
    ->Range(10, 16383)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(node16_random_add, unodb::mutex_db)
    ->Range(10, 16383)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(node16_random_add, unodb::olc_db)
    ->Range(10, 16383)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_TEMPLATE(minimal_node16_tree_full_scan, unodb::db)
    ->Range(10, 16383)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(minimal_node16_tree_full_scan, unodb::mutex_db)
    ->Range(10, 16383)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(minimal_node16_tree_full_scan, unodb::olc_db)
    ->Range(10, 16383)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_TEMPLATE(minimal_node16_tree_random_gets, unodb::db)
    ->Range(10, 16383)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(minimal_node16_tree_random_gets, unodb::mutex_db)
    ->Range(10, 16383)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(minimal_node16_tree_random_gets, unodb::olc_db)
    ->Range(10, 16383)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_TEMPLATE(full_node16_tree_full_scan, unodb::db)
    ->Range(64, 246000)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(full_node16_tree_full_scan, unodb::mutex_db)
    ->Range(64, 246000)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(full_node16_tree_full_scan, unodb::olc_db)
    ->Range(64, 246000)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_TEMPLATE(full_node16_tree_random_gets, unodb::db)
    ->Range(64, 24600)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(full_node16_tree_random_gets, unodb::mutex_db)
    ->Range(64, 24600)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(full_node16_tree_random_gets, unodb::olc_db)
    ->Range(64, 24600)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_TEMPLATE(full_node16_tree_sequential_delete, unodb::db)
    ->Range(64, 246000)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(full_node16_tree_sequential_delete, unodb::mutex_db)
    ->Range(64, 246000)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(full_node16_tree_sequential_delete, unodb::olc_db)
    ->Range(64, 246000)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_TEMPLATE(full_node16_tree_random_delete, unodb::db)
    ->Range(64, 246000)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(full_node16_tree_random_delete, unodb::mutex_db)
    ->Range(64, 246000)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(full_node16_tree_random_delete, unodb::olc_db)
    ->Range(64, 246000)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_TEMPLATE(shrink_node48_to_node16_sequentially, unodb::db)
    ->Range(4, 16383)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(shrink_node48_to_node16_sequentially, unodb::mutex_db)
    ->Range(4, 16383)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(shrink_node48_to_node16_sequentially, unodb::olc_db)
    ->Range(4, 16383)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_TEMPLATE(shrink_node48_to_node16_randomly, unodb::db)
    ->Range(4, 16383)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(shrink_node48_to_node16_randomly, unodb::mutex_db)
    ->Range(4, 16383)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_TEMPLATE(shrink_node48_to_node16_randomly, unodb::olc_db)
    ->Range(4, 16383)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_MAIN();
