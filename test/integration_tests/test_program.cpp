#include <array>
#include <cppcoro/async_mutex.hpp>
#include <cppcoro/generator.hpp>
#include <cppcoro/io_service.hpp>
#include <cppcoro/sequence_barrier.hpp>
#include <cppcoro/single_producer_sequencer.hpp>
#include <cppcoro/static_thread_pool.hpp>
#include <cppcoro/sync_wait.hpp>
#include <cppcoro/task.hpp>
#include <cppcoro/when_all_ready.hpp>
#include <flow/configuration.hpp>
#include <flow/data_structures/any_type_set.hpp>
#include <flow/data_structures/mixed_array.hpp>
#include <flow/logging.hpp>
#include <span>
#include <variant>

static constexpr std::size_t message_buffer_size = 32;
static constexpr std::size_t num_threads = 1;
using thread_pool = cppcoro::static_thread_pool;
static thread_pool scheduler{ num_threads };

static constexpr std::size_t num_channels = 32;
cppcoro::async_mutex mutex;
namespace flow_2 {


struct resource {
  using sequence_barrier = cppcoro::sequence_barrier<std::size_t>;
  using single_producer_sequencer = cppcoro::single_producer_sequencer<std::size_t>;

  sequence_barrier barrier{};
  single_producer_sequencer sequencer{ barrier, message_buffer_size };
};

struct channel_resource_generator {
  std::array<resource, num_channels> channel_resources{};
  std::size_t current_resource{};
};

channel_resource_generator generator{};

std::reference_wrapper<resource> get_resource()
{
  return std::ref(generator.channel_resources[std::atomic_ref(generator.current_resource)++]);
}

template<typename message_t>
struct channel {
  std::size_t hash() { return typeid(message_t).hash_code(); }
  channel() = default;
  channel(channel const& other) : connection(get_resource())
  {
    std::copy(std::begin(other.buffer), std::end(other.buffer), std::begin(buffer));
  }

  channel operator=(channel const& other)
  {
    connection = get_resource();
    std::copy(std::begin(other.buffer), std::end(other.buffer), std::begin(buffer));
  }

  static constexpr std::size_t index_mask = message_buffer_size - 1;

  std::reference_wrapper<resource> connection = get_resource();
  std::array<message_t, message_buffer_size> buffer{};
};

template<typename message_t>
struct begin_point {
  cppcoro::task<message_t> spin()
  {
    flow::logging::info("generating!");
    co_return message_t{};
  }
};

template<typename return_t, typename argument_t>
struct transition_point {
  cppcoro::task<return_t> spin(argument_t const& /*unused*/)
  {
    co_return return_t{};
  }
};

template<typename message_t>
struct end_point {
  cppcoro::task<void> spin(message_t const& /*unused*/)
  {
    flow::logging::info("consuming!");
    co_return;
  }
};

template<typename return_t>
cppcoro::task<> spin_publisher(begin_point<return_t>&& e, auto& channels)
{
  using channel_t = channel<return_t>;
  if (not channels.template contains<channel_t>()) {
    throw std::runtime_error("does not contain channel");
  }
  channel_t& entry_channel = channels.template at<channel_t>();

  while (true) {
    flow::logging::info("entry_channel buffer size {}", entry_channel.buffer.size());
    const auto buffer_sequence = co_await entry_channel.connection.get().sequencer.claim_one(scheduler);
    auto& message = entry_channel.buffer[buffer_sequence & entry_channel.index_mask];
    message = cppcoro::sync_wait(e.spin());
    entry_channel.connection.get().sequencer.publish(buffer_sequence);
  }
}

std::atomic_size_t next_to_read = 0;
template<typename argument_t>
cppcoro::task<> spin_subscriber(end_point<argument_t> e, auto& channels)
{
  using channel_t = channel<argument_t>;

  if (not channels.template contains<channel_t>()) {
    throw std::runtime_error("does not contain channel");
  }
  auto& end_channel = channels.template at<channel_t>();
  end_channel.connection = get_resource();

  while (true) {
    auto& sequencer = end_channel.connection.get().sequencer;

    flow::logging::info("wait");
    const size_t available = co_await sequencer.wait_until_published(next_to_read, scheduler);
    flow::logging::info("boo");

    do {
      flow::logging::info("spin");
      auto& message = end_channel.buffer[next_to_read & end_channel.index_mask];
      cppcoro::sync_wait(e.spin(message));
    } while (next_to_read++ < available);

    end_channel.connection.get().barrier.publish(available);
  }
}


struct chain {
  using task_t = cppcoro::task<void>;

  template<typename return_t>
  void push_begin(begin_point<return_t>&& begin)
  {
    if (not channels.contains<channel<return_t>>()) {
      channels.put(channel<return_t>{});
    }
    tasks.push_back(spin_publisher(std::move(begin), channels));
  }

  template<typename argument_t>
  void push_end(end_point<argument_t>&& end)
  {
    if (not channels.contains<channel<argument_t>>()) {
      channels.put(channel<argument_t>{});
    }

    tasks.push_back(spin_subscriber(std::move(end), channels));
  }

  cppcoro::task<> spin()
  {
    co_await cppcoro::when_all_ready(std::move(tasks));
  }

  flow::any_type_set channels{};
  std::vector<task_t> tasks{};
};
}// namespace flow_2


int main()
{
  using namespace flow_2;
  chain ch{};
  ch.push_end(end_point<int>{});
  ch.push_begin(begin_point<int>{});
  cppcoro::sync_wait(ch.spin());
}
