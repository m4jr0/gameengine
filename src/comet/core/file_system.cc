// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "file_system.h"

#include "comet/core/generator.h"
#include "comet/core/hash.h"

namespace comet {
namespace internal {
const tchar* GetTmpNormalizedCopy(CTStringView str) {
#ifdef COMET_WINDOWS
  auto* tmp{GenerateForOneFrame<tchar>(str.GetLength() + 1)};

  for (uindex i{0}; i < str.GetLength(); ++i) {
    tchar c{str[i]};

    if (c == COMET_TCHAR('\\')) {
      c = COMET_TCHAR('/');
    }

    tmp[i] = c;
  }

  tmp[str.GetLength()] = COMET_TCHAR('\0');
  return tmp;
#else
  return str.GetCTStr();
#endif  // COMET_WINDOWS
}
}  // namespace internal

bool OpenFileToWriteTo(CTStringView path, std::ofstream& out_file,
                       bool is_append, bool is_binary) {
  auto mode{
      static_cast<std::ios_base::openmode>(std::ios::out | std::ios::failbit)};

  if (is_binary) {
    mode |= std::ios::binary;
  }

  if (!is_append) {
    mode |= std::ios::trunc;
  } else {
    mode |= std::ios::app;
  }

  out_file.open(path, mode);
  return true;
}

bool OpenFileToReadFrom(CTStringView path, std::ifstream& in_file,
                        bool is_at_end, bool is_binary) {
  try {
    // Weird case to keep MSVC happy.
#ifdef COMET_MSVC
    auto mode{static_cast<s32>(std::ios::in)};
#else
    auto mode{std::ios::in};
#endif  // COMET_MSVC

    if (is_binary) {
      mode |= std::ios::binary;
    }

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

void CloseFile(std::ofstream& file) { file.close(); }

bool WriteBinaryToFile(CTStringView path, const u8* buff, uindex buff_len,
                       bool is_append) {
  std::ofstream file;

  if (!OpenFileToWriteTo(path, file, is_append)) {
    return false;
  }

  file.write(reinterpret_cast<const schar*>(buff), buff_len);
  CloseFile(file);
  return true;
}

bool ReadBinaryFromFile(CTStringView path, std::vector<u8>& buff) {
  std::ifstream input_stream;

  if (!OpenFileToReadFrom(path, input_stream, true, true)) {
    return false;
  }

  const auto file_size{input_stream.tellg()};
  buff.resize(file_size);
  input_stream.seekg(0);
  input_stream.read(reinterpret_cast<schar*>(buff.data()), file_size);

  CloseFile(input_stream);
  return true;
}

void CloseFile(std::ifstream& file) { file.close(); }

bool WriteStrToFile(CTStringView path, const schar* buff, bool is_append) {
  auto mode{std::ios::binary | std::ios::out};

  if (!is_append) {
    mode |= std::ios::trunc;
  } else {
    mode |= std::ios::app;
  }

  std::ofstream out_file;
  out_file.open(path, mode);
  out_file << buff;
  out_file.close();

  return true;
}

bool ReadStrFromFile(CTStringView path, schar* buff, uindex buff_len,
                     uindex* out_len) {
  std::ifstream input_stream;
  input_stream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  auto is_open{false};
  COMET_ASSERT(buff != nullptr, "Buffer provided is null!");
  COMET_ASSERT(buff_len > 0, "Length of buffer provided is 0!");

  try {
    input_stream.open(path, std::ios::in);
    is_open = input_stream.is_open();
  } catch (std::runtime_error& error) {
    COMET_LOG_UTILS_ERROR("Could not open file at path: ", path,
                          "! Reason: ", error.what());
    is_open = false;
  }

  if (!is_open) {
    buff[0] = '\0';
    return false;
  }

  // Get character count.
  input_stream.ignore(std::numeric_limits<std::streamsize>::max());
  const auto file_size{input_stream.gcount()};
  COMET_ASSERT(buff_len > file_size, "File is too big for buffer: ", file_size,
               " >= ", buff_len, "!");
  input_stream.clear();  // Reset file (EOF flag is set).
  input_stream.seekg(0, std::ios_base::beg);

  COMET_ASSERT(input_stream.read(buff, file_size),
               "Something wrong happened while reading the file!");

  buff[file_size] = '\0';
  input_stream.close();

  if (out_len != nullptr) {
    *out_len = input_stream.gcount();
  }

  return true;
}

bool GetLine(std::istream& stream, schar* buff, uindex buff_len,
             uindex* out_len) {
  COMET_ASSERT(buff != nullptr, "Buffer provided is null!");

  if (buff_len == 0) {
    return false;
  }

  schar c;
  uindex i{0};
  uindex max_line_len{buff_len - 1};

  while (i < max_line_len && stream.good() && stream.get(c) && c != '\n') {
    buff[i++] = c;
  }

  buff[i] = '\0';

  if (out_len != nullptr) {
    *out_len = i - 1;
  }

  return i > 0 && i != buff_len;
}

bool CreateFile(CTStringView path, bool is_recursive) {
  if (path.GetLength() == 0) {
    return false;
  }

  const auto last_character{path[path.GetLength() - 1]};

  if (IsSlash(last_character)) {
    return false;
  }

  const auto directory_path{GetParentPath(path)};

  if (is_recursive) {
    if (!CreateDirectory(directory_path, true)) {
      return false;
    }
  }

  if (!IsDirectory(directory_path) || !Exists(directory_path)) {
    return false;
  }

  WriteStrToFile(directory_path, "", true);
  return true;
}

bool CreateDirectory(CTStringView path, bool is_recursive) {
#ifdef COMET_WINDOWS
  auto is_created{CreateDirectoryW(path, nullptr) ||
                  GetLastError() == ERROR_ALREADY_EXISTS};

  if (!is_recursive || is_created) {
    return is_created;
  }

  return CreateDirectory(GetParentPath(path), true) &&
             CreateDirectoryW(path, nullptr) ||
         GetLastError() == ERROR_ALREADY_EXISTS;
#else
  if (mkdir(path, S_IRWXU | S_IRGRP | S_IXGRP | S_IXOTH) == 0 ||
      errno == EEXIST) {
    return true;
  }

  if (!is_recursive) {
    return false;
  }

  return CreateDirectory(GetParentPath(path), true) && CreateDirectory(path);
#endif  // COMET_WINDOWS
}

bool Move(CTStringView previous_name, CTStringView new_name) {
  if (!Exists(previous_name) ||
      (Exists(new_name)) && previous_name != new_name) {
    return false;
  }

#ifdef COMET_WINDOWS
  return MoveFileW(previous_name, new_name) != 0;
#else
  return rename(previous_name, new_name) == 0;
#endif  // COMET_WINDOWS
}

bool Remove(CTStringView path, bool is_recursive) {
  if (!is_recursive && IsDirectory(path) && !path.IsEmpty()) {
    return false;
  }

  // TODO(m4jr0): Implement custom function.
  return std::filesystem::remove_all(path.GetCTStr());
}

TString GetCurrentDirectory(bool is_clean) {
  // TODO(m4jr0): Implement custom function.
  TString current_directory{std::filesystem::current_path().c_str()};
  Clean(current_directory);
  return current_directory;
}

TString GetDirectoryPath(CTStringView path) {
  const auto is_directory{IsDirectory(path)};
  const auto is_file{IsFile(path)};

  if (!is_directory && !is_file) {
    return TString{};
  }

  if (is_directory) {
    TString directory_path{path.GetCTStr()};
    return directory_path;
  }

  return GetParentPath(path);
}

TString GetName(CTStringView path) {
  if (path.GetLength() == 0) {
    return TString{};
  }

  uindex offset;
  // Normalize path to work with slashes (/) only.
  const auto* normalized_path{internal::GetTmpNormalizedCopy(path)};

  if (IsSlash(path.GetLast())) {
    offset = GetNthToLastIndexOf(normalized_path, path.GetLength(),
                                 COMET_TCHAR('/'), 1);
  } else {
    offset =
        GetLastIndexOf(normalized_path, path.GetLength(), COMET_TCHAR('/'));
  }

  if (offset == kInvalidIndex || offset == path.GetLength() - 1) {
    return TString{};
  }

  ++offset;
  return path.GenerateSubString(offset);
}

TString GetExtension(CTStringView path, bool is_force_lowercase) {
  auto offset{GetLastIndexOf(path, path.GetLength(), COMET_TCHAR('.'))};

  if (offset == kInvalidIndex || offset == path.GetLength() - 1) {
    return TString{};
  }

  ++offset;
  TString extension{};
  extension.Resize(path.GetLength() - offset);
  GetSubString(extension.GetTStr(), path, path.GetLength(), offset);
  extension[extension.GetLength()] = COMET_TCHAR('\0');

  if (is_force_lowercase) {
    for (uindex i{0}; i < extension.GetLength(); ++i) {
      extension[i] = ToLower(extension[i]);
    }
  }

  return extension;
}

void ReplaceExtension(CTStringView extension, TString& path,
                      bool is_force_lowercase) {
  if (path.IsEmpty()) {
    return;
  }

  auto dot_pos{path.GetNthToLastIndexOf(COMET_TCHAR('.'), 1)};

  // Case: no extension. New extension will be appended at the end of the path.
  if (dot_pos == kInvalidIndex) {
    dot_pos = GetLength(path);
  }

  if (extension.IsEmpty()) {
    path.Resize(dot_pos);
    return;
  }

  const auto anchor{dot_pos + 1};
  path.Resize(anchor + extension.GetLength());

  if (is_force_lowercase) {
    for (uindex i{0}; i < extension.GetLength(); ++i) {
      path[anchor + i] = ToLower(extension[i]);
    }
  } else {
    for (uindex i{0}; i < extension.GetLength(); ++i) {
      path[anchor + i] = extension[i];
    }
  }
}

TString GetNormalizedPath(CTStringView path) {
  // TODO(m4jr0): Implement custom function.
  const auto normalized_path{std::filesystem::path{path.GetCTStr()}
                                 .lexically_normal()
                                 .generic_string()};
  return TString{normalized_path.c_str()};
}

TString GetAbsolutePath(CTStringView relative_path) {
  // TODO(m4jr0): Implement custom function.
  const auto absolute_path{
      std::filesystem::path{relative_path.GetCTStr()}.generic_string()};
  TString abs_path{absolute_path.c_str()};
  return abs_path;
}

TString GetRelativePath(CTStringView from, CTStringView to) {
  // TODO(m4jr0): Implement custom function.
  const auto relative_path_tmp{std::filesystem::path{from.GetCTStr()}
                                   .lexically_relative(to.GetCTStr())
                                   .generic_string()};

  TString relative_path{relative_path_tmp.c_str()};
  return relative_path;
}

TString GetRelativePath(CTStringView absolute_path) {
  return GetRelativePath(absolute_path, GetCurrentDirectory());
}

TString GetParentPath(CTStringView current_path) {
  if (current_path.IsEmpty()) {
    return TString{};
  }

  uindex cursor{current_path.GetLength() - 1};

  if (cursor > 0 && IsSlash(current_path[cursor])) {
    --cursor;
  }

  while (cursor > 0 && !IsSlash(current_path[cursor])) {
    --cursor;
  }

  if (cursor == 0) {
    TString parent_path{kNativeSlash};
    return parent_path;
  }

  TString parent_path{current_path.GetCTStr(),
                      current_path.GetCTStr() + cursor};
  Clean(parent_path);
  return parent_path;
}

bool IsSlash(tchar c) {
#ifdef COMET_WINDOWS
  return c == COMET_TCHAR('/') || c == COMET_TCHAR('\\');
#else
  return c == COMET_TCHAR('/');
#endif  // COMET_WINDOWS
  return false;
}

bool IsDirectory(CTStringView path) {
  if (path.IsEmpty()) {
    return false;
  }

#ifdef COMET_WINDOWS
  auto attributes{GetFileAttributesW(path)};
  return attributes != 0xFFFFFFFF && attributes & FILE_ATTRIBUTE_DIRECTORY;
#else
  struct stat status;
  return stat(path, &status) != -1 && S_ISDIR(status.st_mode);
#endif  // COMET_WINDOWS
}

bool IsFile(CTStringView path) {
  if (path.IsEmpty()) {
    return false;
  }

#ifdef COMET_WINDOWS
  auto attributes{GetFileAttributesW(path)};
  return attributes != 0xFFFFFFFF &&
         (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
#else
  struct stat status;
  return stat(path, &status) != -1 && S_ISREG(status.st_mode);
#endif  // COMET_WINDOWS
}

bool IsAbsolute(CTStringView path) {
  if (path.IsEmpty()) {
    return false;
  }

#ifdef COMET_WINDOWS
  // Check for root paths.
  if (IsSlash(path[0])) {
    return true;
  }

  // Check for drive letters.
  if (path.GetLength() >= 3 && IsAlpha(path[0]) && path[1] == ':' &&
      IsSlash(path[2])) {
    return true;
  }

  // Check for extended paths.
  if (path.GetLength() >= 4) {
    tchar tmp[5];
    Copy(tmp, path, 4);
    tmp[4] = COMET_TCHAR('\0');

    if (AreStringsEqual(tmp, 4, COMET_TCHAR("\\\\?\\"), 4)) {
      return true;
    }
  }

  // Check for UNC paths.
  if (path.GetLength() >= 8) {
    tchar tmp[9];
    Copy(tmp, path, 8);
    tmp[8] = COMET_TCHAR('\0');

    if (AreStringsEqual(tmp, 8, COMET_TCHAR("\\\\?\\UNC\\"), 8)) {
      return true;
    }
  }

  return false;
#else
  return IsSlash(path[0]);
#endif  // COMET_WINDOWS
}

bool IsRelative(CTStringView path) {
  if (path.IsEmpty()) {
    return false;
  }

  return !IsAbsolute(path);
}

bool Exists(CTStringView path) {
#ifdef _WIN32
  struct _stat64i32 status;
  return _wstat(path.GetCTStr(), &status) == 0;
#else
  struct stat status;
  return stat(path.GetCTStr(), &status) == 0;
#endif
}

bool IsEmpty(CTStringView path) {
  // TODO(m4jr0): Implement custom function.
  return std::filesystem::is_empty(path.GetCTStr());
}

void AppendTo(CTStringView to_append, tchar* buff, uindex buff_len,
              uindex* out_len) {
  COMET_ASSERT(buff != nullptr, "Buffer provided is null!");
  COMET_ASSERT(buff_len > 0, "Length of buffer provided is 0!");

  if (to_append.GetLength() == 0) {
    if (out_len != nullptr) {
      *out_len = buff_len;
    }

    return;
  }

  auto buff_offset{GetLength(buff)};

  if (!IsSlash(buff[buff_offset]) && !IsSlash(to_append[0])) {
    buff[buff_offset++] = kNativeSlash;
  } else if (IsSlash(buff[buff_offset]) && IsSlash(to_append[0])) {
    --buff_offset;
  }

  const auto new_len{buff_offset + to_append.GetLength()};
  COMET_ASSERT(buff_len > new_len,
               "Length of buffer provided is too small: ", buff_len,
               " <= ", new_len, "!");
  Copy(buff, to_append, to_append.GetLength(), buff_offset);
  buff[new_len] = COMET_TCHAR('\0');

  if (out_len != nullptr) {
    *out_len = new_len;
  }
}

void Append(CTStringView path_a, CTStringView path_b, tchar* buff,
            uindex buff_len, uindex* out_len) {
  COMET_ASSERT(buff != nullptr, "Buffer provided is null!");
  const auto path_a_len{path_a.GetLength()};
  COMET_ASSERT(buff_len > path_a_len,
               "Length of buffer provided is too small: ", buff_len,
               " <= ", path_a_len, "!");

  Copy(buff, path_a, path_a_len);
  buff[path_a_len] = COMET_TCHAR('\0');
  AppendTo(path_b, buff, buff_len, out_len);
}

TString Append(CTStringView path_a, CTStringView path_b) {
  TString path{};
  // Worst case: path_a and path_b don't contain a trailing/forwarding
  // (respectively) slash, so we need to add 1 character to store it.
  path.Reserve(path_a.GetLength() + path_b.GetLength() + 1);
  path = path_a;
  uindex path_len;
  AppendTo(path_b, path.GetTStr(), path.GetCapacity() + 1, &path_len);
  path.Resize(path_len);
  return path;
}

void RemoveTrailingSlashes(TString& str) {
  if (str.GetLength() == 0) {
    return;
  }

  uindex i{str.GetLength() - 1};

  while (IsSlash(str[i]) && i != kInvalidIndex) {
    --i;
  }

  if (i >= str.GetLength() - 1) {
    return;
  }

  str.Resize(i + 1);
}

f64 GetLastModificationTime(CTStringView path) {
  if (!Exists(path)) {
    return -1;
  }

#ifdef COMET_WINDOWS
  struct _stat64i32 status;

  if (_wstat(path.GetCTStr(), &status) == 0) {
    return status.st_mtime * 1000;
  }
#else
  struct stat status;

  if (stat(path.GetCTStr(), &status) == 0) {
    return status.st_mtime * 1000;
  }
#endif  // COMET_WINDOWS

  return -1;
}

void GetChecksum(CTStringView path, schar* checksum, uindex checksum_len) {
  COMET_ASSERT(checksum_len > kSha256DigestSize,
               "Length of checksum provided should be at least ",
               (kSha256DigestSize + 1), " bytes!");

  if (!IsFile(path)) {
    checksum[0] = '\0';
    return;
  }

  auto file{std::ifstream(path, std::ios::binary)};
  HashSha256(file, checksum, checksum_len);
  checksum[kSha256DigestSize] = COMET_TCHAR('\0');
}

void Normalize(tchar* str, uindex len) {
#ifdef COMET_WINDOWS
  for (uindex i{0}; i < len; ++i) {
    tchar c{str[i]};

    if (str[i] == COMET_TCHAR('\\')) {
      str[i] = COMET_TCHAR('/');
    }
  }
#else
  return;
#endif  // COMET_WINDOWS
}

void Normalize(tchar* str) { return Normalize(str, GetLength(str)); }

void Normalize(TString& str) {
  return Normalize(str.GetTStr(), str.GetLength());
}

void MakeNative(tchar* str, uindex len) {
#ifdef COMET_WINDOWS
  for (uindex i{0}; i < len; ++i) {
    tchar c{str[i]};

    if (str[i] == COMET_TCHAR('/')) {
      str[i] = COMET_TCHAR('\\');
    }
  }
#else
  return;
#endif  // COMET_WINDOWS
}

void MakeNative(tchar* str) { return MakeNative(str, GetLength(str)); }

void MakeNative(TString& str) {
  return MakeNative(str.GetTStr(), str.GetLength());
}

void Clean(tchar* str, uindex len) {
#ifdef COMET_NORMALIZE_PATHS
  Normalize(str, len);
#else
  MakeNative(str, len);
#endif  // COMET_NORMALIZE_PATHS
}

void Clean(tchar* str) { return Clean(str, GetLength(str)); }
void Clean(TString& str) { return Clean(str.GetTStr(), str.GetLength()); }

const tchar* GetTmpTChar(const schar* str, uindex len) {
#ifdef COMET_WIDE_TCHAR
  auto* tmp{GenerateForOneFrame<tchar>(len + 1)};
  Copy(tmp, str, len);
  tmp[len] = COMET_TCHAR('\0');
  return tmp;
#else
  return str;
#endif  // COMET_WIDE_TCHAR
}

const tchar* GetTmpTChar(const schar* str) {
  return GetTmpTChar(str, GetLength(str));
}

const tchar* GetTmpTChar(const wchar* str, uindex len) {
#ifdef COMET_WIDE_TCHAR
  return str;
#else
  auto* tmp{GenerateForOneFrame<tchar>(len + 1)};
  Copy(tmp, str, len);
  tmp[len] = COMET_TCHAR('\0');
  return tmp;
#endif  // COMET_WIDE_TCHAR
}

const tchar* GetTmpTChar(const wchar* str) {
  return GetTmpTChar(str, GetLength(str));
}

uindex GetSize(CTStringView path) {
#ifdef COMET_WINDOWS
  WIN32_FILE_ATTRIBUTE_DATA status;

  if (GetFileAttributesExW(path.GetCTStr(), GetFileExInfoStandard, &status)) {
    ULARGE_INTEGER size;
    size.LowPart = status.nFileSizeLow;
    size.HighPart = status.nFileSizeHigh;
    return static_cast<uindex>(size.QuadPart);
  }

  return 0;
#else
  struct stat status;

  if (stat(path.GetCTStr(), &status) == 0) {
    return static_cast<uindex>(status.st_size);
  }

  return 0;
#endif  // COMET_WINDOWS
}
}  // namespace comet
