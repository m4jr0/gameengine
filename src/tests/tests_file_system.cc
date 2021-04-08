// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "boost/algorithm/string/find.hpp"
#include "catch.hpp"
#include "comet/utils/file_system.h"
#include "comet_precompile.h"

namespace comettests {
const std::string test_dir = "comettests_tests_file_system";
auto current_dir = comet::utils::filesystem::GetCurrentDirectory();
auto tmp_dir = current_dir + "/" + test_dir;

std::string FormatAbsolutePath(const std::string &absolute_path,
                               const std::string &to_search, const int index) {
  auto it = boost::find_nth(absolute_path, to_search, -index);

  auto index_to_cut = std::distance(absolute_path.begin(), it.begin());
  const auto formatted_absolute_path =
      absolute_path.substr(index_to_cut, absolute_path.length());

  return formatted_absolute_path;
}
}  // namespace comettests

TEST_CASE("File system management", "[comet::utils::filesystem]") {
  SECTION("Create operations.") {
    REQUIRE(
        !comet::utils::filesystem::CreateFile(comettests::tmp_dir + "/test"));

    REQUIRE(comet::utils::filesystem::CreateFile(comettests::tmp_dir + "/test",
                                                 true));
    REQUIRE(
        !comet::utils::filesystem::CreateFile(comettests::tmp_dir + "/test2/"));

    REQUIRE(comet::utils::filesystem::CreateDirectory(comettests::tmp_dir +
                                                      "/test3"));

    REQUIRE(comet::utils::filesystem::CreateDirectory(comettests::tmp_dir +
                                                      "/test4/"));

    REQUIRE(!comet::utils::filesystem::CreateDirectory(comettests::tmp_dir +
                                                       "/test5/test6"));

    REQUIRE(comet::utils::filesystem::CreateDirectory(
        comettests::tmp_dir + "/test5/test6", true));
  }

  SECTION("Read & write operations.") {
    std::string test_write = std::string("test_write");

    // The file does not exist (yet).
    REQUIRE(comet::utils::filesystem::WriteToFile(
        comettests::tmp_dir + "/test7", test_write));

    // The file already exists.
    REQUIRE(comet::utils::filesystem::WriteToFile(comettests::tmp_dir + "/test",
                                                  test_write));

    std::string test_read;

    REQUIRE(comet::utils::filesystem::ReadFile(comettests::tmp_dir + "/test7",
                                               &test_read));

    REQUIRE(test_read == test_write);
  }

  SECTION("Move operations.") {
    // Checking with something that does not exist.
    REQUIRE(!comet::utils::filesystem::Move(
        comettests::tmp_dir + "/DOESNOTEXIST", comettests::tmp_dir + "/test8"));

    // Checking with a file.
    REQUIRE(comet::utils::filesystem::Move(comettests::tmp_dir + "/test7",
                                           comettests::tmp_dir + "/test8"));

    // Checking with a file on itself.
    REQUIRE(comet::utils::filesystem::Move(comettests::tmp_dir + "/test8",
                                           comettests::tmp_dir + "/test8"));

    // Checking with a folder.
    REQUIRE(comet::utils::filesystem::Move(comettests::tmp_dir + "/test4",
                                           comettests::tmp_dir + "/test9"));

    // Checking with a folder on itself.
    REQUIRE(comet::utils::filesystem::Move(comettests::tmp_dir + "/test9",
                                           comettests::tmp_dir + "/test9"));

    // Checking with a folder on a file.
    REQUIRE(!comet::utils::filesystem::Move(comettests::tmp_dir + "/test9",
                                            comettests::tmp_dir + "/test8"));

    // Checking with a file on a folder.
    REQUIRE(!comet::utils::filesystem::Move(comettests::tmp_dir + "/test8",
                                            comettests::tmp_dir + "/test9"));
  }

  SECTION("List operations.") {
    std::vector<std::string> dir_to_test;

    dir_to_test.push_back(comettests::tmp_dir + "/test3");
    dir_to_test.push_back(comettests::tmp_dir + "/test5");
    dir_to_test.push_back(comettests::tmp_dir + "/test9");

    REQUIRE(comet::utils::filesystem::ListDirectories(comettests::tmp_dir,
                                                      true) == dir_to_test);

    std::vector<std::string> files_to_test;

    files_to_test.push_back(comettests::tmp_dir + "/test");
    files_to_test.push_back(comettests::tmp_dir + "/test8");

    REQUIRE(comet::utils::filesystem::ListFiles(comettests::tmp_dir, true) ==
            files_to_test);

    std::vector<std::string> all_test;

    all_test.push_back(comettests::tmp_dir + "/test");
    all_test.push_back(comettests::tmp_dir + "/test3");
    all_test.push_back(comettests::tmp_dir + "/test5");
    all_test.push_back(comettests::tmp_dir + "/test8");
    all_test.push_back(comettests::tmp_dir + "/test9");

    REQUIRE(comet::utils::filesystem::ListAll(comettests::tmp_dir, true) ==
            all_test);

    const auto does_not_exist = comettests::tmp_dir + "/DOESNOTEXIST";
    std::vector<std::string> last_tests;  // Empty list.

    REQUIRE(comet::utils::filesystem::ListFiles(does_not_exist, true) ==
            last_tests);

    REQUIRE(comet::utils::filesystem::ListDirectories(does_not_exist, true) ==
            last_tests);

    REQUIRE(comet::utils::filesystem::ListAll(does_not_exist, true) ==
            last_tests);

    const auto file_path = comettests::tmp_dir + "/test8";

    // Should be empty, because a file is not a directory.
    REQUIRE(comet::utils::filesystem::ListDirectories(file_path, true) ==
            last_tests);

    last_tests.push_back(file_path);  // One file only.

    REQUIRE(comet::utils::filesystem::ListFiles(file_path, true) == last_tests);
  }

  SECTION("String operations.") {
    std::string test_string = "///test///";

    comet::utils::filesystem::RemoveLeadingSlashes(test_string);

    REQUIRE(test_string == "test///");

    comet::utils::filesystem::RemoveTrailingSlashes(test_string);

    REQUIRE(test_string == "test");

    REQUIRE(comet::utils::filesystem::IsDirectory(comettests::current_dir));
    REQUIRE(!comet::utils::filesystem::IsFile(comettests::current_dir));

    REQUIRE(comet::utils::filesystem::GetParentPath(comettests::current_dir) ==
            comettests::current_dir.substr(
                0, comettests::current_dir.find_last_of("/")));

    REQUIRE(comet::utils::filesystem::GetNormalizedPath("test/.././test") ==
            "test");

    const auto file_path = comettests::tmp_dir + "/test8";
    const auto dir_path = comettests::tmp_dir + "/test3";
    const auto does_not_exist_path = comettests::tmp_dir + "/DOESNOTEXIST";

    // For tests with absolute paths, we simply cut the beginning of the
    // string to allow the tests to run on any computer.
    auto tmp_file_path = comet::utils::filesystem::GetAbsolutePath(file_path);

    REQUIRE(comettests::FormatAbsolutePath(tmp_file_path, "/", 2) ==
            "/" + comettests::test_dir + "/test8");

    auto tmp_dir_path = comet::utils::filesystem::GetAbsolutePath(dir_path);

    REQUIRE(comettests::FormatAbsolutePath(tmp_dir_path, "/", 2) ==
            "/" + comettests::test_dir + "/test3");

    auto tmp_does_not_exist_path =
        comet::utils::filesystem::GetAbsolutePath(does_not_exist_path);

    REQUIRE(comettests::FormatAbsolutePath(tmp_does_not_exist_path, "/", 2) ==
            "/" + comettests::test_dir + "/DOESNOTEXIST");

    REQUIRE(comet::utils::filesystem::GetRelativePath(file_path) ==
            "" + comettests::test_dir + "/test8");

    REQUIRE(comet::utils::filesystem::GetRelativePath(dir_path) ==
            "" + comettests::test_dir + "/test3");

    REQUIRE(comet::utils::filesystem::GetRelativePath(does_not_exist_path) ==
            "" + comettests::test_dir + "/DOESNOTEXIST");

    tmp_file_path = comet::utils::filesystem::GetDirectoryPath(file_path);

    REQUIRE(comettests::FormatAbsolutePath(tmp_file_path, "/", 1) ==
            "/" + comettests::test_dir);

    tmp_dir_path = comet::utils::filesystem::GetDirectoryPath(dir_path);

    REQUIRE(comettests::FormatAbsolutePath(tmp_dir_path, "/", 2) ==
            "/" + comettests::test_dir + "/test3");

    tmp_does_not_exist_path =
        comet::utils::filesystem::GetDirectoryPath(does_not_exist_path);

    REQUIRE(comettests::FormatAbsolutePath(tmp_does_not_exist_path, "/", 1) ==
            "");

    REQUIRE(comet::utils::filesystem::GetName(file_path) == "test8");
    REQUIRE(comet::utils::filesystem::GetName(dir_path) == "test3");
    REQUIRE(comet::utils::filesystem::GetName(does_not_exist_path) == "");

    REQUIRE(comet::utils::filesystem::GetExtension(file_path) == "");
    REQUIRE(comet::utils::filesystem::GetExtension(file_path) == "");
    REQUIRE(comet::utils::filesystem::GetExtension("/test/test1.txt") == "txt");

    REQUIRE(comet::utils::filesystem::GetExtension("/test/test1.txt.mp3") ==
            "mp3");

    REQUIRE(comet::utils::filesystem::GetExtension("/test/test1.txt.MP3") ==
            "mp3");

    REQUIRE(comet::utils::filesystem::GetExtension(does_not_exist_path) == "");

    REQUIRE(comet::utils::filesystem::IsRelative("../test"));
#ifdef _WIN32
    REQUIRE(!comet::utils::filesystem::IsRelative("C://test1/test2/test3"));
    REQUIRE(comet::utils::filesystem::IsAbsolute("C://test1/test2/test3"));
#endif  // _WIN32
#ifdef __linux__
    REQUIRE(!comet::utils::filesystem::IsRelative("/test1/test2/test3"));
    REQUIRE(comet::utils::filesystem::IsAbsolute("/test1/test2/test3"));
#endif  // __linux__
    REQUIRE(!comet::utils::filesystem::IsAbsolute("../test"));
  }

  SECTION("State operations.") {
    const auto file_path = comettests::tmp_dir + "/test8";
    const auto dir_path = comettests::tmp_dir + "/test3";
    const auto does_not_exist = comettests::tmp_dir + "/DOESNOTEXIST";

    REQUIRE(!comet::utils::filesystem::IsDirectory(file_path));
    REQUIRE(comet::utils::filesystem::IsFile(file_path));
    REQUIRE(comet::utils::filesystem::IsExist(file_path));

    REQUIRE(comet::utils::filesystem::IsDirectory(dir_path));
    REQUIRE(!comet::utils::filesystem::IsFile(dir_path));
    REQUIRE(comet::utils::filesystem::IsExist(dir_path));

    REQUIRE(!comet::utils::filesystem::IsDirectory(does_not_exist));
    REQUIRE(!comet::utils::filesystem::IsFile(does_not_exist));
    REQUIRE(!comet::utils::filesystem::IsFile(does_not_exist));
  }

  SECTION("Delete operations.") {
    REQUIRE(!comet::utils::filesystem::Remove(comettests::tmp_dir));
    REQUIRE(comet::utils::filesystem::Remove(comettests::tmp_dir, true));
  }
}
