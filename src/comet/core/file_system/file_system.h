// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_FILE_SYSTEM_FILE_SYSTEM_H_
#define COMET_COMET_CORE_FILE_SYSTEM_FILE_SYSTEM_H_

#include <vector>

#include "comet/core/essentials.h"

#ifdef COMET_MSVC
#include "comet/core/windows.h"

#ifdef COMET_WIDE_TCHAR
#define MSVC_CREATE_DIRECTORY CreateDirectoryW
#define MSVC_REMOVE_DIRECTORY RemoveDirectoryW
#define MSVC_GET_CURRENT_DIRECTORY GetCurrentDirectoryW
#define MSVC_GET_FULL_PATH_NAME GetFullPathNameW
#define MSVC_MOVE_FILE MoveFileW
#define MSVC_DELETE_FILE DeleteFileW
#define MSVC_FIND_FIRST_FILE FindFirstFileW
#define MSVC_FIND_NEXT_FILE FindNextFileW
#define MSVC_GET_FILE_ATTRIBUTES GetFileAttributesW
#define MSVC_GET_FILE_ATTRIBUTES_EX GetFileAttributesExW
#define MSVC_WIN32_FIND_DATA WIN32_FIND_DATAW
#else
#define MSVC_CREATE_DIRECTORY CreateDirectoryA
#define MSVC_REMOVE_DIRECTORY RemoveDirectoryA
#define MSVC_GET_CURRENT_DIRECTORY GetCurrentDirectoryA
#define MSVC_GET_FULL_PATH_NAME GetFullPathNameA
#define MSVC_MOVE_FILE MoveFileA
#define MSVC_DELETE_FILE DeleteFileA
#define MSVC_FIND_FIRST_FILE FindFirstFileA
#define MSVC_FIND_NEXT_FILE FindNextFileA
#define MSVC_GET_FILE_ATTRIBUTES GetFileAttributesA
#define MSVC_GET_FILE_ATTRIBUTES_EX GetFileAttributesExA
#define MSVC_WIN32_FIND_DATA WIN32_FIND_DATAA
#endif  // COMET_WIDE_TCHAR
#endif  // COMET_MSVC

#ifndef COMET_WINDOWS
#include <dirent.h>
#endif  // !COMET_WINDOWS

#include "comet/core/c_string.h"
#include "comet/core/file_system/slash_helper.h"
#include "comet/core/type/tstring.h"

namespace comet {
constexpr CTStringView kDotFolderName{COMET_TCHAR(".")};
constexpr CTStringView kDotDotFolderName{COMET_TCHAR("..")};
constexpr auto kMaxPathLength{4096};

namespace internal {
const tchar* GetTmpCopyWithNormalizedSlashes(CTStringView str);
const tchar* GetNextElementForRelativePath(const tchar*& cursor,
                                           const tchar* path_end);
}  // namespace internal

bool OpenFileToWriteTo(CTStringView path, std::ofstream& out_file,
                       bool is_append = false, bool is_binary = false);
bool OpenFileToReadFrom(CTStringView path, std::ifstream& in_file,
                        bool is_at_end = false, bool is_binary = false);
void CloseFile(std::ofstream& file);
bool WriteBinaryToFile(CTStringView path, const u8* buff, usize buff_len,
                       bool is_append = false);
bool WriteBinaryToFile(CTStringView path, const u8* buff, usize buff_len,
                       bool is_append);
bool ReadBinaryFromFile(CTStringView path, std::vector<u8>& buff);
void CloseFile(std::ifstream& file);
bool WriteStrToFile(CTStringView path, const schar* buff,
                    bool is_append = false);
bool ReadStrFromFile(CTStringView path, schar* buff, usize buff_len,
                     usize* out_len = nullptr);
bool GetLine(std::istream& stream, schar* buff, usize buff_len,
             usize* out_len = nullptr);
bool CreateFile(CTStringView path, bool is_recursive = false);
bool CreateDirectory(CTStringView path, bool is_recursive = false);
bool Move(CTStringView previous_name, CTStringView new_name);
bool Remove(CTStringView path, bool is_recursive = false);
TString GetCurrentDirectory();
TString GetDirectoryPath(CTStringView path);
TString GetName(CTStringView path);
TString GetExtension(CTStringView path, bool is_force_lowercase = true);
void ReplaceExtension(CTStringView extension, TString& path,
                      bool is_force_lowercase = true);
TString GetNormalizedPath(CTStringView path);
TString GetAbsolutePath(CTStringView relative_path);
TString GetRelativePath(CTStringView from, CTStringView to);
TString GetRelativePath(CTStringView absolute_path);
TString GetParentPath(CTStringView current_path);
bool IsDirectory(CTStringView path);
bool IsFile(CTStringView path);

enum class RootType {
  Unknown = 0,
  Unix,
  WindowsDriveLetter,
  WindowsExtended,
  WindowsUnc,
  Invalid
};

const schar* GetRootTypeLabel(RootType root_type);
RootType GetRootType(CTStringView path);
bool IsAbsolute(CTStringView path);
bool IsRelative(CTStringView path);
bool Exists(CTStringView path);
bool IsEmpty(CTStringView path);
void AppendTo(CTStringView to_append, tchar* buff, usize buff_len,
              usize* out_len);
void Append(CTStringView path_a, CTStringView path_b, tchar* buff,
            usize buff_len, usize* out_len = nullptr);
TString Append(CTStringView path_a, CTStringView path_b);
void RemoveTrailingSlashes(TString& path);
f64 GetLastModificationTime(CTStringView path);
void GetChecksum(CTStringView path, schar* checksum, usize checksum_len);

template <typename DirectoryCallback>
void ForEachDirectory(CTStringView path, DirectoryCallback callback) {
#ifdef COMET_MSVC
  tchar buff[kMaxPathLength]{COMET_TCHAR('\0')};
  Append(path, COMET_TCHAR("\\*"), buff, kMaxPathLength);

  MSVC_WIN32_FIND_DATA find_data;
  auto handle{MSVC_FIND_FIRST_FILE(buff, &find_data)};

  if (handle == INVALID_HANDLE_VALUE) {
    return;
  }

  do {
    if (!(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
      continue;
    }

    auto file_name_len{GetLength(find_data.cFileName)};

    if (AreStringsEqual(find_data.cFileName, file_name_len,
                        kDotFolderName.GetCTStr(),
                        kDotFolderName.GetLength()) ||
        AreStringsEqual(find_data.cFileName, file_name_len,
                        kDotDotFolderName.GetCTStr(),
                        kDotDotFolderName.GetLength())) {
      continue;
    }

    buff[0] = COMET_TCHAR('\0');
    Append(path, find_data.cFileName, buff, kMaxPathLength);
    callback(buff);
  } while (MSVC_FIND_NEXT_FILE(handle, &find_data) != 0);

  FindClose(handle);
#else
  auto* dir{opendir(path)};

  if (dir == nullptr) {
    return;
  }

  struct dirent* entry{nullptr};
  tchar buff[kMaxPathLength]{COMET_TCHAR('\0')};
  const auto path_len{path.GetLength()};

  while ((entry = readdir(dir)) != nullptr) {
    const auto dir_len{GetLength(entry->d_name)};

    if (entry->d_type != DT_DIR ||
        AreStringsEqual(entry->d_name, dir_len, kDotFolderName.GetCTStr(),
                        kDotFolderName.GetLength()) ||
        AreStringsEqual(entry->d_name, dir_len, kDotDotFolderName.GetCTStr(),
                        kDotDotFolderName.GetLength())) {
      continue;
    }

    CTStringView folder_name{entry->d_name, entry->d_reclen};
    Append(path, folder_name, buff, kMaxPathLength);
    buff[path_len + entry->d_reclen + 1] = COMET_TCHAR('\0');
    callback(buff);
  }

  closedir(dir);
#endif  // COMET_MSVC
}

template <typename FileCallback>
void ForEachFile(CTStringView path, FileCallback callback) {
#ifdef COMET_MSVC
  tchar buff[kMaxPathLength]{COMET_TCHAR('\0')};
  Append(path, COMET_TCHAR("\\*"), buff, kMaxPathLength);

  MSVC_WIN32_FIND_DATA find_data;
  auto handle{MSVC_FIND_FIRST_FILE(buff, &find_data)};

  if (handle == INVALID_HANDLE_VALUE) {
    return;
  }

  do {
    if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      continue;
    }

    buff[0] = COMET_TCHAR('\0');
    Append(path, find_data.cFileName, buff, kMaxPathLength);
    callback(buff);
  } while (MSVC_FIND_NEXT_FILE(handle, &find_data) != 0);

  FindClose(handle);
#else
  auto* dir{opendir(path)};

  if (dir == nullptr) {
    return;
  }

  struct dirent* entry{nullptr};
  tchar buff[kMaxPathLength]{COMET_TCHAR('\0')};
  const auto path_len{path.GetLength()};

  while ((entry = readdir(dir)) != nullptr) {
    if (entry->d_type != DT_REG) {
      continue;
    }

    CTStringView folder_name{entry->d_name, entry->d_reclen};
    Append(path, folder_name, buff, kMaxPathLength);
    buff[path_len + entry->d_reclen + 1] = COMET_TCHAR('\0');
    callback(buff);
  }

  closedir(dir);
#endif  // COMET_MSVC
}

template <typename FileCallback>
void ForEachFileAndDirectory(CTStringView path, FileCallback callback) {
#ifdef COMET_MSVC
  tchar buff[kMaxPathLength]{COMET_TCHAR('\0')};
  Append(path, COMET_TCHAR("\\*"), buff, kMaxPathLength);

  MSVC_WIN32_FIND_DATA find_data;
  auto handle{MSVC_FIND_FIRST_FILE(buff, &find_data)};

  if (handle == INVALID_HANDLE_VALUE) {
    return;
  }

  do {
    if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      auto file_name_len{GetLength(find_data.cFileName)};

      if (AreStringsEqual(find_data.cFileName, file_name_len,
                          kDotFolderName.GetCTStr(),
                          kDotFolderName.GetLength()) ||
          AreStringsEqual(find_data.cFileName, file_name_len,
                          kDotDotFolderName.GetCTStr(),
                          kDotDotFolderName.GetLength())) {
        continue;
      }
    }

    buff[0] = COMET_TCHAR('\0');
    Append(path, find_data.cFileName, buff, kMaxPathLength);
    callback(buff);
  } while (MSVC_FIND_NEXT_FILE(handle, &find_data) != 0);

  FindClose(handle);
#else
  auto* dir{opendir(path)};

  if (dir == nullptr) {
    return;
  }

  struct dirent* entry{nullptr};
  tchar buff[kMaxPathLength]{COMET_TCHAR('\0')};
  const auto path_len{path.GetLength()};

  while ((entry = readdir(dir)) != nullptr) {
    CTStringView folder_name{entry->d_name, entry->d_reclen};
    Append(path, folder_name, buff, kMaxPathLength);
    buff[path_len + entry->d_reclen + 1] = COMET_TCHAR('\0');
    callback(buff);
  }

  closedir(dir);
#endif  // COMET_MSVC
}

void NormalizeSlashes(tchar* str, usize len);
void NormalizeSlashes(tchar* str);
void NormalizeSlashes(TString& str);
void MakeNative(tchar* str, usize len);
void MakeNative(tchar* str);
void MakeNative(TString& str);
void Clean(tchar* str, usize len);
void Clean(tchar* str);
void Clean(TString& str);
const tchar* GetTmpTChar(const schar* str, usize len);
const tchar* GetTmpTChar(const schar* str);
const tchar* GetTmpTChar(const wchar* str, usize len);
const tchar* GetTmpTChar(const wchar* str);
usize GetSize(CTStringView path);
}  // namespace comet

#endif  // COMET_COMET_CORE_FILE_SYSTEM_FILE_SYSTEM_H_
