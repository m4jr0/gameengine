// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_UTILS_FILE_SYSTEM_H_
#define COMET_COMET_UTILS_FILE_SYSTEM_H_

#include "comet_precompile.h"

namespace comet {
namespace utils {
namespace filesystem {
bool OpenBinaryFileToWriteTo(const char* path, std::ofstream& out_file,
                             bool is_append = false);
bool OpenBinaryFileToWriteTo(const std::string& path, std::ofstream& out_file,
                             bool is_append = false);
bool OpenBinaryFileToReadFrom(const char* path, std::ifstream& in_file);
bool OpenBinaryFileToReadFrom(const std::string& path, std::ifstream& in_file);
void CloseFile(std::ofstream& file);
bool WriteBinaryToFile(const char* path, const char* buffer, uindex buffer_size,
                       bool is_append = false);
bool WriteBinaryToFile(const std::string& path, const char* buffer,
                       uindex buffer_size, bool is_append = false);
void CloseFile(std::ifstream& file);
bool WriteStrToFile(const char* path, const char* buffer,
                    bool is_append = false);
bool WriteStrToFile(const char* path, std::string& buffer,
                    bool is_append = false);
bool WriteStrToFile(const std::string& path, const char* buffer,
                    bool is_append = false);
bool WriteStrToFile(const std::string& path, const std::string& buffer,
                    bool is_append = false);
bool ReadStrFromFile(const char* path, std::string& buffer);
bool ReadStrFromFile(const std::string& path, std::string& buffer);
bool CreateFile(const char* path, bool is_recursive = false);
bool CreateFile(const std::string& path, bool is_recursive = false);
bool CreateDirectory(const char* path, bool is_recursive = false);
bool CreateDirectory(const std::string& path, bool is_recursive = false);
bool Move(const char* previous_name, const char* new_name);
bool Move(const char* previous_name, const std::string& new_name);
bool Move(const std::string& previous_name, const char* new_name);
bool Move(const std::string& previous_name, const std::string& new_name);
bool Remove(const char* path, bool = false);
bool Remove(const std::string& path, bool = false);
std::vector<std::string> ListDirectories(const char* path,
                                         bool is_recursive = false);
std::vector<std::string> ListDirectories(const std::string& path,
                                         bool is_recursive = false);
std::vector<std::string> ListFiles(const char* directory_path,
                                   bool is_sorted = false);
std::vector<std::string> ListFiles(const std::string& directory_path,
                                   bool is_sorted = false);
std::vector<std::string> ListAll(const char* path, bool is_sorted = false);
std::vector<std::string> ListAll(const std::string& path,
                                 bool is_sorted = false);
std::string GetCurrentDirectory();

template <typename Path>
std::string GetAbsolutePath(Path&& relative_path) {
  return std::filesystem::absolute(std::forward<Path>(relative_path))
      .generic_string();
}

template <typename FromPath, typename ToPath>
std::string GetRelativePath(FromPath&& from_path, ToPath&& to_path) {
  const auto from_path_obj{
      std::filesystem::path(std::forward<FromPath>(from_path))};
  const auto to_path_obj{std::filesystem::path(std::forward<ToPath>(to_path))};

  return from_path_obj.lexically_relative(to_path_obj).generic_string();
}

template <typename Path>
std::string GetRelativePath(Path&& absolute_path) {
  return GetRelativePath(std::forward<Path>(absolute_path),
                         GetCurrentDirectory());
}

std::string GetDirectoryPath(const char* path);
std::string GetDirectoryPath(const std::string& path);
std::string GetName(const std::string& path);

template <typename Path>
std::string GetExtension(Path&& path, bool is_force_lowercase = true) {
  auto extension{std::filesystem::path(std::forward<Path>(path))
                     .extension()
                     .generic_string()};

  extension.erase(0, 1);

  if (is_force_lowercase) {
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](u8 c) { return std::tolower(c); });
  }

  return extension;
}

std::string ReplaceExtension(const std::string& path, std::string extension,
                             bool is_force_lowercase = true);

template <typename Path>
std::string GetParentPath(Path&& current_path) {
  auto absolute_path{GetAbsolutePath(std::forward<Path>(current_path))};
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

template <typename Path>
std::string GetNativePath(Path&& path) {
  return std::filesystem::path(std::forward<Path>(path)).string();
}

template <typename Path>
bool IsDirectory(Path path) {
  return std::filesystem::is_directory(std::forward<Path>(path));
}

template <typename Path>
bool IsFile(Path path) {
  return std::filesystem::is_regular_file(std::forward<Path>(path));
}

template <typename Path>
bool IsAbsolute(Path path) {
  return std::filesystem::path(std::forward<Path>(path)).is_absolute();
}

template <typename Path>
bool IsRelative(Path path) {
  return std::filesystem::path(std::forward<Path>(path)).is_relative();
}

template <typename Path>
bool Exists(Path path) {
  return std::filesystem::exists(std::forward<Path>(path));
}

template <typename Path>
bool IsEmpty(Path path) {
  return std::filesystem::is_empty(std::forward<Path>(path));
}

std::string Append(const std::string& path_a, const std::string& path_b);

template <typename Path>
std::string GetNormalizedPath(Path&& path) {
  return std::filesystem::path(std::forward<Path>(path))
      .lexically_normal()
      .generic_string();
}

void RemoveTrailingSlashes(std::string& path);
void RemoveLeadingSlashes(std::string& path);
f64 GetLastModificationTime(const char* path);
f64 GetLastModificationTime(const std::string& path);
std::string GetChecksum(const char* path);
std::string GetChecksum(const std::string& path);
}  // namespace filesystem
}  // namespace utils
}  // namespace comet

#endif  // COMET_COMET_UTILS_FILE_SYSTEM_H_
