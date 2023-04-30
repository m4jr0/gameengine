// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_precompile.h"

#include "comet/core/file_system.h"

#include "catch.hpp"

#include "comet/core/string.h"

namespace comet {
namespace comettests {
const std::string test_dir{"comettests_tests_file_system"};
auto current_dir{comet::GetCurrentDirectory()};
auto tmp_dir{comet::Append(current_dir, test_dir)};

std::string FormatAbsolutePath(std::string_view absolute_path,
                               std::string_view to_search, const uindex count) {
  if (absolute_path.size() == 0 || count == 0) {
    return std::string{absolute_path};
  }

  uindex index_to_cut{GetLastNthPos(absolute_path, to_search, count)};

  if (index_to_cut == kInvalidIndex) {
    return "";
  }

  return std::string{absolute_path.substr(index_to_cut, absolute_path.size())};
}
}  // namespace comettests
}  // namespace comet

TEST_CASE("File system management", "[comet::utils::filesystem]") {
  SECTION("Create operations.") {
    REQUIRE(
        !comet::CreateFile(comet::Append(comet::comettests::tmp_dir, "/test")));

    REQUIRE(comet::CreateFile(
        comet::Append(comet::comettests::tmp_dir, "/test"), true));

    comet::GetChecksum(comet::Append(comet::comettests::tmp_dir, "/test"));

    REQUIRE(!comet::CreateFile(
        comet::Append(comet::comettests::tmp_dir, "/test2/")));

    REQUIRE(comet::CreateDirectory(
        comet::Append(comet::comettests::tmp_dir, "/test3")));

    REQUIRE(comet::CreateDirectory(
        comet::Append(comet::comettests::tmp_dir, "/test4/")));

    REQUIRE(!comet::CreateDirectory(
        comet::Append(comet::comettests::tmp_dir, "/test5/test6")));

    REQUIRE(comet::CreateDirectory(
        comet::Append(comet::comettests::tmp_dir, "/test5/test6"), true));
  }

  SECTION("Read & write operations.") {
    const auto& test_write{std::string("test_write")};

    // The file does not exist (yet).
    REQUIRE(comet::WriteStrToFile(
        comet::Append(comet::comettests::tmp_dir, "/test7"), test_write));

    // The file already exists.
    REQUIRE(comet::WriteStrToFile(
        comet::Append(comet::comettests::tmp_dir, "/test"), test_write));

    std::string test_read{};

    REQUIRE(comet::ReadStrFromFile(
        comet::Append(comet::comettests::tmp_dir, "/test7"), test_read));

    REQUIRE(test_read == test_write);
  }

  SECTION("Move operations.") {
    // Checking with something that does not exist.
    REQUIRE(
        !comet::Move(comet::Append(comet::comettests::tmp_dir, "/DOESNOTEXIST"),
                     comet::Append(comet::comettests::tmp_dir, "/test8")));

    // Checking with a file.
    REQUIRE(comet::Move(comet::Append(comet::comettests::tmp_dir, "/test7"),
                        comet::Append(comet::comettests::tmp_dir, "/test8")));

    // Checking with a file on itself.
    REQUIRE(comet::Move(comet::Append(comet::comettests::tmp_dir, "/test8"),
                        comet::Append(comet::comettests::tmp_dir, "/test8")));

    // Checking with a folder.
    REQUIRE(comet::Move(comet::Append(comet::comettests::tmp_dir, "/test4"),
                        comet::Append(comet::comettests::tmp_dir, "/test9")));

    // Checking with a folder on itself.
    REQUIRE(comet::Move(comet::Append(comet::comettests::tmp_dir, "/test9"),
                        comet::Append(comet::comettests::tmp_dir, "/test9")));

    // Checking with a folder on a file.
    REQUIRE(!comet::Move(comet::Append(comet::comettests::tmp_dir, "/test9"),
                         comet::Append(comet::comettests::tmp_dir, "/test8")));

    // Checking with a file on a folder.
    REQUIRE(!comet::Move(comet::Append(comet::comettests::tmp_dir, "/test8"),
                         comet::Append(comet::comettests::tmp_dir, "/test9")));
  }

  SECTION("List operations.") {
    std::vector<std::string> dir_to_test;

    dir_to_test.push_back(comet::Append(comet::comettests::tmp_dir, "/test3"));
    dir_to_test.push_back(comet::Append(comet::comettests::tmp_dir, "/test5"));
    dir_to_test.push_back(comet::Append(comet::comettests::tmp_dir, "/test9"));

    REQUIRE(comet::ListDirectories(comet::comettests::tmp_dir, true) ==
            dir_to_test);

    std::vector<std::string> files_to_test;

    files_to_test.push_back(comet::Append(comet::comettests::tmp_dir, "/test"));
    files_to_test.push_back(
        comet::Append(comet::comettests::tmp_dir, "/test8"));

    REQUIRE(comet::ListFiles(comet::comettests::tmp_dir, true) ==
            files_to_test);

    std::vector<std::string> all_test;

    all_test.push_back(comet::Append(comet::comettests::tmp_dir, "/test"));
    all_test.push_back(comet::Append(comet::comettests::tmp_dir, "/test3"));
    all_test.push_back(comet::Append(comet::comettests::tmp_dir, "/test5"));
    all_test.push_back(comet::Append(comet::comettests::tmp_dir, "/test8"));
    all_test.push_back(comet::Append(comet::comettests::tmp_dir, "/test9"));

    REQUIRE(comet::ListAll(comet::comettests::tmp_dir, true) == all_test);

    const auto does_not_exist{
        comet::Append(comet::comettests::tmp_dir, "/DOESNOTEXIST")};
    std::vector<std::string> last_tests;  // Empty list.

    REQUIRE(comet::ListFiles(does_not_exist, true) == last_tests);

    REQUIRE(comet::ListDirectories(does_not_exist, true) == last_tests);

    REQUIRE(comet::ListAll(does_not_exist, true) == last_tests);

    const auto file_path{comet::Append(comet::comettests::tmp_dir, "/test8")};

    // Should be empty, because a file is not a directory.
    REQUIRE(comet::ListDirectories(file_path, true) == last_tests);

    last_tests.push_back(file_path);  // One file only.

    REQUIRE(comet::ListFiles(file_path, true) == last_tests);
  }

  SECTION("String operations.") {
    std::string test_string = "///test///";

    comet::RemoveLeadingSlashes(test_string);

    REQUIRE(test_string == "test///");

    comet::RemoveTrailingSlashes(test_string);

    REQUIRE(test_string == "test");

    REQUIRE(comet::IsDirectory(comet::comettests::current_dir));
    REQUIRE(!comet::IsFile(comet::comettests::current_dir));

    REQUIRE(comet::GetParentPath(comet::comettests::current_dir) ==
            comet::comettests::current_dir.substr(
                0, comet::comettests::current_dir.find_last_of("/")));

    REQUIRE(comet::GetNormalizedPath("test/.././test") == "test");

    const auto file_path{comet::Append(comet::comettests::tmp_dir, "/test8")};
    const auto dir_path{comet::Append(comet::comettests::tmp_dir, "/test3")};
    const auto does_not_exist_path{
        comet::Append(comet::comettests::tmp_dir, "/DOESNOTEXIST")};

    // For tests with absolute paths, we simply cut the beginning of the
    // string to allow the tests to run on any computer.
    auto tmp_file_path{comet::GetAbsolutePath(file_path)};

    REQUIRE(comet::comettests::FormatAbsolutePath(tmp_file_path, "/", 2) ==
            "/" + comet::Append(comet::comettests::test_dir, "/test8"));

    auto tmp_dir_path{comet::GetAbsolutePath(dir_path)};

    REQUIRE(comet::comettests::FormatAbsolutePath(tmp_dir_path, "/", 2) ==
            "/" + comet::Append(comet::comettests::test_dir, "/test3"));

    auto tmp_does_not_exist_path{comet::GetAbsolutePath(does_not_exist_path)};

    REQUIRE(comet::comettests::FormatAbsolutePath(tmp_does_not_exist_path, "/",
                                                  2) ==
            "/" + comet::Append(comet::comettests::test_dir, "/DOESNOTEXIST"));

    REQUIRE(comet::GetRelativePath(file_path) ==
            comet::Append(comet::comettests::test_dir, "/test8"));

    REQUIRE(comet::GetRelativePath(dir_path) ==
            comet::Append(comet::comettests::test_dir, "/test3"));

    REQUIRE(comet::GetRelativePath(does_not_exist_path) ==
            comet::Append(comet::comettests::test_dir, "/DOESNOTEXIST"));

    tmp_file_path = comet::GetDirectoryPath(file_path);

    REQUIRE(comet::comettests::FormatAbsolutePath(tmp_file_path, "/", 1) ==
            "/" + comet::comettests::test_dir);

    tmp_dir_path = comet::GetDirectoryPath(dir_path);

    REQUIRE(comet::comettests::FormatAbsolutePath(tmp_dir_path, "/", 2) ==
            "/" + comet::Append(comet::comettests::test_dir, "/test3"));

    tmp_does_not_exist_path = comet::GetDirectoryPath(does_not_exist_path);

    REQUIRE(comet::comettests::FormatAbsolutePath(tmp_does_not_exist_path, "/",
                                                  1) == "");

    REQUIRE(comet::GetName(file_path) == "test8");
    REQUIRE(comet::GetName(dir_path) == "test3");
    REQUIRE(comet::GetName(does_not_exist_path) == "DOESNOTEXIST");

    REQUIRE(comet::GetNameView(file_path) == "test8");
    REQUIRE(comet::GetNameView(dir_path) == "test3");
    REQUIRE(comet::GetNameView(does_not_exist_path) == "DOESNOTEXIST");

    REQUIRE(comet::GetExtension(file_path) == "");
    REQUIRE(comet::GetExtension(file_path) == "");
    REQUIRE(comet::GetExtension("/test/test1.txt") == "txt");

    REQUIRE(comet::GetExtension("/test/test1.txt.mp3") == "mp3");

    REQUIRE(comet::GetExtension("/test/test1.txt.MP3") == "mp3");

    REQUIRE(comet::ReplaceExtensionToCopy("fla", "/test/test1.txt.MP3") ==
            "/test/test1.txt.fla");

    REQUIRE(comet::GetExtension(does_not_exist_path) == "");

    REQUIRE(comet::IsRelative("../test"));
#ifdef COMET_WINDOWS
    REQUIRE(!comet::IsRelative("C://test1/test2/test3"));
    REQUIRE(comet::IsAbsolute("C://test1/test2/test3"));
#endif  // COMET_WINDOWS
#ifdef COMET_UNIX
    REQUIRE(!comet::IsRelative("/test1/test2/test3"));
    REQUIRE(comet::IsAbsolute("/test1/test2/test3"));
#endif  // COMET_UNIX
    REQUIRE(!comet::IsAbsolute("../test"));
  }

  SECTION("State operations.") {
    const auto file_path{comet::Append(comet::comettests::tmp_dir, "/test8")};
    const auto dir_path{comet::Append(comet::comettests::tmp_dir, "/test3")};
    const auto does_not_exist{
        comet::Append(comet::comettests::tmp_dir, "/DOESNOTEXIST")};

    REQUIRE(!comet::IsDirectory(file_path));
    REQUIRE(comet::IsFile(file_path));
    REQUIRE(comet::Exists(file_path));

    REQUIRE(comet::IsDirectory(dir_path));
    REQUIRE(!comet::IsFile(dir_path));
    REQUIRE(comet::Exists(dir_path));

    REQUIRE(!comet::IsDirectory(does_not_exist));
    REQUIRE(!comet::IsFile(does_not_exist));
    REQUIRE(!comet::IsFile(does_not_exist));
  }

  SECTION("Delete operations.") {
    REQUIRE(!comet::Remove(comet::comettests::tmp_dir));
    REQUIRE(comet::Remove(comet::comettests::tmp_dir, true));
  }
}