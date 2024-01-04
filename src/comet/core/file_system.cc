// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "file_system.h"

#include "comet/core/generator.h"
#include "comet/core/hash.h"

namespace comet {
namespace internal {
const tchar* GetTmpCopyWithNormalizedSlashes(CTStringView str) {
#ifdef COMET_WINDOWS
  auto* tmp{GenerateForOneFrame<tchar>(str.GetLength())};

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

const tchar* GetNextElementForRelativePath(const tchar*& cursor,
                                           const tchar* path_end) {
  auto is_dot{false};
  auto is_dot_dot{false};
  auto is_from_non_dot_dot{false};

  while (cursor < path_end) {
    tchar from_c{*cursor};

    if (IsSlash(from_c)) {
      ++cursor;

      if (is_dot_dot) {
        is_dot = false;
        is_dot_dot = false;
        // Do nothing, the dot-dot folder points to an already known name (the
        // previous one).
        continue;
      }

      if (is_dot) {
        is_dot = false;
        continue;
      }
    }

    if (is_dot_dot) {
      cursor -= kDotDotFolderName.GetLength();
    } else if (is_dot) {
      cursor -= kDotFolderName.GetLength();
    }

    return cursor;
  }

  return nullptr;
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
    COMET_LOG_CORE_ERROR("Could not open file at path: ", path,
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
    COMET_LOG_CORE_ERROR("Could not open file at path: ", path,
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
  if (path.IsEmpty()) {
    return false;
  }

  const auto last_character{path.GetLast()};

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

  return WriteStrToFile(path, "", true);
}

bool CreateDirectory(CTStringView path, bool is_recursive) {
#ifdef COMET_MSVC
  auto is_created{MSVC_CREATE_DIRECTORY(path, nullptr) ||
                  GetLastError() == ERROR_ALREADY_EXISTS};

  if (!is_recursive || is_created) {
    return is_created;
  }

  return CreateDirectory(GetParentPath(path), true) &&
             MSVC_CREATE_DIRECTORY(path, nullptr) ||
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
#endif  // COMET_MSVC
}

bool Move(CTStringView previous_name, CTStringView new_name) {
  if (!Exists(previous_name) ||
      (Exists(new_name)) && previous_name != new_name) {
    return false;
  }

#ifdef COMET_MSVC
  return MSVC_MOVE_FILE(previous_name, new_name) != 0;
#else
  return rename(previous_name, new_name) == 0;
#endif  // COMET_MSVC
}

bool Remove(CTStringView path, bool is_recursive) {
  if (IsFile(path)) {
#ifdef COMET_MSVC
    return MSVC_DELETE_FILE(path) != 0;
#else
    return unlink(path) == 0;
#endif  // COMET_MSVC
  }

  if (!is_recursive) {
#ifdef COMET_MSVC
    return MSVC_REMOVE_DIRECTORY(path);
#else
    return rmdir(path) == 0;
#endif  // COMET_MSVC
  }

#ifdef COMET_MSVC
  MSVC_WIN32_FIND_DATA find_data;
  // Add 1 character for *, and 1 for an extra-slash (which could be needed).
  const auto full_path_capacity{path.GetLength() + 2};
  auto* full_path{GenerateForOneFrame<tchar>(full_path_capacity)};
  uindex full_path_len{0};
  Append(path, COMET_TCHAR("*"), full_path, full_path_capacity, &full_path_len);
  auto file_handle{MSVC_FIND_FIRST_FILE(full_path, &find_data)};

  if (file_handle == INVALID_HANDLE_VALUE) {
    return false;
  }

  full_path[--full_path_len] = COMET_TCHAR('\0');

  do {
    const auto path_len{GetLength(find_data.cFileName)};

    if (AreStringsEqual(find_data.cFileName, path_len,
                        kDotFolderName.GetCTStr(),
                        kDotFolderName.GetLength()) ||
        AreStringsEqual(find_data.cFileName, path_len,
                        kDotDotFolderName.GetCTStr(),
                        kDotDotFolderName.GetLength())) {
      continue;
    }

    auto sub_path_len{full_path_len + path_len};
    auto* sub_path{GenerateForOneFrame<tchar>(sub_path_len)};
    Append(full_path, find_data.cFileName, sub_path, sub_path_len);

    if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      if (!Remove(sub_path, true)) {
        FindClose(file_handle);
        return false;
      }

      continue;
    }

    if (!MSVC_DELETE_FILE(sub_path)) {
      return false;
    }
  } while (MSVC_FIND_NEXT_FILE(file_handle, &find_data) != 0);

  FindClose(file_handle);
  return MSVC_REMOVE_DIRECTORY(path);
#else
  auto* dir{opendir(path)};

  if (dir == nullptr) {
    return false;
  }

  struct dirent* entry{nullptr};
  tchar buff[kMaxPathLength]{COMET_TCHAR('\0')};
  const auto path_len{path.GetLength()};

  while ((entry = readdir(dir)) != nullptr) {
    const auto dir_len{GetLength(entry->d_name)};

    if (entry->d_type == DT_DIR &&
        (AreStringsEqual(entry->d_name, dir_len, kDotFolderName.GetCTStr(),
                         kDotFolderName.GetLength()) ||
         AreStringsEqual(entry->d_name, dir_len, kDotDotFolderName.GetCTStr(),
                         2))) {
      continue;
    }

    CTStringView folder_name{entry->d_name, entry->d_reclen};
    Append(path, folder_name, buff, kMaxPathLength);
    buff[path_len + entry->d_reclen + 1] = COMET_TCHAR('\0');

    if (entry->d_type == DT_DIR) {
      if (!Remove(buff, true)) {
        closedir(dir);
        return false;
      }

      continue;
    }

    if (unlink(buff) != 0) {
      perror("unlink");
      closedir(dir);
      return false;
    }
  }

  closedir(dir);
  return rmdir(path) == 0;
#endif  // COMET_MSVC
}

TString GetCurrentDirectory(bool is_clean) {
  tchar path_str[kMaxPathLength]{COMET_TCHAR('\0')};
  bool is_ok{false};

#ifdef COMET_MSVC
  is_ok = MSVC_GET_CURRENT_DIRECTORY(kMaxPathLength, path_str) != 0;
#else
  is_ok = getcwd(path_str, kMaxPathLength) != nullptr;
#endif  // COMET_MSVC
  COMET_ASSERT(is_ok, "Unable to get current directory!");
  TString path{reinterpret_cast<const tchar*>(path_str)};
  Clean(path);
  return path;
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
  if (path.IsEmpty()) {
    return TString{};
  }

  uindex offset;
  // Normalize path to work with slashes (/) only.
  const auto* normalized_path{internal::GetTmpCopyWithNormalizedSlashes(path)};

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
  // Case: empty path. Returning empty path as well.
  if (path.IsEmpty()) {
    return TString{};
  }

  uindex normalized_cursor{0};
  auto* normalized{GenerateForOneFrame<tchar>(path.GetLength())};
  const auto is_absolute{IsAbsolute(path)};
  auto is_previous_slash{false};
  auto dot_index{kInvalidIndex};
  auto dot_dot_index{kInvalidIndex};

  uindex pre_i{0};
  uindex start_anchor{0};

  // Case: path is absolute. Copy root and ignore every dot-dot folders that
  // follow immediately after.
  if (is_absolute) {
#ifdef COMET_WINDOWS
    // Copy letter and colon.
    normalized[normalized_cursor++] = path[pre_i++];
    normalized[normalized_cursor++] = path[pre_i++];
#endif  // COMET_WINDOWS
    // Ignore all leading slashes.
    while (IsSlash(path[pre_i]) && pre_i < path.GetLength()) {
      ++pre_i;
    }

    // Add only one leading slash.
    normalized[normalized_cursor++] = kNativeSlash;

    // Ignore all dot and dot-dot folder names and their related slashes.
    auto is_dot{false};
    auto is_dot_dot{false};
    auto is_non_dot_dot{false};
    start_anchor = pre_i - 1;

    while (!is_non_dot_dot && pre_i < path.GetLength()) {
      auto c{path[pre_i++]};

      if (IsSlash(c)) {
        start_anchor = pre_i - 1;
        is_dot = false;
        is_dot_dot = false;
        continue;
      }

      if (c == kDotFolderName) {
        if (!is_dot) {
          is_dot = true;
          continue;
        }

        if (!is_dot_dot) {
          is_dot_dot = true;
          continue;
        }
      }

      is_non_dot_dot = true;

      // Add one because it contains the last slash index for now.
      ++start_anchor;
    }
    // Case: path is relative.
    // Copy all the dot-dot folder names and their related slashes. Ignore all
    // the dot ones.
  } else {
    auto is_dot{false};
    auto is_dot_dot{false};
    auto is_non_dot_dot{false};

    while (!is_non_dot_dot && pre_i < path.GetLength()) {
      auto c{path[pre_i++]};

      if (IsSlash(c)) {
        // Case: we had a dot-dot folder previously.
        if (is_dot_dot) {
          Copy(normalized, kDotDotFolderName.GetCTStr(),
               kDotDotFolderName.GetLength(), normalized_cursor);
          normalized_cursor += kDotDotFolderName.GetLength();

          const auto dot_dot_index = kDotDotFolderName.GetLength() - 1;

          if (normalized_cursor > dot_dot_index &&
              !IsSlash(normalized[normalized_cursor - dot_dot_index])) {
            normalized[normalized_cursor++] = kNativeSlash;
          }
        }

        start_anchor = pre_i - 1;
        is_dot = false;
        is_dot_dot = false;
        continue;
      }

      if (c == kDotFolderName) {
        if (!is_dot) {
          is_dot = true;
          continue;
        }

        if (!is_dot_dot) {
          is_dot_dot = true;
          continue;
        }
      }

      is_non_dot_dot = true;
    }

    if (is_dot_dot) {
      Copy(normalized, kDotDotFolderName.GetCTStr(),
           kDotDotFolderName.GetLength(), normalized_cursor);
      normalized_cursor += kDotDotFolderName.GetLength();
      start_anchor = path.GetLength();
    }
  }

  is_previous_slash =
      normalized_cursor != 0 && IsSlash(normalized[normalized_cursor - 1]);

  for (uindex i{start_anchor}; i < path.GetLength(); ++i) {
    const auto c{path[i]};

    if (IsSlash(c)) {
      if (is_previous_slash) {
        // Ignore duplicated slashes.
        continue;
      }

      if (dot_dot_index != kInvalidIndex) {
        // Case: non-dot-dot filename + directory separator + dot-dot folder
        // name.
        //
        // Remove last filename added as the dot-dot folder name cancels it out.
        normalized_cursor -=
            kDotDotFolderName
                .GetLength();  // Set cursor to last slash and remove it.

        while (!IsSlash(normalized[normalized_cursor]) &&
               normalized_cursor != 0) {
          --normalized_cursor;
        }

        dot_index = kInvalidIndex;
        dot_dot_index = kInvalidIndex;
        continue;
      } else if (dot_index != kInvalidIndex) {
        // Case: dot folder name. Ignoring it.
        dot_index = kInvalidIndex;
        continue;
      }

      // Case: slash. Normalizing it.
      normalized[normalized_cursor++] = kNativeSlash;
      is_previous_slash = true;
      continue;
    }

    is_previous_slash = false;

    if (c == kDotFolderName) {
      if (dot_index != kInvalidIndex) {
        dot_dot_index = normalized_cursor;
        continue;
      }

      dot_index = normalized_cursor;
      continue;
    }

    while (dot_index != kInvalidIndex && dot_index != i) {
      normalized[normalized_cursor++] = path[dot_index++];
    }

    normalized[normalized_cursor++] = path[i];
    dot_index = kInvalidIndex;
    dot_dot_index = kInvalidIndex;
  }

  if (dot_dot_index != kInvalidIndex) {
    normalized_cursor -=
        kDotDotFolderName
            .GetLength();  // Set cursor to last slash and remove it.

    while (!IsSlash(normalized[normalized_cursor]) && normalized_cursor != 0) {
      --normalized_cursor;
    }

    if (normalized_cursor > 0) {
      ++normalized_cursor;
    }
  } else if (dot_index != kInvalidIndex && normalized_cursor != 0 &&
             !IsSlash(normalized[normalized_cursor - 1])) {
    ++normalized_cursor;
  }

  if (normalized_cursor == 0) {
    normalized[normalized_cursor++] = COMET_TCHAR('.');
  }

  normalized[normalized_cursor] = COMET_TCHAR('\0');
  return TString{normalized, GetLength(normalized)};
}

TString GetAbsolutePath(CTStringView relative_path) {
  tchar path_str[kMaxPathLength]{COMET_TCHAR('\0')};
  bool is_ok{false};

#ifdef COMET_MSVC
  is_ok = MSVC_GET_FULL_PATH_NAME(relative_path.GetCTStr(), kMaxPathLength,
                                  path_str, nullptr) != 0;
#else
  is_ok = realpath(relative_path.GetCTStr(), path_str) != nullptr;

  if (!is_ok) {
    auto last_slash{GetLastIndexOf(relative_path.GetCTStr(),
                                   relative_path.GetLength(), kNativeSlash)};
    auto len{last_slash + 1};

    if (last_slash != kInvalidIndex) {
      auto* tmp{GenerateForOneFrame<tchar>(len)};
      Copy(tmp, relative_path, len);
      tmp[len] = COMET_TCHAR('\0');

      while (!is_ok && last_slash != kInvalidIndex) {
        is_ok = realpath(tmp, path_str) != nullptr;

        if (!is_ok) {
          last_slash = GetLastIndexOf(tmp, len, kNativeSlash);

          if (last_slash != kInvalidIndex) {
            len = last_slash + 1;
          }

          tmp[len] = COMET_TCHAR('\0');
        }
      }
    }

    if (is_ok) {
      Append("", relative_path.GetCTStr() + len, path_str + len - 1,
             kMaxPathLength - len);
    }
  }

#endif  // COMET_MSVC

  COMET_ASSERT(is_ok, "Unable to get absolute path!");
  auto path{TString{reinterpret_cast<const tchar*>(path_str)}};
  Clean(path);
  return path;
}

TString GetRelativePath(CTStringView to, CTStringView from) {
  const auto* from_p{from.GetCTStr()};
  const auto* to_p{to.GetCTStr()};
  const tchar* from_cursor;
  const tchar* to_cursor;

#ifdef COMET_WINDOWS
  const auto from_root_type{GetRootType(from)};
  const auto to_root_type{GetRootType(to)};

  if (from_root_type != to_root_type || from_root_type == RootType::Unknown) {
    return TString{};
  }

  switch (from_root_type) {
    case RootType::Unix:
      from_cursor = from_p + 1;
      to_cursor = to_p + 1;
      break;
    case RootType::WindowsDriveLetter:
      // Case: drive letters won't match. No relative path possible.
      if (from[0] != to[0]) {
        return TString{};
      }

      from_cursor = from_p + 3;
      to_cursor = to_p + 3;
      break;
    case RootType::WindowsExtended:
      from_cursor = from_p + 4;
      to_cursor = to_p + 4;
      break;
    case RootType::WindowsUnc:
      from_cursor = from_p + 8;
      to_cursor = to_p + 8;
      break;
    // Path is relative. We don't have anything to do.
    case RootType::Invalid:
      from_cursor = from_p;
      to_cursor = to_p;
      break;
    default:
      COMET_ASSERT(false, "Unknown or unsupported root type: ",
                   GetRootTypeLabel(from_root_type), "!");
  }
#else
  const auto is_from_absolute{IsAbsolute(from)};
  const auto is_to_absolute{IsAbsolute(to)};

  if (is_from_absolute != is_to_absolute) {
    return TString{};
  }

  if (is_from_absolute) {
    from_cursor = from_p + 1;
    to_cursor = to_p + 1;
  } else {
    from_cursor = from_p;
    to_cursor = to_p;
  }
#endif  // COMET_WINDOWS

  const auto* from_end{from_p + from.GetLength()};
  const auto* to_end{to_p + to.GetLength()};

  // 1. 1st non-matching element with from & to (process every . and .. along
  // the way).
  const tchar* from_folder_cursor{nullptr};
  const tchar* to_folder_cursor{nullptr};

  while (from_cursor < from_end && to_cursor < to_end) {
    from_folder_cursor =
        internal::GetNextElementForRelativePath(from_cursor, from_end);
    to_folder_cursor =
        internal::GetNextElementForRelativePath(to_cursor, to_end);
    auto are_equal{true};
    auto is_from_slash{IsSlash(*from_folder_cursor)};
    auto is_to_slash{IsSlash(*to_folder_cursor)};

    while (!is_from_slash && !is_to_slash && from_folder_cursor < from_end &&
           to_folder_cursor < to_end) {
      if (*from_folder_cursor != *to_folder_cursor) {
        are_equal = false;
        break;
      }

      ++from_folder_cursor;
      ++to_folder_cursor;
      is_from_slash = IsSlash(*from_folder_cursor);
      is_to_slash = IsSlash(*to_folder_cursor);
    }

    from_cursor = is_from_slash ? from_folder_cursor + 1 : from_folder_cursor;
    to_cursor = is_to_slash ? to_folder_cursor + 1 : to_folder_cursor;

    if (!are_equal) {
      break;
    }
  }

  auto relative_path_len{from_end - from_cursor + to_end - to_cursor};

  if (relative_path_len == 0) {
    return TString{kDotFolderName.GetCTStr(), kDotFolderName.GetLength()};
  }

  // 2. Get tmp string with specific size to contain both remaining paths.
  auto* relative_path{GenerateForOneFrame<tchar>(relative_path_len)};
  uindex relative_path_cursor{0};
  auto is_previous_slash{IsSlash(*from_cursor)};

  // 3. Add dot-dot folders  for every folder remaining in from. If a dot-dot
  // folder is found, cancel the last one added.
  auto is_dot{false};
  auto is_dot_dot{false};
  auto is_non_dot_dot{false};

  while (from_cursor < from_end) {
    if (IsSlash(*from_cursor++)) {
      is_non_dot_dot = false;

      if (is_dot_dot) {
        is_dot = false;
        is_dot_dot = false;

        // Case: nothing has been copied yet.
        if (relative_path_cursor == 0) {
          Copy(relative_path, kDotDotFolderName.GetCTStr(),
               kDotDotFolderName.GetLength(), relative_path_cursor);
          relative_path_cursor += kDotDotFolderName.GetLength();
          relative_path[relative_path_cursor++] = kNativeSlash;
        }

        relative_path_cursor -= kDotDotFolderName.GetLength() + 1;
        is_previous_slash = false;
        continue;
      }

      if (is_dot) {
        is_dot = false;
        is_previous_slash = false;
        continue;
      }

      if (!is_previous_slash) {
        if (relative_path_cursor > 0) {
          relative_path[relative_path_cursor++] = kNativeSlash;
        }

        Copy(relative_path, kDotDotFolderName.GetCTStr(),
             kDotDotFolderName.GetLength(), relative_path_cursor);
        relative_path_cursor += kDotDotFolderName.GetLength();
        is_previous_slash = true;
      }

      continue;
    }

    is_previous_slash = false;
    is_non_dot_dot = true;
  }

  if (is_non_dot_dot) {
    if (relative_path_cursor > 0) {
      relative_path[relative_path_cursor++] = kNativeSlash;
    }

    Copy(relative_path, kDotDotFolderName.GetCTStr(),
         kDotDotFolderName.GetLength(), relative_path_cursor);
    relative_path_cursor += kDotDotFolderName.GetLength();
  }

  const auto to_copy_len{static_cast<uindex>(to_end - to_cursor)};

  if (relative_path_cursor == 0 && to_copy_len == 0) {
    return TString{kDotFolderName.GetCTStr(), kDotFolderName.GetLength()};
  }

  if (relative_path_cursor > 0 &&
      !IsSlash(relative_path[relative_path_cursor - 1]) && to_copy_len > 0 &&
      !IsSlash(*to_cursor)) {
    relative_path[relative_path_cursor++] = kNativeSlash;
  }

  // 4. Add the rest for every folder in to.
  Copy(relative_path, to_cursor, to_copy_len, relative_path_cursor);

  relative_path_cursor += to_copy_len;
  relative_path[relative_path_cursor++] = COMET_TCHAR('\0');

  // 5. Return normalized version.
  return GetNormalizedPath(relative_path);
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
#ifdef COMET_MSVC
  return c == COMET_TCHAR('/') || c == COMET_TCHAR('\\');
#else
  return c == COMET_TCHAR('/');
#endif  // COMET_MSVC
  return false;
}

bool IsDirectory(CTStringView path) {
  if (path.IsEmpty()) {
    return false;
  }

#ifdef COMET_MSVC
  auto attributes{MSVC_GET_FILE_ATTRIBUTES(path)};
  return attributes != 0xFFFFFFFF && attributes & FILE_ATTRIBUTE_DIRECTORY;
#else
  struct stat status;
  return stat(path, &status) != -1 && S_ISDIR(status.st_mode);
#endif  // COMET_MSVC
}

bool IsFile(CTStringView path) {
  if (path.IsEmpty()) {
    return false;
  }

#ifdef COMET_MSVC
  auto attributes{MSVC_GET_FILE_ATTRIBUTES(path)};
  return attributes != 0xFFFFFFFF &&
         (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
#else
  struct stat status;
  return stat(path, &status) != -1 && S_ISREG(status.st_mode);
#endif  // COMET_MSVC
}

const schar* GetRootTypeLabel(RootType root_type) {
  switch (root_type) {
    case RootType::Unknown:
      return "unknown";
    case RootType::Unix:
      return "Unix";
    case RootType::WindowsDriveLetter:
      return "Windows drive letter";
    case RootType::WindowsExtended:
      return "Windows extended";
    case RootType::WindowsUnc:
      return "Windows UNC";
    case RootType::Invalid:
      return "invalid";
  }

  return "???";
}

RootType GetRootType(CTStringView path) {
  if (path.IsEmpty()) {
    return RootType::Invalid;
  }

#ifdef COMET_MSVC
  // Check for root paths.
  if (IsSlash(path[0])) {
    return RootType::Unix;
  }

  // Check for drive letters.
  if (path.GetLength() >= 3 && IsAlpha(path[0]) && path[1] == ':' &&
      IsSlash(path[2])) {
    return RootType::WindowsDriveLetter;
  }

  // Check for extended paths.
  if (path.GetLength() >= 4) {
    tchar tmp[5];
    Copy(tmp, path, 4);
    tmp[4] = COMET_TCHAR('\0');

    if (AreStringsEqual(tmp, 4, COMET_TCHAR("\\\\?\\"), 4)) {
      return RootType::WindowsExtended;
    }
  }

  // Check for UNC paths.
  if (path.GetLength() >= 8) {
    tchar tmp[9];
    Copy(tmp, path, 8);
    tmp[8] = COMET_TCHAR('\0');

    if (AreStringsEqual(tmp, 8, COMET_TCHAR("\\\\?\\UNC\\"), 8)) {
      return RootType::WindowsUnc;
    }
  }

  return RootType::Invalid;
#else
  return IsSlash(path[0]) ? RootType::Unix : RootType::Invalid;
#endif  // COMET_MSVC
}

bool IsAbsolute(CTStringView path) {
  const auto root_type{GetRootType(path)};
  return root_type != RootType::Unknown && root_type != RootType::Invalid;
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
  COMET_ASSERT(Exists(path), path, " does not exist!");

  if (IsFile(path)) {
    return GetSize(path) == 0;
  }

#ifdef COMET_MSVC
  MSVC_WIN32_FIND_DATA find_data;
  // Add 1 character for *, and 1 for an extra-slash (which could be needed).
  const auto full_path_capacity{path.GetLength() + 2};
  auto* full_path{GenerateForOneFrame<tchar>(full_path_capacity)};
  uindex full_path_len{0};
  Append(path, COMET_TCHAR("*"), full_path, full_path_capacity, &full_path_len);
  auto file_handle{MSVC_FIND_FIRST_FILE(full_path, &find_data)};

  if (file_handle == INVALID_HANDLE_VALUE) {
    return true;
  }

  do {
    const auto path_len{GetLength(find_data.cFileName)};

    if (AreStringsEqual(find_data.cFileName, path_len,
                        kDotFolderName.GetCTStr(),
                        kDotFolderName.GetLength()) ||
        AreStringsEqual(find_data.cFileName, path_len,
                        kDotDotFolderName.GetCTStr(),
                        kDotDotFolderName.GetLength())) {
      continue;
    }

    FindClose(file_handle);
    return false;
  } while (MSVC_FIND_NEXT_FILE(file_handle, &find_data) != 0);

  FindClose(file_handle);
  return true;
#else
  struct stat status;

  if (stat(path.GetCTStr(), &status) != 0) {
    return false;
  }

  COMET_ASSERT(S_ISDIR(status.st_mode), path,
               " is not a directory! What happened?");
  auto* dir{opendir(path)};
  COMET_ASSERT(dir != nullptr, path, " is not a directory! What happened?");
  struct dirent* entry{nullptr};

  while ((entry = readdir(dir))) {
    const auto dir_len{GetLength(entry->d_name)};

    if (entry->d_type == DT_DIR &&
        (AreStringsEqual(entry->d_name, dir_len, kDotFolderName.GetCTStr(),
                         kDotFolderName.GetLength()) ||
         AreStringsEqual(entry->d_name, dir_len, kDotDotFolderName.GetCTStr(),
                         2))) {
      continue;
    }

    if (entry->d_type == DT_REG || entry->d_type == DT_DIR) {
      closedir(dir);
      return false;
    }
  }

  closedir(dir);
  return true;
#endif  // COMET_MSVC
}

void AppendTo(CTStringView to_append, tchar* buff, uindex buff_len,
              uindex* out_len) {
  COMET_ASSERT(buff != nullptr, "Buffer provided is null!");
  COMET_ASSERT(buff_len > 0, "Length of buffer provided is 0!");

  if (to_append.IsEmpty()) {
    if (out_len != nullptr) {
      *out_len = buff_len;
    }

    return;
  }

  auto buff_offset{GetLength(buff)};

  if (!IsSlash(buff[buff_offset - 1]) && !IsSlash(to_append[0])) {
    buff[buff_offset++] = kNativeSlash;
  } else if (IsSlash(buff[buff_offset]) && IsSlash(to_append[0])) {
    --buff_offset;
  }

  const auto new_len{buff_offset + to_append.GetLength()};
  COMET_ASSERT(buff_len >= new_len,
               "Length of buffer provided is too small: ", buff_len, " < ",
               new_len, "!");
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
  COMET_ASSERT(buff_len >= path_a_len,
               "Length of buffer provided is too small: ", buff_len, " < ",
               path_a_len, "!");

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
  if (str.IsEmpty()) {
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

#ifdef COMET_MSVC
  struct _stat64i32 status;

  if (_wstat(path.GetCTStr(), &status) == 0) {
    return status.st_mtime * 1000;
  }
#else
  struct stat status;

  if (stat(path.GetCTStr(), &status) == 0) {
    return status.st_mtime * 1000;
  }
#endif  // COMET_MSVC

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

void NormalizeSlashes(tchar* str, uindex len) {
#ifdef COMET_MSVC
  for (uindex i{0}; i < len; ++i) {
    tchar c{str[i]};

    if (str[i] == COMET_TCHAR('\\')) {
      str[i] = COMET_TCHAR('/');
    }
  }
#else
  return;
#endif  // COMET_MSVC
}

void NormalizeSlashes(tchar* str) {
  return NormalizeSlashes(str, GetLength(str));
}

void NormalizeSlashes(TString& str) {
  return NormalizeSlashes(str.GetTStr(), str.GetLength());
}

void MakeNative(tchar* str, uindex len) {
#ifdef COMET_MSVC
  for (uindex i{0}; i < len; ++i) {
    tchar c{str[i]};

    if (str[i] == COMET_TCHAR('/')) {
      str[i] = COMET_TCHAR('\\');
    }
  }
#else
  return;
#endif  // COMET_MSVC
}

void MakeNative(tchar* str) { return MakeNative(str, GetLength(str)); }

void MakeNative(TString& str) {
  return MakeNative(str.GetTStr(), str.GetLength());
}

void Clean(tchar* str, uindex len) {
#ifdef COMET_NORMALIZE_PATHS
  NormalizeSlashes(str, len);
#else
  MakeNative(str, len);
#endif  // COMET_NORMALIZE_PATHS
}

void Clean(tchar* str) { return Clean(str, GetLength(str)); }
void Clean(TString& str) { return Clean(str.GetTStr(), str.GetLength()); }

const tchar* GetTmpTChar(const schar* str, uindex len) {
#ifdef COMET_WIDE_TCHAR
  auto* tmp{GenerateForOneFrame<tchar>(len)};
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
  auto* tmp{GenerateForOneFrame<tchar>(len)};
  Copy(tmp, str, len);
  tmp[len] = COMET_TCHAR('\0');
  return tmp;
#endif  // COMET_WIDE_TCHAR
}

const tchar* GetTmpTChar(const wchar* str) {
  return GetTmpTChar(str, GetLength(str));
}

uindex GetSize(CTStringView path) {
#ifdef COMET_MSVC
  WIN32_FILE_ATTRIBUTE_DATA status{};

  if (MSVC_GET_FILE_ATTRIBUTES_EX(path.GetCTStr(), GetFileExInfoStandard,
                                  &status)) {
    ULARGE_INTEGER size{};
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
#endif  // COMET_MSVC
}
}  // namespace comet
