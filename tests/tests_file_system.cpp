// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include <iostream>
#include <memory>

#include "boost/algorithm/string/find.hpp"
#include "tests_game.hpp"
#include "utils/file_system.hpp"

namespace komatests {
const std::string test_dir = "komatests_tests_file_system";
auto current_dir = koma::filesystem::GetCurrentDirectory();
auto tmp_dir = current_dir + "/" + test_dir;

std::string FormatAbsolutePath(const std::string &absolute_path,
                               const std::string &to_search, const int index) {
  auto it = boost::find_nth(absolute_path, to_search, -index);

  auto index_to_cut = std::distance(absolute_path.begin(), it.begin());
  const auto formatted_absolute_path =
      absolute_path.substr(index_to_cut, absolute_path.length());

  return formatted_absolute_path;
}
}  // namespace komatests

TEST_CASE("File system management", "[koma::filesystem]") {
  SECTION("Create operations.") {
    REQUIRE(!koma::filesystem::CreateFile(komatests::tmp_dir + "/test"));
    REQUIRE(koma::filesystem::CreateFile(komatests::tmp_dir + "/test", true));
    REQUIRE(!koma::filesystem::CreateFile(komatests::tmp_dir + "/test2/"));
    REQUIRE(koma::filesystem::CreateDirectory(komatests::tmp_dir + "/test3"));

    REQUIRE(koma::filesystem::CreateDirectory(komatests::tmp_dir + "/test4/"));

    REQUIRE(!koma::filesystem::CreateDirectory(komatests::tmp_dir +
                                               "/test5/test6"));

    REQUIRE(koma::filesystem::CreateDirectory(
        komatests::tmp_dir + "/test5/test6", true));
  }

  SECTION("Read & write operations.") {
    std::string test_write = std::string("test_write");

    // The file does not exist (yet).
    REQUIRE(koma::filesystem::WriteToFile(komatests::tmp_dir + "/test7",
                                          test_write));

    // The file already exists.
    REQUIRE(koma::filesystem::WriteToFile(komatests::tmp_dir + "/test",
                                          test_write));

    std::string test_read;

    REQUIRE(
        koma::filesystem::ReadFile(komatests::tmp_dir + "/test7", &test_read));

    REQUIRE(test_read == test_write);
  }

  SECTION("Move operations.") {
    // Checking with something that does not exist.
    REQUIRE(!koma::filesystem::Move(komatests::tmp_dir + "/DOESNOTEXIST",
                                    komatests::tmp_dir + "/test8"));

    // Checking with a file.
    REQUIRE(koma::filesystem::Move(komatests::tmp_dir + "/test7",
                                   komatests::tmp_dir + "/test8"));

    // Checking with a file on itself.
    REQUIRE(koma::filesystem::Move(komatests::tmp_dir + "/test8",
                                   komatests::tmp_dir + "/test8"));

    // Checking with a folder.
    REQUIRE(koma::filesystem::Move(komatests::tmp_dir + "/test4",
                                   komatests::tmp_dir + "/test9"));

    // Checking with a folder on itself.
    REQUIRE(koma::filesystem::Move(komatests::tmp_dir + "/test9",
                                   komatests::tmp_dir + "/test9"));

    // Checking with a folder on a file.
    REQUIRE(!koma::filesystem::Move(komatests::tmp_dir + "/test9",
                                    komatests::tmp_dir + "/test8"));

    // Checking with a file on a folder.
    REQUIRE(!koma::filesystem::Move(komatests::tmp_dir + "/test8",
                                    komatests::tmp_dir + "/test9"));
  }

  SECTION("List operations.") {
    std::vector<std::string> dir_to_test;

    dir_to_test.push_back(komatests::tmp_dir + "/test3");
    dir_to_test.push_back(komatests::tmp_dir + "/test5");
    dir_to_test.push_back(komatests::tmp_dir + "/test9");

    REQUIRE(koma::filesystem::ListDirectories(komatests::tmp_dir, true) ==
            dir_to_test);

    std::vector<std::string> files_to_test;

    files_to_test.push_back(komatests::tmp_dir + "/test");
    files_to_test.push_back(komatests::tmp_dir + "/test8");

    REQUIRE(koma::filesystem::ListFiles(komatests::tmp_dir, true) ==
            files_to_test);

    std::vector<std::string> all_test;

    all_test.push_back(komatests::tmp_dir + "/test");
    all_test.push_back(komatests::tmp_dir + "/test3");
    all_test.push_back(komatests::tmp_dir + "/test5");
    all_test.push_back(komatests::tmp_dir + "/test8");
    all_test.push_back(komatests::tmp_dir + "/test9");

    REQUIRE(koma::filesystem::ListAll(komatests::tmp_dir, true) == all_test);

    const auto does_not_exist = komatests::tmp_dir + "/DOESNOTEXIST";
    std::vector<std::string> last_tests;  // Empty list.

    REQUIRE(koma::filesystem::ListFiles(does_not_exist, true) == last_tests);

    REQUIRE(koma::filesystem::ListDirectories(does_not_exist, true) ==
            last_tests);

    REQUIRE(koma::filesystem::ListAll(does_not_exist, true) == last_tests);

    const auto file_path = komatests::tmp_dir + "/test8";

    // Should be empty, because a file is not a directory.
    REQUIRE(koma::filesystem::ListDirectories(file_path, true) == last_tests);

    last_tests.push_back(file_path);  // One file only.

    REQUIRE(koma::filesystem::ListFiles(file_path, true) == last_tests);
  }

  SECTION("String operations.") {
    std::string test_string = "///test///";

    koma::filesystem::RemoveLeadingSlashes(test_string);

    REQUIRE(test_string == "test///");

    koma::filesystem::RemoveTrailingSlashes(test_string);

    REQUIRE(test_string == "test");

    REQUIRE(koma::filesystem::IsDirectory(komatests::current_dir));
    REQUIRE(!koma::filesystem::IsFile(komatests::current_dir));

    REQUIRE(koma::filesystem::GetParentPath(komatests::current_dir) ==
            komatests::current_dir.substr(
                0, komatests::current_dir.find_last_of("/")));

    REQUIRE(koma::filesystem::GetNormalizedPath("test/.././test") == "test");

    const auto file_path = komatests::tmp_dir + "/test8";
    const auto dir_path = komatests::tmp_dir + "/test3";
    const auto does_not_exist_path = komatests::tmp_dir + "/DOESNOTEXIST";

    // For tests with absolute paths, we simply cut the beginning of the
    // string to allow the tests to run on any computer.
    auto tmp_file_path = koma::filesystem::GetAbsolutePath(file_path);

    REQUIRE(komatests::FormatAbsolutePath(tmp_file_path, "/", 2) ==
            "/" + komatests::test_dir + "/test8");

    auto tmp_dir_path = koma::filesystem::GetAbsolutePath(dir_path);

    REQUIRE(komatests::FormatAbsolutePath(tmp_dir_path, "/", 2) ==
            "/" + komatests::test_dir + "/test3");

    auto tmp_does_not_exist_path =
        koma::filesystem::GetAbsolutePath(does_not_exist_path);

    REQUIRE(komatests::FormatAbsolutePath(tmp_does_not_exist_path, "/", 2) ==
            "/" + komatests::test_dir + "/DOESNOTEXIST");

    REQUIRE(koma::filesystem::GetRelativePath(file_path) ==
            "" + komatests::test_dir + "/test8");

    REQUIRE(koma::filesystem::GetRelativePath(dir_path) ==
            "" + komatests::test_dir + "/test3");

    REQUIRE(koma::filesystem::GetRelativePath(does_not_exist_path) ==
            "" + komatests::test_dir + "/DOESNOTEXIST");

    tmp_file_path = koma::filesystem::GetDirectoryPath(file_path);

    REQUIRE(komatests::FormatAbsolutePath(tmp_file_path, "/", 1) ==
            "/" + komatests::test_dir);

    tmp_dir_path = koma::filesystem::GetDirectoryPath(dir_path);

    REQUIRE(komatests::FormatAbsolutePath(tmp_dir_path, "/", 2) ==
            "/" + komatests::test_dir + "/test3");

    tmp_does_not_exist_path =
        koma::filesystem::GetDirectoryPath(does_not_exist_path);

    REQUIRE(komatests::FormatAbsolutePath(tmp_does_not_exist_path, "/", 1) ==
            "");

    REQUIRE(koma::filesystem::GetName(file_path) == "test8");
    REQUIRE(koma::filesystem::GetName(dir_path) == "test3");
    REQUIRE(koma::filesystem::GetName(does_not_exist_path) == "");

    REQUIRE(koma::filesystem::GetExtension(file_path) == "");
    REQUIRE(koma::filesystem::GetExtension(file_path) == "");
    REQUIRE(koma::filesystem::GetExtension("/test/test1.txt") == "txt");
    REQUIRE(koma::filesystem::GetExtension("/test/test1.txt.mp3") == "mp3");
    REQUIRE(koma::filesystem::GetExtension("/test/test1.txt.MP3") == "mp3");
    REQUIRE(koma::filesystem::GetExtension(does_not_exist_path) == "");

    REQUIRE(koma::filesystem::IsRelative("../test"));
#ifdef _WIN32
    REQUIRE(!koma::filesystem::IsRelative("C://test1/test2/test3"));
    REQUIRE(koma::filesystem::IsAbsolute("C://test1/test2/test3"));
#endif  // _WIN32
#ifdef __linux__
    REQUIRE(!koma::filesystem::IsRelative("/test1/test2/test3"));
    REQUIRE(koma::filesystem::IsAbsolute("/test1/test2/test3"));
#endif  // __linux__
    REQUIRE(!koma::filesystem::IsAbsolute("../test"));
  }

  SECTION("State operations.") {
    const auto file_path = komatests::tmp_dir + "/test8";
    const auto dir_path = komatests::tmp_dir + "/test3";
    const auto does_not_exist = komatests::tmp_dir + "/DOESNOTEXIST";

    REQUIRE(!koma::filesystem::IsDirectory(file_path));
    REQUIRE(koma::filesystem::IsFile(file_path));
    REQUIRE(koma::filesystem::IsExist(file_path));

    REQUIRE(koma::filesystem::IsDirectory(dir_path));
    REQUIRE(!koma::filesystem::IsFile(dir_path));
    REQUIRE(koma::filesystem::IsExist(dir_path));

    REQUIRE(!koma::filesystem::IsDirectory(does_not_exist));
    REQUIRE(!koma::filesystem::IsFile(does_not_exist));
    REQUIRE(!koma::filesystem::IsFile(does_not_exist));
  }

  SECTION("Delete operations.") {
    REQUIRE(!koma::filesystem::Remove(komatests::tmp_dir));
    REQUIRE(koma::filesystem::Remove(komatests::tmp_dir, true));
  }
}
