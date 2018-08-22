// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Allow debugging memory leaks.
#include "../debug.hpp"

#include "file.hpp"

#include <fstream>
#include <sstream>

#include "logger.hpp"

namespace koma {
bool ReadFile(std::string *buffer, std::string file_path) {
  std::ifstream input_stream = std::ifstream(file_path, std::ios::in);

  input_stream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  if (!input_stream.is_open()) {
    Logger::Get(LOGGER_KOMA_UTILS)->Error(
      "Unable to open ", file_path
    );

    return false;
  }

  std::stringstream string_stream;

  string_stream << input_stream.rdbuf();
  *buffer = string_stream.str();

  input_stream.close();

  return true;
}
};  // namespace koma
