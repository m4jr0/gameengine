// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_precompile.h"

#include "comet/core/file_system.h"

#include "catch.hpp"
#include "catch2/reporters/catch_reporter_event_listener.hpp"
#include "catch2/reporters/catch_reporter_registrars.hpp"

#include "comet/core/c_string.h"
#include "comet/core/type/tstring.h"

#ifndef COMET_NORMALIZE_PATHS
#define COMET_NORMALIZE_PATHS
#endif  // !COMET_NORMALIZE_PATHS

namespace comet {
namespace comettests {
const auto* test_dir{COMET_TCHAR("comettests_tests_file_system")};
comet::TString current_dir{};
comet::TString tmp_dir{};

class TestsFileSystemEventListener : public Catch::EventListenerBase {
  using Catch::EventListenerBase::EventListenerBase;

 public:
  void testCaseStarting(Catch::TestCaseInfo const&) override {
    current_dir = comet::GetCurrentDirectory();
    tmp_dir = comet::comettests::current_dir / comet::comettests::test_dir;
    comet::Remove(comet::comettests::tmp_dir, true);
  }
};

CATCH_REGISTER_LISTENER(TestsFileSystemEventListener)

TString FormatAbsolutePath(CTStringView absolute_path, tchar to_search,
                           const uindex count) {
  if (absolute_path.IsEmpty() || count == 0) {
    return TString{absolute_path.GetCTStr()};
  }

  uindex index_to_cut{GetNthToLastIndexOf(
      absolute_path.GetCTStr(), absolute_path.GetLength(), to_search, count)};

  if (index_to_cut == kInvalidIndex) {
    return TString{""};
  }

  return TString{
      absolute_path.GenerateSubString(index_to_cut, absolute_path.GetLength())};
}
}  // namespace comettests
}  // namespace comet

TEST_CASE("File system management", "[comet::filesystem]") {
  SECTION("Create operations.") {
    REQUIRE(!comet::CreateFile(
        comet::Append(comet::comettests::tmp_dir, COMET_TCHAR("/test"))));

    REQUIRE(comet::CreateFile(
        comet::Append(comet::comettests::tmp_dir, COMET_TCHAR("/test")), true));

    comet::schar checksum[33];
    comet::GetChecksum(
        comet::Append(comet::comettests::tmp_dir, COMET_TCHAR("/test")),
        checksum, 33);

    REQUIRE(!comet::CreateFile(
        comet::Append(comet::comettests::tmp_dir, COMET_TCHAR("/test2/"))));

    REQUIRE(comet::CreateDirectory(
        comet::Append(comet::comettests::tmp_dir, COMET_TCHAR("/test3"))));

    REQUIRE(comet::CreateDirectory(
        comet::Append(comet::comettests::tmp_dir, COMET_TCHAR("/test4/"))));

    REQUIRE(!comet::CreateDirectory(comet::Append(
        comet::comettests::tmp_dir, COMET_TCHAR("/test5/test6"))));

    REQUIRE(comet::CreateDirectory(
        comet::Append(comet::comettests::tmp_dir, COMET_TCHAR("/test5/test6")),
        true));
  }

  SECTION("Read & write operations.") {
    constexpr auto* test_write{"test_write"};
    constexpr auto test_write_len{comet::GetLength(test_write)};

    // The file does not exist (yet).
    REQUIRE(comet::WriteStrToFile(
        comet::Append(comet::comettests::tmp_dir, COMET_TCHAR("/test7")),
        test_write));

    // The file already exists.
    REQUIRE(comet::WriteStrToFile(
        comet::Append(comet::comettests::tmp_dir, COMET_TCHAR("/test")),
        test_write));

    comet::schar test_read[4096];
    comet::uindex test_read_len;

    REQUIRE(comet::ReadStrFromFile(
        comet::Append(comet::comettests::tmp_dir, COMET_TCHAR("/test7")),
        test_read, 4096, &test_read_len));

    REQUIRE(comet::AreStringsEqual(test_read, test_read_len, test_write,
                                   test_write_len));
  }

  SECTION("Move operations.") {
    // Checking with something that does not exist.
    REQUIRE(!comet::Move(
        comet::Append(comet::comettests::tmp_dir, COMET_TCHAR("/DOESNOTEXIST")),
        comet::Append(comet::comettests::tmp_dir, COMET_TCHAR("/test8"))));

    // Checking with a file.
    REQUIRE(comet::Move(
        comet::Append(comet::comettests::tmp_dir, COMET_TCHAR("/test7")),
        comet::Append(comet::comettests::tmp_dir, COMET_TCHAR("/test8"))));

    // Checking with a file on itself.
    REQUIRE(comet::Move(
        comet::Append(comet::comettests::tmp_dir, COMET_TCHAR("/test8")),
        comet::Append(comet::comettests::tmp_dir, COMET_TCHAR("/test8"))));

    // Checking with a folder.
    REQUIRE(comet::Move(
        comet::Append(comet::comettests::tmp_dir, COMET_TCHAR("/test4")),
        comet::Append(comet::comettests::tmp_dir, COMET_TCHAR("/test9"))));

    // Checking with a folder on itself.
    REQUIRE(comet::Move(
        comet::Append(comet::comettests::tmp_dir, COMET_TCHAR("/test9")),
        comet::Append(comet::comettests::tmp_dir, COMET_TCHAR("/test9"))));

    // Checking with a folder on a file.
    REQUIRE(!comet::Move(
        comet::Append(comet::comettests::tmp_dir, COMET_TCHAR("/test9")),
        comet::Append(comet::comettests::tmp_dir, COMET_TCHAR("/test8"))));

    // Checking with a file on a folder.
    REQUIRE(!comet::Move(
        comet::Append(comet::comettests::tmp_dir, COMET_TCHAR("/test8")),
        comet::Append(comet::comettests::tmp_dir, COMET_TCHAR("/test9"))));
  }

  SECTION("Files and folders operations.") {
    std::array<const comet::TString, 3> dir_to_test{
        {comet::comettests::tmp_dir / COMET_TCHAR("/test3"),
         comet::comettests::tmp_dir / COMET_TCHAR("/test5"),
         comet::comettests::tmp_dir / COMET_TCHAR("/test9")}};
    comet::uindex count{0};

    comet::ForEachDirectory(comet::comettests::tmp_dir,
                            [&](comet::CTStringView directory_path) {
                              for (const auto& path : dir_to_test) {
                                if (path == directory_path) {
                                  ++count;
                                  break;
                                }
                              }
                            });

    REQUIRE(count == dir_to_test.size());

    std::array<const comet::TString, 2> files_to_test{
        {comet::comettests::tmp_dir / COMET_TCHAR("/test"),
         comet::comettests::tmp_dir / COMET_TCHAR("/test8")}};
    count = 0;

    comet::ForEachFile(comet::comettests::tmp_dir,
                       [&](comet::CTStringView file_path) {
                         for (const auto& path : files_to_test) {
                           if (path == file_path) {
                             ++count;
                             break;
                           }
                         }
                       });

    REQUIRE(count == files_to_test.size());

    std::array<const comet::TString, 5> all_test{
        {comet::comettests::tmp_dir / COMET_TCHAR("/test"),
         comet::comettests::tmp_dir / COMET_TCHAR("/test3"),
         comet::comettests::tmp_dir / COMET_TCHAR("/test5"),
         comet::comettests::tmp_dir / COMET_TCHAR("/test8"),
         comet::comettests::tmp_dir / COMET_TCHAR("/test9")}};
    count = 0;

    comet::ForEachFileAndDirectory(comet::comettests::tmp_dir,
                                   [&](comet::CTStringView other_path) {
                                     for (const auto& path : all_test) {
                                       if (path == other_path) {
                                         ++count;
                                         break;
                                       }
                                     }
                                   });

    REQUIRE(count == all_test.size());

    std::array<const comet::TString, 1> does_not_exist{
        {comet::comettests::tmp_dir / COMET_TCHAR("/DOESNOTEXIST")}};
    count = 0;

    comet::ForEachDirectory(comet::comettests::tmp_dir,
                            [&](comet::CTStringView directory_path) {
                              for (const auto& path : does_not_exist) {
                                if (path == directory_path) {
                                  ++count;
                                  break;
                                }
                              }
                            });

    REQUIRE(count == 0);
    count = 0;

    comet::ForEachFile(comet::comettests::tmp_dir,
                       [&](comet::CTStringView file_path) {
                         for (const auto& path : all_test) {
                           if (path == file_path) {
                             ++count;
                             break;
                           }
                         }
                       });

    REQUIRE(count == 2);
    count = 0;

    const auto file_path{
        comet::Append(comet::comettests::tmp_dir, COMET_TCHAR("/test8"))};

    // Count should be 0, because a file is not a directory.
    comet::ForEachDirectory(
        file_path, [&](comet::CTStringView directory_path) { ++count; });

    REQUIRE(count == 0);
    count = 0;

    comet::ForEachFile(file_path,
                       [&](comet::CTStringView directory_path) { ++count; });

    REQUIRE(count == 0);
    count = 0;

    comet::ForEachFileAndDirectory(
        file_path, [&](comet::CTStringView directory_path) { ++count; });

    REQUIRE(count == 0);
  }

  SECTION("String operations.") {
    comet::TString test_string{"///test///"};
    comet::RemoveTrailingSlashes(test_string);
    REQUIRE(test_string == COMET_TCHAR("///test"));

    REQUIRE(comet::IsDirectory(comet::comettests::current_dir));
    REQUIRE(!comet::IsFile(comet::comettests::current_dir));

    REQUIRE(comet::GetParentPath(comet::comettests::current_dir) ==
            comet::comettests::current_dir.GenerateSubString(
                0, comet::comettests::current_dir.GetLastIndexOf(
                       COMET_TCHAR('/'))));

#ifdef COMET_WINDOWS
    REQUIRE(comet::GetNormalizedPath(COMET_TCHAR("C:\\..\\..\\.")) ==
            COMET_TCHAR("C:/"));

    REQUIRE(comet::GetNormalizedPath(COMET_TCHAR("C:\\Users\\comet\\test")) ==
            COMET_TCHAR("C:/Users/comet/test"));

    REQUIRE(comet::GetNormalizedPath(COMET_TCHAR("D:\\Users\\comet\\test")) ==
            COMET_TCHAR("D:/Users/comet/test"));

    REQUIRE(comet::GetNormalizedPath(COMET_TCHAR("C:/Users/comet/test")) ==
            COMET_TCHAR("C:/Users/comet/test"));

    REQUIRE(comet::GetNormalizedPath(
                COMET_TCHAR("C:\\..\\..\\.\\Users\\comet\\test\\..")) ==
            COMET_TCHAR("C:/Users/comet/"));

    REQUIRE(comet::GetNormalizedPath(
                COMET_TCHAR("C:\\..\\..\\.\\Users\\comet\\test\\..\\.")) ==
            COMET_TCHAR("C:/Users/comet/"));
#else
    REQUIRE(comet::GetNormalizedPath(COMET_TCHAR("/../../.")) ==
            COMET_TCHAR("/"));

    REQUIRE(comet::GetNormalizedPath(COMET_TCHAR("/home/comet/test")) ==
            COMET_TCHAR("/home/comet/test"));

    REQUIRE(comet::GetNormalizedPath(COMET_TCHAR(
                "/../.././home/comet/test/..")) == COMET_TCHAR("/home/comet/"));

    REQUIRE(comet::GetNormalizedPath(
                COMET_TCHAR("/../.././home/comet/test/../.")) ==
            COMET_TCHAR("/home/comet/"));
#endif  // COMET_WINDOWS

    const auto normalized_path{
        std::filesystem::path{COMET_TCHAR("..")}.lexically_normal()};

    REQUIRE(comet::GetNormalizedPath(COMET_TCHAR("..")) == COMET_TCHAR(".."));

    REQUIRE(comet::GetNormalizedPath(COMET_TCHAR("TEST")) ==
            COMET_TCHAR("TEST"));

    REQUIRE(comet::GetNormalizedPath(COMET_TCHAR("test/..")) ==
            COMET_TCHAR("."));

    REQUIRE(comet::GetNormalizedPath(COMET_TCHAR("test/../test")) ==
            COMET_TCHAR("test"));

    REQUIRE(comet::GetNormalizedPath(COMET_TCHAR("test/.././test")) ==
            COMET_TCHAR("test"));

    const auto file_path{
        comet::Append(comet::comettests::tmp_dir, COMET_TCHAR("/test8"))};
    const auto dir_path{
        comet::Append(comet::comettests::tmp_dir, COMET_TCHAR("/test3"))};
    const auto does_not_exist_path{comet::Append(comet::comettests::tmp_dir,
                                                 COMET_TCHAR("/DOESNOTEXIST"))};

    // For tests with absolute paths, we simply cut the beginning of the
    // string to allow the tests to run on any computer.
    auto tmp_file_path{comet::GetAbsolutePath(file_path)};
    comet::TString buffer{};
    buffer.Reserve(512);
    buffer /= comet::comettests::test_dir;
    buffer /= COMET_TCHAR("/test8");

    REQUIRE(comet::comettests::FormatAbsolutePath(
                tmp_file_path, COMET_TCHAR('/'), 2) == buffer);
    buffer.Clear();
    buffer /= comet::comettests::test_dir;
    buffer /= COMET_TCHAR("/test3");

    auto tmp_dir_path{comet::GetAbsolutePath(dir_path)};

    REQUIRE(comet::comettests::FormatAbsolutePath(
                tmp_dir_path, COMET_TCHAR('/'), 2) == buffer);

    auto tmp_does_not_exist_path{comet::GetAbsolutePath(does_not_exist_path)};
    buffer.Clear();
    buffer /= comet::comettests::test_dir;
    buffer /= COMET_TCHAR("/DOESNOTEXIST");

    REQUIRE(comet::comettests::FormatAbsolutePath(
                tmp_does_not_exist_path, COMET_TCHAR('/'), 2) == buffer);

    REQUIRE(comet::GetRelativePath(COMET_TCHAR("videos"),
                                   COMET_TCHAR("my_documents/videos")) ==
            COMET_TCHAR("../../videos"));

    REQUIRE(comet::GetRelativePath(COMET_TCHAR("my_documents/videos"),
                                   COMET_TCHAR("my_documents")) ==
            COMET_TCHAR("videos"));

    REQUIRE(comet::GetRelativePath(COMET_TCHAR("my_documents"),
                                   COMET_TCHAR("my_documents/videos")) ==
            COMET_TCHAR(".."));

    REQUIRE(comet::GetRelativePath(COMET_TCHAR("my_documents/videos"),
                                   COMET_TCHAR("my_documents/videos")) ==
            COMET_TCHAR("."));

    REQUIRE(comet::GetRelativePath(COMET_TCHAR("my_documents/../../other"),
                                   COMET_TCHAR("my_documents/../videos")) ==
            COMET_TCHAR("../../other"));

    REQUIRE(comet::GetRelativePath(file_path) ==
            comet::Append(comet::comettests::test_dir, COMET_TCHAR("/test8")));

    REQUIRE(comet::GetRelativePath(dir_path) ==
            comet::Append(comet::comettests::test_dir, COMET_TCHAR("/test3")));

    REQUIRE(comet::GetRelativePath(does_not_exist_path) ==
            comet::Append(comet::comettests::test_dir,
                          COMET_TCHAR("/DOESNOTEXIST")));

    tmp_file_path = comet::GetDirectoryPath(file_path);

    REQUIRE(comet::comettests::FormatAbsolutePath(tmp_file_path,
                                                  COMET_TCHAR('/'), 1) ==
            COMET_TCHAR("/") + comet::TString{comet::comettests::test_dir});

    tmp_dir_path = comet::GetDirectoryPath(dir_path);

    REQUIRE(comet::comettests::FormatAbsolutePath(tmp_dir_path,
                                                  COMET_TCHAR('/'), 2) ==
            COMET_TCHAR("/") + comet::Append(comet::comettests::test_dir,
                                             COMET_TCHAR("/test3")));

    tmp_does_not_exist_path = comet::GetDirectoryPath(does_not_exist_path);

    REQUIRE(comet::comettests::FormatAbsolutePath(tmp_does_not_exist_path,
                                                  COMET_TCHAR('/'),
                                                  1) == COMET_TCHAR(""));

    REQUIRE(comet::GetName(file_path) == COMET_TCHAR("test8"));
    REQUIRE(comet::GetName(dir_path) == COMET_TCHAR("test3"));
    REQUIRE(comet::GetName(does_not_exist_path) == COMET_TCHAR("DOESNOTEXIST"));

    REQUIRE(comet::GetExtension(file_path) == COMET_TCHAR(""));
    REQUIRE(comet::GetExtension(file_path) == COMET_TCHAR(""));
    REQUIRE(comet::GetExtension(COMET_TCHAR("/test/test1.txt")) ==
            COMET_TCHAR("txt"));

    REQUIRE(comet::GetExtension(COMET_TCHAR("/test/test1.txt.mp3")) ==
            COMET_TCHAR("mp3"));

    REQUIRE(comet::GetExtension(COMET_TCHAR("/test/test1.txt.MP3")) ==
            COMET_TCHAR("mp3"));

    comet::TString replace_extension_path{"/test/test1.txt.MP3"};
    comet::ReplaceExtension(COMET_TCHAR("fla"), replace_extension_path);

    REQUIRE(replace_extension_path == COMET_TCHAR("/test/test1.txt.fla"));

    REQUIRE(comet::GetExtension(does_not_exist_path) == COMET_TCHAR(""));

    REQUIRE(comet::IsRelative(COMET_TCHAR("../test")));
#ifdef COMET_WINDOWS
    REQUIRE(!comet::IsRelative(COMET_TCHAR("C://test1/test2/test3")));
    REQUIRE(comet::IsAbsolute(COMET_TCHAR("C://test1/test2/test3")));
#endif  // COMET_WINDOWS
#ifdef COMET_UNIX
    REQUIRE(!comet::IsRelative(COMET_TCHAR("/test1/test2/test3")));
    REQUIRE(comet::IsAbsolute(COMET_TCHAR("/test1/test2/test3")));
#endif  // COMET_UNIX
    REQUIRE(!comet::IsAbsolute(COMET_TCHAR("../test")));
  }

  SECTION("State operations.") {
    const auto file_path{
        comet::Append(comet::comettests::tmp_dir, COMET_TCHAR("/test8"))};
    const auto dir_path{
        comet::Append(comet::comettests::tmp_dir, COMET_TCHAR("/test3"))};
    const auto does_not_exist{comet::Append(comet::comettests::tmp_dir,
                                            COMET_TCHAR("/DOESNOTEXIST"))};

    REQUIRE(!comet::IsDirectory(file_path));
    REQUIRE(comet::IsFile(file_path));
    REQUIRE(comet::Exists(file_path));

    REQUIRE(comet::IsDirectory(dir_path));
    REQUIRE(!comet::IsFile(dir_path));
    REQUIRE(comet::Exists(dir_path));

    REQUIRE(!comet::IsDirectory(does_not_exist));
    REQUIRE(!comet::IsFile(does_not_exist));
    REQUIRE(!comet::IsFile(does_not_exist));

    const auto empty_directory_path{
        comet::Append(comet::comettests::tmp_dir, COMET_TCHAR("empty_dir"))};
    const auto non_empty_directory_path{comet::Append(
        comet::comettests::tmp_dir, COMET_TCHAR("non_empty_dir"))};

    const auto empty_file_path{
        comet::Append(non_empty_directory_path, COMET_TCHAR("empty_file"))};
    const auto non_empty_file_path{
        comet::Append(non_empty_directory_path, COMET_TCHAR("non_empty_file"))};

    comet::CreateDirectory(empty_directory_path);
    comet::CreateDirectory(non_empty_directory_path);

    comet::CreateFile(empty_file_path);
    comet::CreateFile(non_empty_file_path);

    WriteStrToFile(non_empty_file_path, "Hello World");

    REQUIRE(comet::IsEmpty(empty_directory_path));
    REQUIRE(!comet::IsEmpty(non_empty_directory_path));
    REQUIRE(comet::IsEmpty(empty_file_path));
    REQUIRE(!comet::IsEmpty(non_empty_file_path));
  }

  SECTION("Delete operations.") {
    REQUIRE(!comet::Remove(comet::comettests::tmp_dir));
    REQUIRE(comet::Remove(comet::comettests::tmp_dir, true));
  }
}