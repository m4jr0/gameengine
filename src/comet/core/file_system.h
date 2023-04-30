// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_FILE_SYSTEM_H_
#define COMET_COMET_CORE_FILE_SYSTEM_H_

#include "comet_precompile.h"

#ifdef COMET_MSVC
// Special case to prevent the IDE from being confused.
#undef CreateFile
#undef CreateDirectory
#undef GetCurrentDirectory
#endif  // COMET_MSVC

#include "comet/core/string.h"

namespace comet {
bool OpenBinaryFileToWriteTo(const schar* path, std::ofstream& out_file,
                             bool is_append = false);
bool OpenBinaryFileToWriteTo(const std::string& path, std::ofstream& out_file,
                             bool is_append = false);
bool OpenBinaryFileToReadFrom(const schar* path, std::ifstream& in_file,
                              bool is_at_end = false);
bool OpenBinaryFileToReadFrom(const std::string& path, std::ifstream& in_file,
                              bool is_at_end = false);
void CloseFile(std::ofstream& file);
bool WriteBinaryToFile(const schar* path, const u8* buffer, uindex buffer_size,
                       bool is_append = false);
bool WriteBinaryToFile(const std::string& path, const u8* buffer,
                       uindex buffer_size, bool is_append = false);
bool WriteBinaryToFile(const schar* path, const u8* buffer, uindex buffer_size,
                       bool is_append);
bool ReadBinaryFromFile(const std::string& path, std::vector<u8>& buffer);
void CloseFile(std::ifstream& file);
bool WriteStrToFile(const schar* path, std::string_view buffer,
                    bool is_append = false);
bool WriteStrToFile(const std::string& path, std::string_view buffer,
                    bool is_append = false);
bool ReadStrFromFile(const schar* path, std::string& buffer);
bool ReadStrFromFile(const std::string& path, std::string& buffer);
bool CreateFile(const schar* path, bool is_recursive = false);
bool CreateFile(const std::string& path, bool is_recursive = false);
bool CreateDirectory(std::string_view path, bool is_recursive = false);
bool Move(std::string_view previous_name, std::string_view new_name);
bool Remove(const std::filesystem::path& path, bool is_recursive = false);

template <typename Path>
bool Remove(Path&& str, bool is_recursive = false) {
  const std::filesystem::path path{std::forward<Path>(str)};
  return Remove(path, is_recursive);
}

std::vector<std::string> ListDirectories(const std::filesystem::path& path,
                                         bool is_recursive = false);

template <typename Path>
std::vector<std::string> ListDirectories(Path&& str_path,
                                         bool is_sorted = false) {
  const std::filesystem::path path{std::forward<Path>(str_path)};
  return ListDirectories(path, is_sorted);
}

std::vector<std::string> ListFiles(const std::filesystem::path& path,
                                   bool is_sorted = false);

template <typename Path>
std::vector<std::string> ListFiles(Path&& str_path, bool is_sorted = false) {
  const std::filesystem::path path{std::forward<Path>(str_path)};
  return ListFiles(path, is_sorted);
}

std::vector<std::string> ListAll(const std::filesystem::path& path,
                                 bool is_sorted = false);

template <typename Path>
std::vector<std::string> ListAll(Path&& str_path, bool is_sorted = false) {
  const std::filesystem::path path{std::forward<Path>(str_path)};
  return ListAll(path, is_sorted);
}

std::filesystem::path GetCurrentDirectoryAsPath();
std::string GetCurrentDirectory();

std::string GetDirectoryPath(const std::filesystem::path& path);

template <typename Path>
std::string GetDirectoryPath(Path&& str_path) {
  const std::filesystem::path path{std::forward<Path>(str_path)};
  return GetDirectoryPath(path);
}

std::string GetName(std::string_view path);
std::string_view GetNameView(std::string_view path);

std::string GetExtension(const std::filesystem::path& path,
                         bool is_force_lowercase = true);

template <typename Path>
std::string GetExtension(Path&& str_path, bool is_force_lowercase = true) {
  const std::filesystem::path path{std::forward<Path>(str_path)};
  return GetExtension(path, is_force_lowercase);
}

void ReplaceExtension(std::string_view extension, std::string& path,
                      bool is_force_lowercase = true);
std::string ReplaceExtensionToCopy(std::string_view extension,
                                   std::string_view path,
                                   bool is_force_lowercase = true);

std::string GetNormalizedPath(const std::filesystem::path& path);

template <typename Path>
std::string GetNormalizedPath(Path&& str_path) {
  const std::filesystem::path path{std::forward<Path>(str_path)};
  // Call non-templated function.
  return GetNormalizedPath(path);
}

std::string GetAbsolutePath(const std::filesystem::path& relative_path);

template <typename Path>
std::string GetAbsolutePath(Path&& str_path) {
  const std::filesystem::path path{std::forward<Path>(str_path)};
  // Call non-templated function.
  return GetAbsolutePath(path);
}

std::string GetRelativePath(const std::filesystem::path& from_path,
                            const std::filesystem::path& to_path);

template <typename FromPath, typename ToPath>
std::string GetRelativePath(FromPath&& str_from_path, ToPath&& str_to_path) {
  const std::filesystem::path from_path{std::forward<FromPath>(str_from_path)};
  const std::filesystem::path to_path{std::forward<ToPath>(str_to_path)};
  return GetRelativePath(from_path, to_path);
}

std::string GetRelativePath(const std::filesystem::path& absolute_path);

template <typename Path>
std::string GetRelativePath(Path&& str_path) {
  const std::filesystem::path path{std::forward<Path>(str_path)};
  const auto current_directory_path{GetCurrentDirectoryAsPath()};
  // Call non-templated function.
  return GetRelativePath(path, current_directory_path);
}

std::string GetNativePath(const std::filesystem::path& path);

template <typename Path>
std::string GetNativePath(Path&& str_path) {
  const std::filesystem::path path{std::forward<Path>(str_path)};
  // Call non-templated function.
  return GetNativePath(path);
}

template <typename Path>
std::string GetParentPath(Path&& str_path) {
  auto absolute_path{GetAbsolutePath(std::forward<Path>(str_path))};

  if (absolute_path.size() <= 1) {
    return absolute_path;
  }

  auto absolute_view{std::string_view{absolute_path}};

  if (absolute_view[absolute_view.size() - 1] == '/') {
    // No allocation here.
    absolute_view = absolute_view.substr(0, absolute_view.size() - 1);
  }

  const auto last_index{GetLastNthPos(absolute_view, '/', 1)};

  if (last_index == kInvalidIndex) {
    return absolute_path;
  }

  absolute_path.resize(last_index);
  return absolute_path;
}

std::string_view GetParentPathView(std::string_view current_path);

bool IsDirectory(const std::filesystem::path& path);

template <typename Path>
bool IsDirectory(Path&& str_path) {
  const std::filesystem::path path{std::forward<Path>(str_path)};
  // Call non-templated function.
  return std::filesystem::is_directory(path);
}

bool IsFile(const std::filesystem::path& path);

template <typename Path>
bool IsFile(Path&& str_path) {
  const std::filesystem::path path{std::forward<Path>(str_path)};
  // Call non-templated function.
  return std::filesystem::is_regular_file(path);
}

bool IsAbsolute(const std::filesystem::path& path);

template <typename Path>
bool IsAbsolute(Path&& str_path) {
  const std::filesystem::path path{std::forward<Path>(str_path)};
  // Call non-templated function.
  return IsAbsolute(path);
}

bool IsRelative(const std::filesystem::path& path);

template <typename Path>
bool IsRelative(Path&& str_path) {
  const std::filesystem::path path{std::forward<Path>(str_path)};
  // Call non-templated function.
  return IsRelative(path);
}

bool Exists(const std::filesystem::path& path);

template <typename Path>
bool Exists(Path&& str_path) {
  const std::filesystem::path path{std::forward<Path>(str_path)};
  // Call non-templated function.
  return Exists(path);
}

bool IsEmpty(const std::filesystem::path& path);

template <typename Path>
bool IsEmpty(Path&& str_path) {
  const std::filesystem::path path{std::forward<Path>(str_path)};
  // Call non-templated function.
  return IsEmpty(path);
}

std::string Append(std::string_view path_a, std::string_view path_b);
void RemoveTrailingSlashes(std::string& path);
void RemoveLeadingSlashes(std::string& path);
f64 GetLastModificationTime(const schar* path);
f64 GetLastModificationTime(const std::string& path);
std::string GetChecksum(std::string_view path);
}  // namespace comet

#endif  // COMET_COMET_CORE_FILE_SYSTEM_H_
