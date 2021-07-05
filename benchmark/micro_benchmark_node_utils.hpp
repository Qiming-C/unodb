// Copyright 2019-2021 Laurynas Biveinis
#ifndef UNODB_DETAIL_MICRO_BENCHMARK_NODE_UTILS_HPP
#define UNODB_DETAIL_MICRO_BENCHMARK_NODE_UTILS_HPP

#include "global.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#ifndef NDEBUG
#include <iostream>
#endif
#include <limits>
#include <random>
#include <string>
#include <tuple>
#include <vector>

#include <benchmark/benchmark.h>

#include "art_common.hpp"
#include "micro_benchmark_utils.hpp"
#include "node_type.hpp"

namespace unodb::benchmark {

// Key manipulation with key zero bits

template <unsigned NodeSize>
constexpr auto node_size_to_key_zero_bits() noexcept {
  static_assert(NodeSize == 2 || NodeSize == 4 || NodeSize == 16 ||
                NodeSize == 256);
  if constexpr (NodeSize == 2) {
    return 0xFEFEFEFE'FEFEFEFEULL;
  } else if constexpr (NodeSize == 4) {
    return 0xFCFCFCFC'FCFCFCFCULL;
  } else if constexpr (NodeSize == 16) {
    return 0xF0F0F0F0'F0F0F0F0ULL;
  }
  return 0ULL;
}

constexpr auto next_key(unodb::key k, std::uint64_t key_zero_bits) noexcept {
  assert((k & key_zero_bits) == 0);

  const auto result = ((k | key_zero_bits) + 1) & ~key_zero_bits;

  assert(result > k);
  assert((result & key_zero_bits) == 0);

  return result;
}

// PRNG

class batched_prng final {
  using result_type = std::uint64_t;

 public:
  explicit batched_prng(
      result_type max_value = std::numeric_limits<result_type>::max())
      : random_key_dist{0ULL, max_value} {
    refill();
  }

  auto get(::benchmark::State &state) {
    if (random_key_ptr == random_keys.cend()) {
      state.PauseTiming();
      refill();
      state.ResumeTiming();
    }
    return *(random_key_ptr++);
  }

 private:
  void refill() {
    std::generate(random_keys.begin(), random_keys.end(),
                  [this]() { return random_key_dist(get_prng()); });
    random_key_ptr = random_keys.cbegin();
  }

  static constexpr auto random_batch_size = 10000;

  std::vector<result_type> random_keys{random_batch_size};
  decltype(random_keys)::const_iterator random_key_ptr;

  std::uniform_int_distribution<result_type> random_key_dist;
};

namespace detail {

// Node sizes

template <unsigned NodeCapacity>
constexpr auto node_capacity_to_minimum_size() noexcept {
  static_assert(NodeCapacity == 16 || NodeCapacity == 48 ||
                NodeCapacity == 256);
  if constexpr (NodeCapacity == 16) {
    return 5;
  } else if constexpr (NodeCapacity == 48) {
    return 17;
  }
  return 49;
}

template <unsigned NodeCapacity>
constexpr auto node_capacity_over_minimum() noexcept {
  static_assert(NodeCapacity == 16 || NodeCapacity == 48 ||
                NodeCapacity == 256);
  return NodeCapacity - node_capacity_to_minimum_size<NodeCapacity>();
}

template <unsigned NodeSize>
constexpr auto node_size_has_key_zero_bits() noexcept {
  // If node size is a power of two, then can use key zero bit-based operations
  return (NodeSize & (NodeSize - 1)) == 0;
}

template <unsigned NodeSize>
constexpr auto node_size_to_node_type() noexcept {
  static_assert(NodeSize == 2 || NodeSize == 4 || NodeSize == 16 ||
                NodeSize == 48 || NodeSize == 256);
  if constexpr (NodeSize == 2 || NodeSize == 4) return node_type::I4;
  if constexpr (NodeSize == 16) return node_type::I16;
  if constexpr (NodeSize == 48) return node_type::I48;
  return node_type::I256;
}

#ifndef NDEBUG

template <unsigned SmallerNodeSize>
constexpr auto node_size_to_larger_node_type() noexcept {
  static_assert(SmallerNodeSize == 4 || SmallerNodeSize == 16 ||
                SmallerNodeSize == 48);
  if constexpr (SmallerNodeSize == 4) return node_type::I16;
  if constexpr (SmallerNodeSize == 16) return node_type::I48;
  return node_type::I256;
}

#endif  // NDEBUG

// Key manipulation

template <unsigned B, unsigned S, unsigned O>
constexpr auto to_scaled_base_n_value(std::uint64_t i) noexcept {
  assert(i / (static_cast<std::uint64_t>(B) * B * B * B * B * B * B) < B);
  return (i % B * S + O) | (i / B % B * S + O) << 8U |
         ((i / (B * B) % B * S + O) << 16U) |
         ((i / (B * B * B) % B * S + O) << 24U) |
         ((i / (static_cast<std::uint64_t>(B) * B * B * B) % B * S + O)
          << 32U) |
         ((i / (static_cast<std::uint64_t>(B) * B * B * B * B) % B * S + O)
          << 40U) |
         ((i / (static_cast<std::uint64_t>(B) * B * B * B * B * B) % B * S + O)
          << 48U) |
         ((i / (static_cast<std::uint64_t>(B) * B * B * B * B * B * B) % B * S +
           O)
          << 56U);
}

template <unsigned B>
constexpr auto to_base_n_value(std::uint64_t i) noexcept {
  assert(i / (static_cast<std::uint64_t>(B) * B * B * B * B * B * B) < B);
  return to_scaled_base_n_value<B, 1, 0>(i);
}

template <unsigned NodeSize>
constexpr auto number_to_full_node_size_tree_key(std::uint64_t i) noexcept {
  return to_base_n_value<NodeSize>(i);
}

template <unsigned NodeSize>
constexpr auto number_to_minimal_node_size_tree_key(std::uint64_t i) noexcept {
  return to_base_n_value<node_capacity_to_minimum_size<NodeSize>()>(i);
}

template <unsigned NodeSize>
constexpr auto number_to_full_leaf_over_minimal_tree_key(
    std::uint64_t i) noexcept {
  constexpr auto min = node_capacity_to_minimum_size<NodeSize>();
  constexpr auto delta = node_capacity_over_minimum<NodeSize>();
  assert(i / (delta * min * min * min * min * min * min) < min);
  return ((i % delta) + min) |
         number_to_minimal_node_size_tree_key<NodeSize>(i / delta) << 8U;
}

template <unsigned NodeSize>
constexpr auto number_to_minimal_leaf_over_smaller_node_tree(
    std::uint64_t i) noexcept {
  constexpr auto N = static_cast<std::uint64_t>(NodeSize);
  assert(i / (N * N * N * N * N * N) < N);
  return N | number_to_full_node_size_tree_key<N>(i) << 8U;
}

template <unsigned NodeSize>
constexpr auto number_to_full_node_tree_with_gaps_key(
    std::uint64_t i) noexcept {
  static_assert(NodeSize == 4 || NodeSize == 16 || NodeSize == 48);
  // Full Node4 tree keys with 1, 3, 5, & 7 as the different key byte values
  // so that a new byte could be inserted later at any position:
  // 0x0101010101010101 to ...107
  // 0x0101010101010301 to ...307
  // 0x0101010101010501 to ...507
  // 0x0101010101010701 to ...707
  // 0x0101010101030101 to ...107
  // 0x0101010101030301 to ...307
  // Full Node16 tree keys with 1, 3, 5, ..., 33 as the different key byte
  // values so that a new byte could be inserted later at any position.
  return to_scaled_base_n_value<NodeSize, 2, 1>(i);
}

// Key vectors

template <typename NumberToKeyFn>
auto generate_keys_to_limit(unodb::key key_limit,
                            NumberToKeyFn number_to_key_fn) {
  std::vector<unodb::key> result;
  std::uint64_t i = 0;
  while (true) {
    const auto key = number_to_key_fn(i);
    if (key > key_limit) break;
    ++i;
    result.push_back(key);
  }
  return result;
}

template <std::uint8_t NumByteValues>
std::vector<unodb::key> generate_random_keys_over_full_smaller_tree(
    unodb::key key_limit) {
  // The last byte at the limit will be randomly-generated and may happen to
  // fall above or below the limit. Reset the limit so that any byte value will
  // pass.
  key_limit |= 0xFFU;
  std::uniform_int_distribution<std::uint8_t> prng_byte_values{0,
                                                               NumByteValues};

  std::vector<unodb::key> result;
  union {
    std::uint64_t as_int;
    std::array<std::uint8_t, 8> as_bytes;
  } key;

  for (std::uint8_t i = 0; i < NumByteValues; ++i) {
    key.as_bytes[7] = static_cast<std::uint8_t>(i * 2 + 1);
    for (std::uint8_t i2 = 0; i2 < NumByteValues; ++i2) {
      key.as_bytes[6] = static_cast<std::uint8_t>(i2 * 2 + 1);
      for (std::uint8_t i3 = 0; i3 < NumByteValues; ++i3) {
        key.as_bytes[5] = static_cast<std::uint8_t>(i3 * 2 + 1);
        for (std::uint8_t i4 = 0; i4 < NumByteValues; ++i4) {
          key.as_bytes[4] = static_cast<std::uint8_t>(i4 * 2 + 1);
          for (std::uint8_t i5 = 0; i5 < NumByteValues; ++i5) {
            key.as_bytes[3] = static_cast<std::uint8_t>(i5 * 2 + 1);
            for (std::uint8_t i6 = 0; i6 < NumByteValues; ++i6) {
              key.as_bytes[2] = static_cast<std::uint8_t>(i6 * 2 + 1);
              for (std::uint8_t i7 = 0; i7 < NumByteValues; ++i7) {
                key.as_bytes[1] = static_cast<std::uint8_t>(i7 * 2 + 1);
                key.as_bytes[0] = static_cast<std::uint8_t>(
                    prng_byte_values(unodb::benchmark::get_prng()) * 2);

                const unodb::key k = key.as_int;
                if (k > key_limit) {
                  result.shrink_to_fit();
                  std::shuffle(result.begin(), result.end(),
                               unodb::benchmark::get_prng());
                  return result;
                }
                result.push_back(k);
              }
            }
          }
        }
      }
    }
  }
  UNODB_DETAIL_CANNOT_HAPPEN();
}

}  // namespace detail

// Stats

template <class Db>
struct tree_stats final {
  constexpr tree_stats() noexcept = default;

  explicit constexpr tree_stats(const Db &test_db) noexcept
      : node_counts{test_db.get_node_counts()},
        growing_inode_counts{test_db.get_growing_inode_counts()},
        key_prefix_splits{test_db.get_key_prefix_splits()} {}

  constexpr void get(const Db &test_db) noexcept {
    node_counts = test_db.get_node_counts();
    growing_inode_counts = test_db.get_growing_inode_counts();
    key_prefix_splits = test_db.get_key_prefix_splits();
  }

  constexpr bool operator==(const tree_stats<Db> &other) const noexcept {
    return node_counts == other.node_counts;
  }

  constexpr bool internal_levels_equal(
      const tree_stats<Db> &other) const noexcept {
    return node_counts[unodb::as_i<unodb::node_type::I4>] ==
               other.node_counts[unodb::as_i<unodb::node_type::I4>] &&
           node_counts[unodb::as_i<unodb::node_type::I16>] ==
               other.node_counts[unodb::as_i<unodb::node_type::I16>] &&
           node_counts[unodb::as_i<unodb::node_type::I48>] ==
               other.node_counts[unodb::as_i<unodb::node_type::I48>] &&
           node_counts[unodb::as_i<unodb::node_type::I256>] ==
               other.node_counts[unodb::as_i<unodb::node_type::I256>] &&
           growing_inode_counts == other.growing_inode_counts &&
           key_prefix_splits == other.key_prefix_splits;
  }

  node_type_counter_array node_counts;
  inode_type_counter_array growing_inode_counts;
  std::uint64_t key_prefix_splits{0};
};

template <class Db>
class growing_tree_node_stats final {
 public:
  constexpr void get(const Db &test_db) noexcept {
    stats.get(test_db);
#ifndef NDEBUG
    get_called = true;
    db = &test_db;
#endif
  }

  constexpr void publish(::benchmark::State &state) const noexcept {
    assert(get_called);
    state.counters["L"] = static_cast<double>(
        stats.node_counts[::unodb::as_i<unodb::node_type::LEAF>]);
    state.counters["4"] = static_cast<double>(
        stats.node_counts[::unodb::as_i<unodb::node_type::I4>]);
    state.counters["16"] = static_cast<double>(
        stats.node_counts[::unodb::as_i<unodb::node_type::I16>]);
    state.counters["48"] = static_cast<double>(
        stats.node_counts[::unodb::as_i<unodb::node_type::I48>]);
    state.counters["256"] = static_cast<double>(
        stats.node_counts[::unodb::as_i<unodb::node_type::I256>]);
    state.counters["+4"] =
        static_cast<double>(stats.growing_inode_counts
                                [::unodb::internal_as_i<unodb::node_type::I4>]);
    state.counters["4^"] = static_cast<double>(
        stats.growing_inode_counts
            [::unodb::internal_as_i<unodb::node_type::I16>]);
    state.counters["16^"] = static_cast<double>(
        stats.growing_inode_counts
            [::unodb::internal_as_i<unodb::node_type::I48>]);
    state.counters["48^"] = static_cast<double>(
        stats.growing_inode_counts
            [::unodb::internal_as_i<unodb::node_type::I256>]);
    state.counters["KPfS"] = static_cast<double>(stats.key_prefix_splits);
  }

 private:
  tree_stats<Db> stats;
#ifndef NDEBUG
  bool get_called{false};
  const Db *db{nullptr};
#endif
};

inline void set_size_counter(::benchmark::State &state,
                             const std::string &label, std::size_t value) {
  // state.SetLabel might be a better logical fit but the automatic k/M/G
  // suffix is too nice
  state.counters[label] = ::benchmark::Counter(
      static_cast<double>(value), ::benchmark::Counter::Flags::kDefaults,
      ::benchmark::Counter::OneK::kIs1024);
}

// Asserts

template <class Db, node_type DominatingINodeType>
void assert_dominating_inode_tree(
    const Db &test_db UNODB_DETAIL_USED_IN_DEBUG) noexcept {
#ifndef NDEBUG
  static_assert(DominatingINodeType != node_type::LEAF);
  const auto node_counts{test_db.get_node_counts()};
  auto node_type_i = as_i<node_type::I4>;
  std::uint64_t smaller_inode_count = 0;
  // Smaller-than-dominating inode types are allowed on the rightmost tree edge,
  // including the root.
  while (node_type_i < as_i<DominatingINodeType>) {
    smaller_inode_count += node_counts[node_type_i];
    assert(smaller_inode_count <= 8);
    ++node_type_i;
  }
  ++node_type_i;
  while (node_type_i <= as_i<node_type::I256>) {
    assert(node_counts[node_type_i] == 0);
    ++node_type_i;
  }
#endif
}

namespace detail {

template <class Db, unsigned NodeSize>
void assert_dominating_inode_size_tree(const Db &test_db) noexcept {
  assert_dominating_inode_tree<Db, node_size_to_node_type<NodeSize>()>(test_db);
}

template <class Db, unsigned SmallerNodeSize>
void assert_growing_nodes(
    const Db &test_db UNODB_DETAIL_USED_IN_DEBUG,
    std::uint64_t
        expected_number_of_nodes UNODB_DETAIL_USED_IN_DEBUG) noexcept {
#ifndef NDEBUG
  constexpr auto larger_node_type =
      node_size_to_larger_node_type<SmallerNodeSize>();
  const auto actual_number_of_nodes =
      test_db.template get_growing_inode_count<larger_node_type>();
  if (expected_number_of_nodes != actual_number_of_nodes) {
    std::cerr << "Difference between inserts: " << expected_number_of_nodes
              << ", N" << SmallerNodeSize << "^: " << actual_number_of_nodes;
    assert(expected_number_of_nodes == actual_number_of_nodes);
  }
#endif
}

template <class Db, unsigned SmallerNodeSize>
void assert_shrinking_nodes(
    const Db &test_db UNODB_DETAIL_USED_IN_DEBUG,
    std::uint64_t
        expected_number_of_nodes UNODB_DETAIL_USED_IN_DEBUG) noexcept {
#ifndef NDEBUG
  constexpr auto larger_node_type =
      node_size_to_larger_node_type<SmallerNodeSize>();
  const auto actual_number_of_nodes =
      test_db.template get_shrinking_inode_count<larger_node_type>();
  assert(expected_number_of_nodes == actual_number_of_nodes);
  assert_dominating_inode_size_tree<Db, SmallerNodeSize>(test_db);
#endif
}

template <class Db>
class tree_shape_snapshot final {
 public:
  explicit constexpr tree_shape_snapshot(
      const Db &test_db UNODB_DETAIL_USED_IN_DEBUG) noexcept
#ifndef NDEBUG
      : db{test_db}, stats {
    test_db
  }
#endif
  {}

  constexpr void assert_internal_levels_same() const noexcept {
#ifndef NDEBUG
    const tree_stats<Db> current_stats{db};
    assert(stats.internal_levels_equal(current_stats));
#endif
  }

 private:
#ifndef NDEBUG
  const Db &db;
  const tree_stats<Db> stats;
#endif
};

}  // namespace detail

// Insertion

template <class Db, unsigned NodeSize>
unodb::key insert_sequentially(Db &db, unsigned key_count) {
  unodb::key k = 0;
  decltype(key_count) i = 0;
  while (true) {
    insert_key(db, k, unodb::value_view{value100});
    if (i == key_count) break;
    ++i;
    k = next_key(k, node_size_to_key_zero_bits<NodeSize>());
  }
  detail::assert_dominating_inode_size_tree<Db, NodeSize>(db);
  return k;
}

template <class Db>
void insert_keys(Db &db, const std::vector<unodb::key> &keys) {
  for (const auto k : keys) {
    insert_key(db, k, unodb::value_view{value100});
  }
}

namespace detail {

template <class Db, typename NumberToKeyFn>
auto insert_keys_to_limit(Db &db, unodb::key key_limit,
                          NumberToKeyFn number_to_key_fn) {
  std::uint64_t i{0};
  while (true) {
    unodb::key key = number_to_key_fn(i);
    if (key > key_limit) break;
    insert_key(db, key, unodb::value_view{value100});
    ++i;
  }
  return i;
}

template <class Db, typename NumberToKeyFn>
auto insert_n_keys(Db &db, unsigned n, NumberToKeyFn number_to_key_fn) {
  unodb::key last_inserted_key{0};

  for (decltype(n) i = 0; i < n; ++i) {
    last_inserted_key = number_to_key_fn(i);
    insert_key(db, last_inserted_key, unodb::value_view{value100});
  }

  return last_inserted_key;
}

template <class Db, unsigned NodeSize, typename NumberToKeyFn>
auto insert_n_keys_to_empty_tree(Db &db, unsigned n,
                                 NumberToKeyFn number_to_key_fn) {
  assert(db.empty());
  const auto result = insert_n_keys(db, n, number_to_key_fn);
  assert_dominating_inode_size_tree<Db, NodeSize>(db);
  return result;
}

template <class Db, unsigned NodeSize>
auto make_full_node_size_tree(Db &db, unsigned key_count) {
  static_assert(NodeSize == 4 || NodeSize == 16 || NodeSize == 48 ||
                NodeSize == 256);

  if constexpr (node_size_has_key_zero_bits<NodeSize>()) {
    return insert_sequentially<Db, NodeSize>(db, key_count);
  } else {
    return insert_n_keys_to_empty_tree<
        Db, NodeSize, decltype(number_to_full_node_size_tree_key<NodeSize>)>(
        db, key_count, number_to_full_node_size_tree_key<NodeSize>);
  }
}

template <class Db, unsigned NodeCapacity>
std::tuple<unodb::key, const tree_shape_snapshot<Db>> make_base_tree_for_add(
    Db &test_db, unsigned node_count) {
  const auto key_limit = insert_n_keys_to_empty_tree<
      Db, NodeCapacity,
      decltype(number_to_minimal_node_size_tree_key<NodeCapacity>)>(
      test_db, node_count * (node_capacity_to_minimum_size<NodeCapacity>() + 1),
      number_to_minimal_node_size_tree_key<NodeCapacity>);
  return std::make_tuple(key_limit, tree_shape_snapshot<Db>{test_db});
}

template <class Db, unsigned NodeSize>
auto make_minimal_node_size_tree(Db &db, unsigned key_count) {
  return insert_n_keys_to_empty_tree<
      Db, NodeSize, decltype(number_to_minimal_node_size_tree_key<NodeSize>)>(
      db, key_count * node_capacity_to_minimum_size<NodeSize>(),
      number_to_minimal_node_size_tree_key<NodeSize>);
}

template <class Db, unsigned SmallerNodeSize>
auto grow_full_node_tree_to_minimal_next_size_leaf_level(Db &db,
                                                         unodb::key key_limit) {
  static_assert(SmallerNodeSize == 4 || SmallerNodeSize == 16 ||
                SmallerNodeSize == 48);

#ifndef NDEBUG
  assert_dominating_inode_size_tree<Db, SmallerNodeSize>(db);
  const auto initial_growing_inode_counts = db.get_growing_inode_counts();
#endif

  const auto keys_inserted = insert_keys_to_limit<
      Db,
      decltype(number_to_minimal_leaf_over_smaller_node_tree<SmallerNodeSize>)>(
      db, key_limit,
      number_to_minimal_leaf_over_smaller_node_tree<SmallerNodeSize>);

#ifndef NDEBUG
  assert_growing_nodes<Db, SmallerNodeSize>(db, keys_inserted);
  const auto final_growing_inode_counts = db.get_growing_inode_counts();
  for (auto node_type_i = as_i<node_type::I4>;
       node_type_i < as_i<node_size_to_node_type<SmallerNodeSize>()>;
       ++node_type_i) {
    assert(initial_growing_inode_counts[node_type_i] ==
           final_growing_inode_counts[node_type_i]);
  }
#endif

  return keys_inserted;
}

// Gets

template <class Db, typename NumberToKeyFn>
auto get_key_loop(Db &db, unodb::key key_limit,
                  NumberToKeyFn number_to_key_fn) {
  std::uint64_t i{0};
  while (true) {
    const auto key = number_to_key_fn(i);
    if (key > key_limit) break;
    get_existing_key(db, key);
    ++i;
  }
  return static_cast<std::int64_t>(i);
}

}  // namespace detail

// Deletes

template <class Db>
void delete_keys(Db &db, const std::vector<unodb::key> &keys) {
  for (const auto k : keys) delete_key(db, k);
}

// Benchmarks

template <class Db, unsigned NodeSize>
void full_node_scan_benchmark(::benchmark::State &state) {
  const auto key_count = static_cast<unsigned>(state.range(0));
  Db test_db;
  std::int64_t items_processed{0};

  if constexpr (detail::node_size_has_key_zero_bits<NodeSize>()) {
    const auto key_limit UNODB_DETAIL_USED_IN_DEBUG =
        detail::make_full_node_size_tree<Db, NodeSize>(test_db, key_count);
    for (auto _ : state) {
      unodb::key k = 0;
      for (std::uint64_t j = 0; j < key_count; ++j) {
        assert(k <= key_limit);
        get_existing_key(test_db, k);
        k = next_key(k, node_size_to_key_zero_bits<NodeSize>());
      }
      items_processed += key_count;
    }
  } else {
    const auto key_limit =
        detail::make_full_node_size_tree<Db, NodeSize>(test_db, key_count);
    for (auto _ : state) {
      items_processed += detail::get_key_loop(
          test_db, key_limit,
          detail::number_to_full_node_size_tree_key<NodeSize>);
    }
  }

  state.SetItemsProcessed(items_processed);
  set_size_counter(state, "size", test_db.get_current_memory_use());
}

template <class Db, unsigned NodeSize>
void full_node_random_get_benchmark(::benchmark::State &state) {
  Db test_db;
  const auto key_count = static_cast<unsigned>(state.range(0));

  detail::make_full_node_size_tree<Db, NodeSize>(test_db, key_count);
  const auto tree_size = test_db.get_current_memory_use();

  batched_prng random_key_positions{key_count - 1};

  for (auto _ : state) {
    for (std::uint64_t i = 0; i < key_count; ++i) {
      const auto key_index = random_key_positions.get(state);
      const auto key =
          detail::number_to_full_node_size_tree_key<NodeSize>(key_index);
      get_existing_key(test_db, key);
    }
  }

  state.SetItemsProcessed(static_cast<std::int64_t>(state.iterations()) *
                          key_count);
  set_size_counter(state, "size", tree_size);
}

// Benchmark e.g. growing Node4 to Node16: insert to full Node4 tree first:
// 0x0000000000000000 to ...003
// 0x0000000000000100 to ...103
// 0x0000000000000200 to ...203
// 0x0000000000000300 to ...303
// 0x0000000000010000 to ...003
// 0x0000000000010100 to ...103
// ...
// Then insert in the gaps a "base-5" value that varies each byte from 0 to 5
// with the last one being a constant 4 to get a minimal Node16 tree:
// 0x0000000000000004
// 0x0000000000000104
// 0x0000000000000204
// 0x0000000000000304
// 0x0000000000000404
// 0x0000000000010004
// 0x0000000000010104
// ...
// Node16 to Node48: insert to full Node16 tree first:
// 0x0000000000000000 to ...0000F
// 0x0000000000000100 to ...0010F
// ...
// 0x0000000000000F00 to ...00F0F
// 0x0000000000010000 to ...1000F
// ...
// 0x00000000000F0000 to ...F000F
// 0x0000000000010100 to ...1010F
// 0x0000000000010200 to ...1020F
// ...
// The insert in the gaps a "base-17" value with the last byte being a constant
// 10 to get a minimal Node48 tree:
// 0x0000000000000010
// 0x0000000000000110
// ...
// 0x0000000000000F10
// 0x0000000000010010
// ...

// Do not bother with extern templates due to large parameter space
template <class Db, unsigned SmallerNodeSize>
void grow_node_sequentially_benchmark(::benchmark::State &state) {
  std::size_t tree_size{0};
  const auto smaller_node_count = static_cast<unsigned>(state.range(0));
  std::uint64_t benchmark_keys_inserted{0};

  for (auto _ : state) {
    state.PauseTiming();
    Db test_db;
    unodb::key key_limit =
        detail::make_full_node_size_tree<Db, SmallerNodeSize>(
            test_db, smaller_node_count * SmallerNodeSize);
    ::benchmark::ClobberMemory();
    state.ResumeTiming();

    benchmark_keys_inserted =
        detail::grow_full_node_tree_to_minimal_next_size_leaf_level<
            Db, SmallerNodeSize>(test_db, key_limit);

    state.PauseTiming();
    detail::assert_growing_nodes<Db, SmallerNodeSize>(test_db,
                                                      benchmark_keys_inserted);
    tree_size = test_db.get_current_memory_use();
    destroy_tree(test_db, state);
  }

  state.SetItemsProcessed(static_cast<std::int64_t>(state.iterations()) *
                          static_cast<std::int64_t>(benchmark_keys_inserted));
  set_size_counter(state, "size", tree_size);
}

// Benchmark e.g. growing Node4 to Node16: insert to full Node4 tree first. Use
// 1, 3, 5, 7 as the different key byte values, so that a new byte could be
// inserted later at any position.
// 0x0101010101010101 to ...107
// 0x0101010101010301 to ...307
// 0x0101010101010501 to ...507
// 0x0101010101010701 to ...707
// 0x0101010101030101 to ...107
// 0x0101010101030301 to ...307
// ...
// Then in the gaps insert a value that has the last byte randomly chosen from
// 0, 2, 4, 6, and 8, and leading bytes enumerating through 1, 3, 5, 7, and one
// randomly-selected value from 0, 2, 4, 6, and 8.

// Do not bother with extern templates due to large parameter space
template <class Db, unsigned SmallerNodeSize>
void grow_node_randomly_benchmark(::benchmark::State &state) {
  std::size_t tree_size{0};
  const auto smaller_node_count = static_cast<unsigned>(state.range(0));
  std::uint64_t benchmark_keys_inserted{0};

  for (auto _ : state) {
    state.PauseTiming();
    Db test_db;
    const auto key_limit = detail::insert_n_keys_to_empty_tree<
        Db, SmallerNodeSize,
        decltype(detail::number_to_full_node_tree_with_gaps_key<
                 SmallerNodeSize>)>(
        test_db, smaller_node_count * SmallerNodeSize,
        detail::number_to_full_node_tree_with_gaps_key<SmallerNodeSize>);

    const auto larger_tree_keys =
        detail::generate_random_keys_over_full_smaller_tree<SmallerNodeSize>(
            key_limit);
    state.ResumeTiming();

    insert_keys(test_db, larger_tree_keys);

    state.PauseTiming();
    benchmark_keys_inserted = larger_tree_keys.size();
    detail::assert_growing_nodes<Db, SmallerNodeSize>(test_db,
                                                      benchmark_keys_inserted);
    tree_size = test_db.get_current_memory_use();
    destroy_tree(test_db, state);
  }

  state.SetItemsProcessed(static_cast<std::int64_t>(state.iterations()) *
                          static_cast<std::int64_t>(benchmark_keys_inserted));
  set_size_counter(state, "size", tree_size);
}

// Benchmark e.g. shrinking Node16 to Node4: insert to minimal Node16 first:
// 0x0000000000000000 to ...004
// 0x0000000000000100 to ...104
// 0x0000000000000200 to ...204
// 0x0000000000000300 to ...304
// 0x0000000000000404 (note that no 0x0400..0x403 to avoid creating Node4).
//
// Then remove the minimal Node16 over full Node4 key subset, see
// number_to_minimal_node16_over_node4_key.

template <class Db, unsigned SmallerNodeSize>
void shrink_node_sequentially_benchmark(::benchmark::State &state) {
  std::size_t tree_size{0};
  std::uint64_t removed_key_count{0};

  const auto smaller_node_count = static_cast<unsigned>(state.range(0));

  for (auto _ : state) {
    state.PauseTiming();
    Db test_db;
    unodb::key key_limit =
        detail::make_full_node_size_tree<Db, SmallerNodeSize>(
            test_db, smaller_node_count * SmallerNodeSize);

    const auto node_growing_keys_inserted =
        detail::grow_full_node_tree_to_minimal_next_size_leaf_level<
            Db, SmallerNodeSize>(test_db, key_limit);
    detail::assert_growing_nodes<Db, SmallerNodeSize>(
        test_db, node_growing_keys_inserted);
    tree_size = test_db.get_current_memory_use();
    state.ResumeTiming();

    for (removed_key_count = 0; removed_key_count < node_growing_keys_inserted;
         ++removed_key_count) {
      const auto remove_key =
          detail::number_to_minimal_leaf_over_smaller_node_tree<
              SmallerNodeSize>(removed_key_count);
      delete_key(test_db, remove_key);
    }

    state.PauseTiming();
    detail::assert_shrinking_nodes<Db, SmallerNodeSize>(test_db,
                                                        removed_key_count);
    destroy_tree(test_db, state);
  }

  state.SetItemsProcessed(
      static_cast<std::int64_t>(state.iterations() * removed_key_count));
  set_size_counter(state, "size", tree_size);
}

template <class Db, unsigned SmallerNodeSize>
void shrink_node_randomly_benchmark(::benchmark::State &state) {
  std::size_t tree_size{0};
  std::uint64_t removed_key_count{0};

  const auto smaller_node_count = static_cast<unsigned>(state.range(0));

  for (auto _ : state) {
    state.PauseTiming();
    Db test_db;
    const auto key_limit = detail::insert_n_keys_to_empty_tree<
        Db, SmallerNodeSize,
        decltype(detail::number_to_full_node_tree_with_gaps_key<
                 SmallerNodeSize>)>(
        test_db, smaller_node_count * SmallerNodeSize,
        detail::number_to_full_node_tree_with_gaps_key<SmallerNodeSize>);

    const auto node_growing_keys =
        detail::generate_random_keys_over_full_smaller_tree<SmallerNodeSize>(
            key_limit);
    insert_keys(test_db, node_growing_keys);
    detail::assert_growing_nodes<Db, SmallerNodeSize>(test_db,
                                                      node_growing_keys.size());
    tree_size = test_db.get_current_memory_use();
    state.ResumeTiming();

    delete_keys(test_db, node_growing_keys);

    state.PauseTiming();
    removed_key_count = node_growing_keys.size();
    detail::assert_shrinking_nodes<Db, SmallerNodeSize>(test_db,
                                                        removed_key_count);
    destroy_tree(test_db, state);
  }

  state.SetItemsProcessed(
      static_cast<std::int64_t>(state.iterations() * removed_key_count));
  set_size_counter(state, "size", tree_size);
}

template <class Db, unsigned NodeSize>
void sequential_add_benchmark(::benchmark::State &state) {
  std::size_t tree_size{0};
  const auto node_count = static_cast<unsigned>(state.range(0));
  std::uint64_t benchmark_keys_inserted{0};

  for (auto _ : state) {
    state.PauseTiming();
    Db test_db;
    auto [key_limit, tree_shape] =
        detail::make_base_tree_for_add<Db, NodeSize>(test_db, node_count);
    state.ResumeTiming();

    benchmark_keys_inserted = detail::insert_keys_to_limit<
        Db,
        decltype(detail::number_to_full_leaf_over_minimal_tree_key<NodeSize>)>(
        test_db, key_limit,
        detail::number_to_full_leaf_over_minimal_tree_key<NodeSize>);

    state.PauseTiming();
    detail::assert_dominating_inode_size_tree<Db, NodeSize>(test_db);
    tree_shape.assert_internal_levels_same();
    tree_size = test_db.get_current_memory_use();
    destroy_tree(test_db, state);
  }

  state.SetItemsProcessed(static_cast<std::int64_t>(state.iterations()) *
                          static_cast<std::int64_t>(benchmark_keys_inserted));
  set_size_counter(state, "size", tree_size);
}

template <class Db, unsigned NodeSize>
void random_add_benchmark(::benchmark::State &state) {
  std::size_t tree_size{0};
  const auto node_count = static_cast<unsigned>(state.range(0));
  std::int64_t benchmark_keys_inserted{0};

  for (auto _ : state) {
    state.PauseTiming();
    Db test_db;
    auto [key_limit, tree_shape] =
        detail::make_base_tree_for_add<Db, NodeSize>(test_db, node_count);
    auto benchmark_keys = detail::generate_keys_to_limit(
        key_limit, detail::number_to_full_leaf_over_minimal_tree_key<NodeSize>);
    std::shuffle(benchmark_keys.begin(), benchmark_keys.end(), get_prng());
    state.ResumeTiming();

    insert_keys(test_db, benchmark_keys);

    state.PauseTiming();
    detail::assert_dominating_inode_size_tree<Db, NodeSize>(test_db);
    tree_shape.assert_internal_levels_same();
    tree_size = test_db.get_current_memory_use();
    benchmark_keys_inserted = static_cast<std::int64_t>(benchmark_keys.size());
    destroy_tree(test_db, state);
  }

  state.SetItemsProcessed(static_cast<std::int64_t>(state.iterations()) *
                          benchmark_keys_inserted);
  set_size_counter(state, "size", tree_size);
}

template <class Db, unsigned NodeSize>
void minimal_tree_full_scan(::benchmark::State &state) {
  const auto key_count = static_cast<unsigned>(state.range(0));
  Db test_db;

  const auto key_limit =
      detail::make_minimal_node_size_tree<Db, NodeSize>(test_db, key_count);
  const auto tree_size = test_db.get_current_memory_use();

  std::int64_t items_processed = 0;
  for (auto _ : state) {
    items_processed += detail::get_key_loop(
        test_db, key_limit,
        detail::number_to_minimal_node_size_tree_key<NodeSize>);
  }

  state.SetItemsProcessed(items_processed);
  set_size_counter(state, "size", tree_size);
}

template <class Db, unsigned NodeSize>
void minimal_tree_random_gets(::benchmark::State &state) {
  const auto node_count = static_cast<unsigned>(state.range(0));
  Db test_db;
  const auto key_limit UNODB_DETAIL_USED_IN_DEBUG =
      detail::make_minimal_node_size_tree<Db, NodeSize>(test_db, node_count);
  assert(detail::number_to_minimal_node_size_tree_key<NodeSize>(
             node_count * detail::node_capacity_to_minimum_size<NodeSize>() -
             1) == key_limit);
  const auto tree_size = test_db.get_current_memory_use();
  batched_prng random_key_positions{
      node_count * detail::node_capacity_to_minimum_size<16>() - 1};
  std::int64_t items_processed = 0;

  for (auto _ : state) {
    const auto key_index = random_key_positions.get(state);
    const auto key =
        detail::number_to_minimal_node_size_tree_key<NodeSize>(key_index);
    get_existing_key(test_db, key);
    ++items_processed;
  }

  state.SetItemsProcessed(items_processed);
  set_size_counter(state, "size", tree_size);
}

template <class Db, unsigned NodeSize>
void sequential_delete_benchmark(::benchmark::State &state) {
  const auto key_count = static_cast<unsigned>(state.range(0));
  int i{0};
  std::size_t tree_size{0};

  for (auto _ : state) {
    state.PauseTiming();
    Db test_db;
    const auto key_limit =
        detail::make_full_node_size_tree<Db, NodeSize>(test_db, key_count);
    tree_size = test_db.get_current_memory_use();
    const detail::tree_shape_snapshot<Db> tree_shape{test_db};
    state.ResumeTiming();

    i = 0;
    while (true) {
      const auto k =
          detail::number_to_full_leaf_over_minimal_tree_key<NodeSize>(
              static_cast<std::uint64_t>(i));
      if (k > key_limit) break;
      delete_key(test_db, k);
      ++i;
    }

    state.PauseTiming();
    detail::assert_dominating_inode_size_tree<Db, NodeSize>(test_db);
    tree_shape.assert_internal_levels_same();
    destroy_tree(test_db, state);
  }

  state.SetItemsProcessed(static_cast<std::int64_t>(state.iterations()) * i);
  set_size_counter(state, "size", tree_size);
}

template <class Db, unsigned NodeSize>
void random_delete_benchmark(::benchmark::State &state) {
  const auto key_count{static_cast<unsigned>(state.range(0))};
  std::size_t tree_size{0};
  std::size_t remove_key_count{0};

  for (auto _ : state) {
    state.PauseTiming();
    Db test_db;
    const auto key_limit =
        detail::make_full_node_size_tree<Db, NodeSize>(test_db, key_count);
    tree_size = test_db.get_current_memory_use();
    const detail::tree_shape_snapshot<Db> tree_shape{test_db};
    auto remove_keys = detail::generate_keys_to_limit(
        key_limit, detail::number_to_full_leaf_over_minimal_tree_key<NodeSize>);
    remove_key_count = remove_keys.size();
    std::shuffle(remove_keys.begin(), remove_keys.end(), get_prng());
    state.ResumeTiming();

    delete_keys(test_db, remove_keys);

    state.PauseTiming();
    detail::assert_dominating_inode_size_tree<Db, NodeSize>(test_db);
    tree_shape.assert_internal_levels_same();
    destroy_tree(test_db, state);
  }
  state.SetItemsProcessed(static_cast<std::int64_t>(state.iterations()) *
                          static_cast<std::int64_t>(remove_key_count));
  set_size_counter(state, "size", tree_size);
}

}  // namespace unodb::benchmark

#endif  // UNODB_DETAIL_MICRO_BENCHMARK_NODE_UTILS_HPP
