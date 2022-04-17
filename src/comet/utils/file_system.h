// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_UTILS_FILE_SYSTEM_H_
#define COMET_COMET_UTILS_FILE_SYSTEM_H_

#include "boost/filesystem.hpp"
#include "boost/filesystem/fstream.hpp"
#include "comet_precompile.h"

namespace comet {
namespace utils {
namespace filesystem {
bool WriteToFile(const std::string& path, const std::string& buffer,
                 bool is_append = false);
bool ReadFile(const std::string& path, std::string*);
bool CreateFile(const std::string& path, bool is_recursive = false);
bool CreateDirectory(const std::string& path, bool is_recursive = false);
bool Move(const std::string& previous_name, const std::string& new_name);
bool Remove(const std::string& path, bool = false);
std::vector<std::string> ListDirectories(const std::string& path,
                                         bool is_recursive = false);
std::vector<std::string> ListFiles(const std::string& directory_path,
                                   bool is_sorted = false);
std::vector<std::string> ListAll(const std::string& path,
                                 bool is_sorted = false);
std::string GetCurrentDirectory();
std::string GetAbsolutePath(const std::string& path);
std::string GetRelativePath(const std::string& path);
std::string GetDirectoryPath(const std::string& path);
std::string GetName(const std::string& path);
std::string GetExtension(const std::string& path);
std::string GetParentPath(const std::string& path);
std::string GetNativePath(const std::string& path);
bool IsDirectory(const std::string& path);
bool IsFile(const std::string& path);
bool IsRelative(const std::string& path);
bool IsAbsolute(const std::string& path);
bool IsExist(const std::string& path);
bool IsEmpty(const std::string& path);
std::string Append(const std::string& path_a, const std::string& path_b);
std::string GetNormalizedPath(const std::string& path);
std::string GetRelativePath(const std::string& from_path,
                            const std::string& to_path);
void RemoveTrailingSlashes(std::string& path);
void RemoveLeadingSlashes(std::string& path);
double GetLastModificationTime(const std::string& path);
std::string GetChecksum(const std::string& path);
}  // namespace filesystem
}  // namespace utils
}  // namespace comet

#endif  // COMET_COMET_UTILS_FILE_SYSTEM_H_
