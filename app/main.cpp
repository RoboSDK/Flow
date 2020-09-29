#include <cppcoro/task.hpp>
#include <cppcoro/sync_wait.hpp>
#include "LidarTask.hpp"
#include <flow/logging.hpp>
#include <cppcoro/static_thread_pool.hpp>

template <typename Data>
struct request {
  Data data;
};

std::vector<request<LidarData>> requests;

template <typename CallBack>
cppcoro::task<LidarData> spin_task(LidarTask& task, cppcoro::static_thread_pool& tp, CallBack&& cb)
{
  flow::logging::info("spinning!");
  co_await tp.schedule();
  auto data = task.spin();
  cb();
  co_return data;
}

cppcoro::task<> process_requests() {
  LidarTask task;
  cppcoro::static_thread_pool tp;
  while (not requests.empty()) {
    flow::logging::info("Spinning task..");
    requests.back().data = co_await spin_task(task, tp, [&]{
      requests.emplace_back();
      flow::logging::info("Placing new request...sleep for 1 sec");
      std::this_thread::sleep_for(std::chrono::seconds(1));
    });
  }
  co_return;
}

template <typename Data>
void send_request(request<Data>&& r) {
  requests.push_back(std::move(r));
}


int main()
{
  send_request(request<LidarData>());
  cppcoro::sync_wait(process_requests());
}