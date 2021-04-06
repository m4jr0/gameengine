// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "file_system.hpp"

#include "boost/algorithm/string.hpp"
#include "date.hpp"
#include "logger.hpp"
#include "picosha2.h"

#ifdef _WIN32
#include "debug_windows.hpp"
#endif  // _WIN32

namespace comet {
namespace filesystem {
bool WriteToFile(const std::string &file_path, const std::string &buffer,
                 bool is_append) {
  std::string directory_path = GetParentPath(file_path);

  if (!IsExist(directory_path) || !IsDirectory(directory_path)) return false;

#ifdef _WIN32
  // Necessary cast to make it work on Windows.
  auto mode = static_cast<int>(boost::filesystem::ofstream::out);
#else
  auto mode = boost::filesystem::ofstream::out;
#endif  // _WIN32

  if (!is_append) {
    mode |= boost::filesystem::ofstream::trunc;
  } else {
    mode |= boost::filesystem::ofstream::app;
  }

  auto file = boost::filesystem::ofstream(file_path, mode);

  file << buffer;
  file.close();

  return true;
}

bool ReadFile(const std::string &file_path, std::string *buffer) {
  std::ifstream input_stream;
  input_stream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  auto is_open = false;

  try {
    input_stream.open(file_path, std::ios::in);
    is_open = input_stream.is_open();
  } catch (std::ifstream::failure &failure) {
    is_open = false;
  }

  if (!is_open) {
    Logger::Get(kLoggerCometUtilsFileSystem)
        ->Error("Unable to open ", file_path);

    return false;
  }

  std::stringstream string_stream{};

  string_stream << input_stream.rdbuf();
  *buffer = string_stream.str();

  input_stream.close();

  return true;
}

bool CreateFile(const std::string &create_path, bool is_recursive) {
  if (create_path.length() < 0) return false;

  const auto last_character = std::string(&create_path.back());

  if (last_character == "/" || last_character == "\\") return false;

  auto path = boost::filesystem::path(create_path);

  if (is_recursive) {
    boost::filesystem::path parent_path(GetParentPath(create_path));

    if (!boost::filesystem::create_directories(parent_path)) return false;
  }

  const auto directory_path = GetParentPath(create_path);

  if (!IsDirectory(directory_path) || !IsExist(directory_path)) {
    return false;
  }

  WriteToFile(create_path, "");

  return true;
}

bool CreateDirectory(const std::string &create_path, bool is_recursive) {
  const auto path = boost::filesystem::path(create_path);

  if (is_recursive) {
    return boost::filesystem::create_directories(path);
  } else {
    if (!IsDirectory(GetParentPath(create_path))) return false;

    return boost::filesystem::create_directory(path);
  }
}

bool Move(const std::string &previous_name, const std::string &new_name) {
  if (!IsExist(previous_name) ||
      (IsExist(new_name)) && previous_name != new_name) {
    return false;
  }

  const auto previous_path = boost::filesystem::path(previous_name);
  const auto new_path = boost::filesystem::path(new_name);
  boost::filesystem::rename(previous_path, new_path);

  return true;
}

bool Remove(const std::string &to_delete, bool is_recursive) {
  const auto path = boost::filesystem::path(to_delete);

  if (!is_recursive && IsDirectory(to_delete) && !IsEmpty(to_delete)) {
    return false;
  } else {
    return boost::filesystem::remove_all(path);
  }
}

std::vector<std::string> ListDirectories(const std::string &directory,
                                         bool is_sorted) {
  if (!IsExist(directory) || IsFile(directory)) {
    return std::vector<std::string>();
  }

  std::vector<std::string> list;
  const auto path = boost::filesystem::path(directory);
  boost::filesystem::directory_iterator end_it;

  for (boost::filesystem::directory_iterator it(path); it != end_it; ++it) {
    if (boost::filesystem::is_directory(it->path())) {
      list.push_back(it->path().generic_string());
    }
  }

  if (is_sorted) {
    std::sort(list.begin(), list.end());
  }

  return list;
}

std::vector<std::string> ListFiles(const std::string &directory,
                                   bool is_sorted) {
  if (!IsExist(directory)) return std::vector<std::string>();

  std::vector<std::string> list;

  if (IsFile(directory)) {
    list.push_back(directory);

    return list;
  }

  const auto path = boost::filesystem::path(directory);
  boost::filesystem::directory_iterator end_it;

  for (boost::filesystem::directory_iterator it(path); it != end_it; ++it) {
    std::string a = it->path().generic_string();
    if (!boost::filesystem::is_directory(it->path())) {
      list.push_back(it->path().generic_string());
    }
  }

  if (is_sorted) {
    std::sort(list.begin(), list.end());
  }

  return list;
}

std::vector<std::string> ListAll(const std::string &directory, bool is_sorted) {
  if (!IsExist(directory)) return std::vector<std::string>();

  std::vector<std::string> list;
  const auto path = boost::filesystem::path(directory);
  boost::filesystem::directory_iterator end_it;

  for (boost::filesystem::directory_iterator it(path); it != end_it; ++it) {
    list.push_back(it->path().generic_string());
  }

  if (is_sorted) {
    std::sort(list.begin(), list.end());
  }

  return list;
}

std::string GetCurrentDirectory() {
  return boost::filesystem::current_path().generic_string();
}

std::string GetAbsolutePath(const std::string &relative_path) {
  return boost::filesystem::path(relative_path).generic_path().generic_string();
}

std::string GetRelativePath(const std::string &absolute_path) {
  return GetRelativePath(absolute_path, GetCurrentDirectory());
}

std::string GetDirectoryPath(const std::string &file_path) {
  const auto is_directory = IsDirectory(file_path);
  const auto is_file = IsFile(file_path);

  if (!is_directory && !is_file) return "";

  if (is_directory) return file_path;

  return GetParentPath(file_path);
}

std::string GetName(const std::string &path) {
  if (!IsFile(path) && !IsDirectory(path)) return "";

  return path.substr(path.find_last_of("/") + 1, path.length());
}

std::string GetExtension(const std::string &file_path) {
  auto extension =
      boost::filesystem::path(file_path).extension().generic_string();

  extension.erase(0, 1);
  boost::algorithm::to_lower(extension);

  return extension;
}

std::string GetParentPath(const std::string &current_path) {
  auto absolute_path = GetAbsolutePath(current_path);
  const auto path_size = absolute_path.length();

  if (path_size <= 0) return "";

  const auto last_character = std::string(&absolute_path.back());

  if (last_character == "/") {
    absolute_path = absolute_path.substr(0, path_size - 1);
  }

  const auto last_index = absolute_path.find_last_of("/");

  if (last_index == std::string::npos) return absolute_path;

  return absolute_path.substr(0, last_index);
}

std::string GetNativePath(const std::string &path) {
  return boost::filesystem::path(path).string();
}

bool IsDirectory(const std::string &path) {
  return boost::filesystem::is_directory(path);
}

bool IsFile(const std::string &path) {
  std::cout << GetCurrentDirectory() << std::endl;
  return boost::filesystem::is_regular_file(path);
}

bool IsAbsolute(const std::string &path) {
  return boost::filesystem::path(path).is_absolute();
}

bool IsRelative(const std::string &path) {
  return boost::filesystem::path(path).is_relative();
}

bool IsExist(const std::string &path) {
  return boost::filesystem::exists(path);
}

bool IsEmpty(const std::string &path) {
  return boost::filesystem::is_empty(path);
}

std::string Append(const std::string &a, const std::string &b) {
  auto formatted_a = std::string(a);
  auto formatted_b = std::string(b);

  RemoveTrailingSlashes(formatted_a);
  RemoveLeadingSlashes(formatted_b);

  return formatted_a + "/" + formatted_b;
}

std::string GetNormalizedPath(const std::string &path) {
  return boost::filesystem::path(path).lexically_normal().generic_string();
}

std::string GetRelativePath(const std::string &from, const std::string &to) {
  const auto from_path = boost::filesystem::path(from);
  const auto to_path = boost::filesystem::path(to);

  return from_path.lexically_relative(to).generic_string();
}

void RemoveTrailingSlashes(std::string &path) {
  path.erase(std::find_if(path.rbegin(), path.rend(),
                          [](int character) { return character != '/'; })
                 .base(),
             path.end());
}

void RemoveLeadingSlashes(std::string &path) {
  path.erase(path.begin(),
             std::find_if(path.begin(), path.end(),
                          [](int character) { return character != '/'; }));
}

double GetLastModificationTime(const std::string &path) {
  if (!IsExist(path)) return -1;

  return comet::date::GetDouble(boost::filesystem::last_write_time(path));
}

std::string GetChecksum(const std::string &path) {
  if (!IsFile(path)) return "";

  auto file = std::ifstream(path, std::ios::binary);

  auto checksum = std::vector<unsigned char>(picosha2::k_digest_size);

  picosha2::hash256(file, checksum.begin(), checksum.end());

  return std::string(checksum.cbegin(), checksum.cend());
}
}  // namespace filesystem
}  // namespace comet
