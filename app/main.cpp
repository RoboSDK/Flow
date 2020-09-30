#include <cppcoro/static_thread_pool.hpp>
#include <cppcoro/single_producer_sequencer.hpp>
#include <cppcoro/io_service.hpp>
#include <cppcoro/task.hpp>
#include <cppcoro/when_all.hpp>
#include <cppcoro/sequence_barrier.hpp>
#include <cppcoro/sync_wait.hpp>

using namespace cppcoro;
using namespace std::chrono;

struct message {
  int id;
  steady_clock::time_point timestamp;
  float data;
};

constexpr size_t bufferSize = 16384;// Must be power-of-two
constexpr size_t indexMask = bufferSize - 1;
message buffer[bufferSize];

task<void> producer(
  io_service& ioSvc,
  single_producer_sequencer<size_t>& sequencer)
{
  [[maybe_unused]] auto start = steady_clock::now();
  for (int i = 0; i < 1'000'000; ++i) {
    // Wait until a slot is free in the buffer.
    size_t seq = co_await sequencer.claim_one(ioSvc);

    // Populate the message.
    auto& msg = buffer[seq & indexMask];
    msg.id = i;
    msg.timestamp = steady_clock::now();
    msg.data = 123;

    // Publish the message.
    sequencer.publish(seq);
  }

  // Publish a sentinel
  auto seq = co_await sequencer.claim_one(ioSvc);
  auto& msg = buffer[seq & indexMask];
  msg.id = -1;
  sequencer.publish(seq);
}

task<void> consumer(
  static_thread_pool& threadPool,
  const single_producer_sequencer<size_t>& sequencer,
  sequence_barrier<size_t>& consumerBarrier)
{
  size_t nextToRead = 0;
  while (true) {
    // Wait until the next message is available
    // There may be more than one available.
    const size_t available = co_await sequencer.wait_until_published(nextToRead, threadPool);
    do {
      auto& msg = buffer[nextToRead & indexMask];
      if (msg.id == -1) {
        consumerBarrier.publish(nextToRead);
        co_return;
      }

//      processMessage(msg);
    } while (nextToRead++ != available);

    // Notify the producer that we've finished processing
    // up to 'nextToRead - 1'.
    consumerBarrier.publish(available);
  }
}

task<void> example(io_service& ioSvc, static_thread_pool& threadPool)
{
  sequence_barrier<size_t> barrier;
  single_producer_sequencer<size_t> sequencer{ barrier, bufferSize };

  co_await when_all(
    producer(ioSvc, sequencer),
    consumer(threadPool, sequencer, barrier));
}

int main()
{
//  [[maybe_unused]] constexpr auto system = flow::make_system<Layer>();
//  [[maybe_unused]] constexpr auto options = flow::make_options(flow::scheduler_subscriber_buffer_size<1024>{},
//    flow::scheduler_publisher_buffer_size<64>{});
//
//  [[maybe_unused]] constexpr auto system2 = flow::make_system<Layer>(options);
  static_thread_pool tp;
  cppcoro::io_service service;
  cppcoro::sync_wait(example(service, tp));
}