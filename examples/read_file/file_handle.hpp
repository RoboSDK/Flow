#ifndef FLOW_FILE_HANDLE_HPP
#define FLOW_FILE_HANDLE_HPP

#include <fstream>
#include <spdlog/spdlog.h>

namespace flow {
class FileHandle {
public:
  FileHandle(std::ifstream* f, std::string filename) : file(f)
  {
    file->open(filename);
    spdlog::info(filename);
    if (file->good() and file->is_open()) {
      spdlog::info("file good");
    }
    else {
      spdlog::error("file bad");
    }
  }

//  ~FileHandle()
//  {
//    file->close();
//  }

  std::string operator()()
  {
    std::string line;
    if (file->is_open() and std::getline(*file, line)) {
      return line;
    }
    else if (not file->is_open()) {
      spdlog::error("file not open");
    }

    on_complete_callback();
    return line;
  }

  void on_complete(std::function<void()> handler)
  {
    on_complete_callback = std::move(handler);
  }

private:
  std::function<void()> on_complete_callback = [] {};
  std::ifstream* file;
};
}// namespace flow

#endif// FLOW_FILE_HANDLE_HPP
