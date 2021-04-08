// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_UTILS_FILE_SYSTEM_HPP_
#define COMET_UTILS_FILE_SYSTEM_HPP_

constexpr auto kLoggerCometUtilsFileSystem = "comet_utils";

#include "boost/filesystem.hpp"
#include "boost/filesystem/fstream.hpp"
#include "comet_precompile.h"

namespace comet {
namespace filesystem {
bool WriteToFile(const std::string &, const std::string &, bool = false);
bool ReadFile(const std::string &, std::string *);
bool CreateFile(const std::string &, bool = false);
bool CreateDirectory(const std::string &, bool = false);
bool Move(const std::string &, const std::string &);
bool Remove(const std::string &, bool = false);
std::vector<std::string> ListDirectories(const std::string &, bool = false);
std::vector<std::string> ListFiles(const std::string &, bool = false);
std::vector<std::string> ListAll(const std::string &, bool = false);
std::string GetCurrentDirectory();
std::string GetAbsolutePath(const std::string &);
std::string GetRelativePath(const std::string &);
std::string GetDirectoryPath(const std::string &);
std::string GetName(const std::string &);
std::string GetExtension(const std::string &);
std::string GetParentPath(const std::string &);
std::string GetNativePath(const std::string &);
bool IsDirectory(const std::string &);
bool IsFile(const std::string &);
bool IsRelative(const std::string &);
bool IsAbsolute(const std::string &);
bool IsExist(const std::string &);
bool IsEmpty(const std::string &);
std::string Append(const std::string &, const std::string &);
std::string GetNormalizedPath(const std::string &);
std::string GetRelativePath(const std::string &, const std::string &);
void RemoveTrailingSlashes(std::string &);
void RemoveLeadingSlashes(std::string &);
double GetLastModificationTime(const std::string &);
std::string GetChecksum(const std::string &);
}  // namespace filesystem
}  // namespace comet

#endif  // COMET_UTILS_FILE_SYSTEM_HPP_
