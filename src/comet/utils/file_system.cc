// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "file_system.h"

#include "comet/utils/date.h"
#include "comet/utils/hash.h"

namespace comet {
namespace utils {
namespace filesystem {
bool OpenBinaryFileToWriteTo(const schar* path, std::ofstream& out_file,
                             bool is_append) {
  auto mode{static_cast<std::ios_base::openmode>(
      std::ios::binary | std::ios::out | std::ios::failbit)};

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

bool OpenBinaryFileToReadFrom(const schar* path, std::ifstream& in_file,
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

bool WriteBinaryToFile(const schar* path, const u8* buffer, uindex buffer_size,
                       bool is_append) {
  std::ofstream file;

  if (!OpenBinaryFileToWriteTo(path, file, is_append)) {
    return false;
  }

  file.write(reinterpret_cast<const schar*>(buffer), buffer_size);
  CloseFile(file);
  return true;
}

bool WriteBinaryToFile(const std::string& path, const u8* buffer,
                       uindex buffer_size, bool is_append) {
  return WriteBinaryToFile(path.c_str(), buffer, buffer_size, is_append);
}

bool ReadBinaryFromFile(const schar* path, std::vector<u8>& buffer) {
  std::ifstream input_stream;

  if (!OpenBinaryFileToReadFrom(path, input_stream, true)) {
    return false;
  }

  const auto file_size{input_stream.tellg()};
  buffer.resize(file_size);
  input_stream.seekg(0);
  input_stream.read(reinterpret_cast<schar*>(buffer.data()), file_size);

  CloseFile(input_stream);
  return true;
}

bool ReadBinaryFromFile(const std::string& path, std::vector<u8>& buffer) {
  return ReadBinaryFromFile(path.c_str(), buffer);
}

void CloseFile(std::ifstream& file) { file.close(); }

bool WriteStrToFile(const schar* path, std::string_view buffer,
                    bool is_append) {
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

bool WriteStrToFile(const std::string& path, std::string_view buffer,
                    bool is_append) {
  return WriteStrToFile(path.c_str(), buffer, is_append);
}

bool ReadStrFromFile(const schar* path, std::string& buffer) {
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

bool CreateFile(const schar* create_path, bool is_recursive) {
  const auto str_len{strlen(create_path)};

  if (str_len == 0) {
    return false;
  }

  const auto last_character{create_path[str_len - 1]};

  if (last_character == '/') {
    return false;
  }

  const auto str_directory_path{GetParentPath(create_path)};

  if (is_recursive) {
    if (!std::filesystem::create_directories(str_directory_path)) {
      return false;
    }
  }

  // Save one allocation.
  std::filesystem::path directory_path{str_directory_path};

  if (!IsDirectory(directory_path) || !Exists(directory_path)) {
    return false;
  }

  WriteStrToFile(str_directory_path, "");
  return true;
}

bool CreateFile(const std::string& create_path, bool is_recursive) {
  return CreateFile(create_path.c_str(), is_recursive);
}

bool CreateDirectory(std::string_view create_path, bool is_recursive) {
  const std::filesystem::path path{create_path};

  if (is_recursive) {
    return std::filesystem::create_directories(path);
  } else {
    if (!IsDirectory(GetParentPathView(create_path))) {
      return false;
    }

    return std::filesystem::create_directory(path);
  }
}

bool Move(std::string_view previous_name, std::string_view new_name) {
  std::filesystem::path previous_name_path{previous_name};
  std::filesystem::path new_name_path{new_name};

  if (!Exists(previous_name_path) ||
      (Exists(new_name_path)) && previous_name != new_name) {
    return false;
  }

  std::filesystem::rename(previous_name_path, new_name_path);
  return true;
}

bool Remove(const std::filesystem::path& path, bool is_recursive) {
  if (!is_recursive && IsDirectory(path) && !IsEmpty(path)) {
    return false;
  } else {
    return std::filesystem::remove_all(path);
  }
}

std::vector<std::string> ListDirectories(const std::filesystem::path& path,
                                         bool is_sorted) {
  if (!Exists(path) || IsFile(path)) {
    return std::vector<std::string>();
  }

  std::vector<std::string> list;
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

std::vector<std::string> ListFiles(const std::filesystem::path& path,
                                   bool is_sorted) {
  if (!Exists(path)) {
    return std::vector<std::string>();
  }

  std::vector<std::string> list;

  if (IsFile(path)) {
    list.push_back(path.string());
    return list;
  }

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

std::vector<std::string> ListAll(const std::filesystem::path& path,
                                 bool is_sorted) {
  if (!Exists(path)) {
    return std::vector<std::string>();
  }

  std::vector<std::string> list;
  std::filesystem::directory_iterator end_it;

  for (std::filesystem::directory_iterator it{path}; it != end_it; ++it) {
    list.push_back(it->path().generic_string());
  }

  if (is_sorted) {
    std::sort(list.begin(), list.end());
  }

  return list;
}

std::filesystem::path GetCurrentDirectoryAsPath() {
  return std::filesystem::current_path();
}

std::string GetCurrentDirectory() {
  return std::filesystem::current_path().generic_string();
}

std::string GetDirectoryPath(const std::filesystem::path& path) {
  const auto is_directory{IsDirectory(path)};
  const auto is_file{IsFile(path)};

  if (!is_directory && !is_file) {
    return "";
  }

  if (is_directory) {
    return path.string();
  }

  return GetParentPath(path);
}

std::string GetName(std::string_view path) {
  return std::string{GetNameView(path)};
}

std::string_view GetNameView(std::string_view path) {
  return path.substr(path.find_last_of('/') + 1, path.size());
}

std::string GetExtension(const std::filesystem::path& path,
                         bool is_force_lowercase) {
  auto extension{path.extension().generic_string()};
  extension.erase(0, 1);

  if (is_force_lowercase) {
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](u8 c) { return std::tolower(c); });
  }

  return extension;
}

void ReplaceExtension(std::string_view extension, std::string& path,
                      bool is_force_lowercase) {
  if (path.empty()) {
    return;
  }

  auto dot_pos{string::GetLastNthPos(path, '.', 1)};

  if (dot_pos == kInvalidIndex) {
    return;
  }

  if (extension.empty()) {
    path.resize(dot_pos);
    return;
  }

  auto anchor{dot_pos + 1};
  std::string_view path_view{path};
  path_view = path_view.substr(0, anchor);
  auto new_size{path_view.size() + extension.size()};

  if (new_size != path.size()) {
    path.resize(new_size);
  }

  if (is_force_lowercase) {
    for (uindex i{0}; i < extension.size(); ++i) {
      path[anchor + i] = std::tolower(extension[i]);
    }
  } else {
    for (uindex i{0}; i < extension.size(); ++i) {
      path[anchor + i] = extension[i];
    }
  }
}

std::string ReplaceExtensionToCopy(std::string_view extension,
                                   std::string_view path,
                                   bool is_force_lowercase) {
  std::string copy{path};
  ReplaceExtension(extension, copy, is_force_lowercase);
  return copy;
}

std::string GetNormalizedPath(const std::filesystem::path& path) {
  return path.lexically_normal().generic_string();
}

std::string GetAbsolutePath(const std::filesystem::path& relative_path) {
  return relative_path.generic_string();
}

std::string GetRelativePath(const std::filesystem::path& from_path,
                            const std::filesystem::path& to_path) {
  return from_path.lexically_relative(to_path).generic_string();
}

std::string GetRelativePath(const std::filesystem::path& absolute_path) {
  return GetRelativePath(absolute_path,
                         std::filesystem::path{GetCurrentDirectory()});
}

std::string GetNativePath(const std::filesystem::path& path) {
  return path.string();
}

std::string_view GetParentPathView(std::string_view current_path) {
  if (current_path.empty()) {
    return std::string_view{};
  }

  uindex index{current_path.size() - 1};

  if (index > 0 && current_path[index] == '/') {
    current_path = current_path.substr(0, index);
    --index;
  }

  while (index > 0 && current_path[index] != '/') {
    --index;
  }

  if (index == 0) {
    if (current_path[0] == '/') {
      return current_path.substr(0, 1);
    }

    return current_path;
  }

  return current_path.substr(0, index);
}

bool IsDirectory(const std::filesystem::path& path) {
  return std::filesystem::is_directory(path);
}

bool IsFile(const std::filesystem::path& path) {
  return std::filesystem::is_regular_file(path);
}

bool IsAbsolute(const std::filesystem::path& path) {
  return path.is_absolute();
}

bool IsRelative(const std::filesystem::path& path) {
  return path.is_relative();
}

bool Exists(const std::filesystem::path& path) {
  return std::filesystem::exists(path);
}

bool IsEmpty(const std::filesystem::path& path) {
  return std::filesystem::is_empty(path);
}

std::string Append(std::string_view path_a, std::string_view path_b) {
  // Error cases: empty strings.
  if (path_a.empty()) {
    if (path_b.empty()) {
      return std::string{};
    }

    return std::string{path_b};
  }

  auto anchor{string::GetLastNonCharacterIndex(path_a, '/')};

  // Case: first character of path_a is /.
  if (anchor == kInvalidIndex) {
    anchor = 0;
  }

  path_a = path_a.substr(0, anchor + 1);
  anchor = string::GetFirstDifferentCharacterIndex(path_b, '/');

  // Case: path_b is root.
  if (anchor == kInvalidIndex) {
    // Return copy of path_a without trailing /.
    return std::string{path_a};
  }

  path_b = path_b.substr(anchor, path_b.size() - anchor);
  std::string path{};

  // Add 1 extra for added /.
  path.resize(path_a.size() + path_b.size() + 1);
  uindex path_index{0};

  for (uindex i{0}; i < path_a.size(); ++i) {
    path[path_index++] = path_a[i];
  }

  path[path_index++] = '/';

  for (uindex i{0}; i < path_b.size(); ++i) {
    path[path_index++] = path_b[i];
  }

  return path;
}

void RemoveTrailingSlashes(std::string& str) {
  str.erase(std::find_if(str.rbegin(), str.rend(),
                         [](schar character) { return character != '/'; })
                .base(),
            str.end());
}

void RemoveLeadingSlashes(std::string& str) {
  str.erase(str.begin(),
            std::find_if(str.begin(), str.end(),
                         [](schar character) { return character != '/'; }));
}

f64 GetLastModificationTime(const schar* path) {
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

std::string GetChecksum(std::string_view path) {
  if (!IsFile(path)) {
    return "";
  }

  auto file{std::ifstream(path.data(), std::ios::binary)};
  return hash::HashSha256(file);
}
}  // namespace filesystem
}  // namespace utils
}  // namespace comet
