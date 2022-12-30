// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "file_system.h"

#include "comet/utils/date.h"
#include "comet/utils/hash.h"

namespace comet {
namespace utils {
namespace filesystem {
bool OpenBinaryFileToWriteTo(const char* path, std::ofstream& out_file,
                             bool is_append) {
  auto directory_path{GetParentPath(path)};

  if (!Exists(directory_path) || !IsDirectory(directory_path)) {
    return false;
  }

  auto mode{std::ios::binary | std::ios::out};

  if (!is_append) {
    mode |= std::ios::trunc;
  } else {
    mode |= std::ios::app;
  }

  out_file.open(path, mode);
  return true;
}

bool OpenBinaryFileToWriteTo(const std::string& path, std::ofstream& out_file,
                             bool is_append) {
  return OpenBinaryFileToWriteTo(path.c_str(), out_file, is_append);
}

bool OpenBinaryFileToReadFrom(const char* path, std::ifstream& in_file,
                              bool is_at_end) {
  in_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  try {
    // Weird case to keep MSVC happy.
#ifdef COMET_MSVC
    auto mode{static_cast<s32>(std::ios::binary)};
#else
    auto mode{std::ios::binary};
#endif  // COMET_MSVC

    if (is_at_end) {
      mode |= std::ios::ate;
    }

    in_file.open(path, mode);
    return in_file.is_open();
  } catch (std::runtime_error& error) {
    COMET_LOG_UTILS_ERROR("Could not open file at path: ", path,
                          "! Reason: ", error.what());
    return false;
  }
}

bool OpenBinaryFileToReadFrom(const std::string& path, std::ifstream& in_file,
                              bool is_at_end) {
  return OpenBinaryFileToReadFrom(path.c_str(), in_file, is_at_end);
}

void CloseFile(std::ofstream& file) { file.close(); }

bool WriteBinaryToFile(const char* path, const char* buffer, uindex buffer_size,
                       bool is_append) {
  std::ofstream file;

  if (!OpenBinaryFileToWriteTo(path, file, is_append)) {
    return false;
  }

  file.write(buffer, buffer_size);
  CloseFile(file);
  return true;
}

bool WriteBinaryToFile(const std::string& path, const char* buffer,
                       uindex buffer_size, bool is_append) {
  return WriteBinaryToFile(path.c_str(), buffer, buffer_size, is_append);
}

bool ReadBinaryFromFile(const std::string& path, std::vector<char>& buffer) {
  std::ifstream input_stream;

  if (!OpenBinaryFileToReadFrom(path, input_stream, true)) {
    return false;
  }

  const auto file_size{input_stream.tellg()};
  buffer.resize(file_size);
  input_stream.seekg(0);
  input_stream.read(buffer.data(), file_size);

  CloseFile(input_stream);
  return true;
}

void CloseFile(std::ifstream& file) { file.close(); }

bool WriteStrToFile(const char* path, const char* buffer, bool is_append) {
  auto directory_path{GetParentPath(path)};

  if (!Exists(directory_path) || !IsDirectory(directory_path)) {
    return false;
  }

  auto mode{std::ios::binary | std::ios::out};

  if (!is_append) {
    mode |= std::ios::trunc;
  } else {
    mode |= std::ios::app;
  }

  std::ofstream out_file;
  out_file.open(path, mode);
  out_file << buffer;
  out_file.close();

  return true;
}

bool WriteStrToFile(const char* path, std::string& buffer, bool is_append) {
  return WriteStrToFile(path, buffer.c_str(), is_append);
}

bool WriteStrToFile(const std::string& path, const char* buffer,
                    bool is_append) {
  return WriteStrToFile(path.c_str(), buffer, is_append);
}

bool WriteStrToFile(const std::string& path, const std::string& buffer,
                    bool is_append) {
  return WriteStrToFile(path.c_str(), buffer.c_str(), is_append);
}

bool ReadStrFromFile(const char* path, std::string& buffer) {
  std::ifstream input_stream;
  input_stream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  auto is_open{false};

  try {
    input_stream.open(path, std::ios::in);
    is_open = input_stream.is_open();
  } catch (std::runtime_error& error) {
    COMET_LOG_UTILS_ERROR("Could not open file at path: ", path,
                          "! Reason: ", error.what());
    is_open = false;
  }

  if (!is_open) {
    return false;
  }

  std::stringstream string_stream{};
  string_stream << input_stream.rdbuf();
  buffer = string_stream.str();
  input_stream.close();
  return true;
}

bool ReadStrFromFile(const std::string& path, std::string& buffer) {
  return ReadStrFromFile(path.c_str(), buffer);
}

bool CreateFile(const char* create_path, bool is_recursive) {
  if (!create_path) {
    return false;
  }

  const auto last_character{create_path[std::strlen(create_path) - 1]};

  if (last_character == '/' || last_character == '\\') {
    return false;
  }

  if (is_recursive) {
    std::filesystem::path parent_path{GetParentPath(create_path)};

    if (!std::filesystem::create_directories(parent_path)) {
      return false;
    }
  }

  const auto directory_path{GetParentPath(create_path)};

  if (!IsDirectory(directory_path) || !Exists(directory_path)) {
    return false;
  }

  WriteStrToFile(create_path, "");

  return true;
}

bool CreateFile(const std::string& create_path, bool is_recursive) {
  return CreateFile(create_path.c_str(), is_recursive);
}

bool CreateDirectory(const char* create_path, bool is_recursive) {
  const std::filesystem::path path{create_path};

  if (is_recursive) {
    return std::filesystem::create_directories(path);
  } else {
    if (!IsDirectory(GetParentPath(create_path))) {
      return false;
    }

    return std::filesystem::create_directory(path);
  }
}

bool CreateDirectory(const std::string& create_path, bool is_recursive) {
  return CreateDirectory(create_path.c_str(), is_recursive);
}

bool Move(const char* previous_name, const char* new_name) {
  if (!Exists(previous_name) ||
      (Exists(new_name)) && std::strcmp(previous_name, new_name) != 0) {
    return false;
  }

  const std::filesystem::path previous_path{previous_name};
  const std::filesystem::path new_path{new_name};
  std::filesystem::rename(previous_path, new_path);

  return true;
}

bool Move(const char* previous_name, const std::string& new_name) {
  return Move(previous_name, new_name.c_str());
}

bool Move(const std::string& previous_name, const char* new_name) {
  return Move(previous_name.c_str(), new_name);
}

bool Move(const std::string& previous_name, const std::string& new_name) {
  return Move(previous_name.c_str(), new_name.c_str());
}

bool Remove(const char* path, bool is_recursive) {
  if (!is_recursive && IsDirectory(path) && !IsEmpty(path)) {
    return false;
  } else {
    const std::filesystem::path path_obj{path};
    return std::filesystem::remove_all(path_obj);
  }
}

bool Remove(const std::string& path, bool is_recursive) {
  return Remove(path.c_str(), is_recursive);
}

std::vector<std::string> ListDirectories(const char* directory,
                                         bool is_sorted) {
  if (!Exists(directory) || IsFile(directory)) {
    return std::vector<std::string>();
  }

  std::vector<std::string> list;
  const std::filesystem::path path{directory};
  std::filesystem::directory_iterator end_it;

  for (std::filesystem::directory_iterator it{path}; it != end_it; ++it) {
    if (std::filesystem::is_directory(it->path())) {
      list.push_back(it->path().generic_string());
    }
  }

  if (is_sorted) {
    std::sort(list.begin(), list.end());
  }

  return list;
}

std::vector<std::string> ListDirectories(const std::string& directory,
                                         bool is_sorted) {
  return ListDirectories(directory.c_str(), is_sorted);
}

std::vector<std::string> ListFiles(const char* directory, bool is_sorted) {
  if (!Exists(directory)) {
    return std::vector<std::string>();
  }

  std::vector<std::string> list;

  if (IsFile(directory)) {
    list.push_back(directory);

    return list;
  }

  const std::filesystem::path path{directory};
  std::filesystem::directory_iterator end_it;

  for (std::filesystem::directory_iterator it{path}; it != end_it; ++it) {
    if (!std::filesystem::is_directory(it->path())) {
      list.push_back(it->path().generic_string());
    }
  }

  if (is_sorted) {
    std::sort(list.begin(), list.end());
  }

  return list;
}

std::vector<std::string> ListFiles(const std::string& directory,
                                   bool is_sorted) {
  return ListFiles(directory.c_str(), is_sorted);
}

std::vector<std::string> ListAll(const char* directory, bool is_sorted) {
  if (!Exists(directory)) {
    return std::vector<std::string>();
  }

  std::vector<std::string> list;
  const auto path{std::filesystem::path(directory)};
  std::filesystem::directory_iterator end_it;

  for (std::filesystem::directory_iterator it{path}; it != end_it; ++it) {
    list.push_back(it->path().generic_string());
  }

  if (is_sorted) {
    std::sort(list.begin(), list.end());
  }

  return list;
}

std::vector<std::string> ListAll(const std::string& directory, bool is_sorted) {
  return ListAll(directory.c_str(), is_sorted);
}

std::string GetCurrentDirectory() {
  return std::filesystem::current_path().generic_string();
}

std::string GetDirectoryPath(const char* path) {
  const auto is_directory{IsDirectory(path)};
  const auto is_file{IsFile(path)};

  if (!is_directory && !is_file) {
    return "";
  }

  if (is_directory) {
    return path;
  }

  return GetParentPath(path);
}

std::string GetDirectoryPath(const std::string& path) {
  return GetDirectoryPath(path.c_str());
}

std::string GetName(const std::string& path) {
  if (!IsFile(path) && !IsDirectory(path)) {
    return "";
  }

  return path.substr(path.find_last_of("/") + 1, path.length());
}

std::string GetExtension(const std::string& path, bool is_force_lowercase) {
  auto extension{std::filesystem::path(path).extension().generic_string()};

  extension.erase(0, 1);

  if (is_force_lowercase) {
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](u8 c) { return std::tolower(c); });
  }

  return extension;
}

std::string ReplaceExtension(const std::string& path, std::string extension,
                             bool is_force_lowercase) {
  std::string new_path{path};
  const auto old_extension{
      std::filesystem::path(path).extension().generic_string()};
  new_path.erase(new_path.size() - old_extension.size(), old_extension.size());

  if (is_force_lowercase) {
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](u8 c) { return std::tolower(c); });
  }

  if (extension.size() > 0 && extension[0] != '.') {
    extension.insert(0, 1, '.');
  }

  new_path = new_path.append(extension);
  return new_path;
}

std::string GetParentPath(const std::string& current_path) {
  auto absolute_path{GetAbsolutePath(current_path)};
  const auto path_size{absolute_path.length()};

  if (path_size <= 0) {
    return "";
  }

  const auto last_character{std::string(&absolute_path.back())};

  if (last_character == "/") {
    absolute_path = absolute_path.substr(0, path_size - 1);
  }

  const auto last_index{absolute_path.find_last_of("/")};

  if (last_index == std::string::npos) {
    return absolute_path;
  }

  return absolute_path.substr(0, last_index);
}

std::string GetNativePath(const std::string& path) {
  return std::filesystem::path(path).string();
}

bool IsDirectory(const std::string& path) {
  return std::filesystem::is_directory(path);
}

bool IsFile(const std::string& path) {
  return std::filesystem::is_regular_file(path);
}

bool IsAbsolute(const std::string& path) {
  return std::filesystem::path(path).is_absolute();
}

bool IsRelative(const std::string& path) {
  return std::filesystem::path(path).is_relative();
}

bool Exists(const std::string& path) { return std::filesystem::exists(path); }

bool IsEmpty(const std::string& path) {
  return std::filesystem::is_empty(path);
}

std::string Append(const std::string& path_a, const std::string& path_b) {
  auto formatted_a{std::string(path_a)};
  auto formatted_b{std::string(path_b)};

  RemoveTrailingSlashes(formatted_a);
  RemoveLeadingSlashes(formatted_b);

  return formatted_a + "/" + formatted_b;
}

std::string GetNormalizedPath(const std::string& path) {
  return std::filesystem::path(path).lexically_normal().generic_string();
}

std::string GetRelativePath(const std::string& from_path,
                            const std::string& to_path) {
  const auto from_path_obj{std::filesystem::path(from_path)};
  const auto to_path_obj{std::filesystem::path(to_path)};

  return from_path_obj.lexically_relative(to_path_obj).generic_string();
}

void RemoveTrailingSlashes(std::string& path) {
  path.erase(std::find_if(path.rbegin(), path.rend(),
                          [](s8 character) { return character != '/'; })
                 .base(),
             path.end());
}

void RemoveLeadingSlashes(std::string& path) {
  path.erase(path.begin(),
             std::find_if(path.begin(), path.end(),
                          [](s8 character) { return character != '/'; }));
}

f64 GetLastModificationTime(const char* path) {
  if (!Exists(path)) {
    return -1;
  }

  struct stat result {};

  if (stat(path, &result) == 0) {
    return result.st_mtime * 1000;
  }

  return -1;
}

f64 GetLastModificationTime(const std::string& path) {
  return GetLastModificationTime(path.c_str());
}

std::string GetChecksum(const char* path) {
  if (!IsFile(path)) {
    return "";
  }

  auto file{std::ifstream(path, std::ios::binary)};
  return hash::HashSha256(file);
}

std::string GetChecksum(const std::string& path) {
  return GetChecksum(path.c_str());
}
}  // namespace filesystem
}  // namespace utils
}  // namespace comet
