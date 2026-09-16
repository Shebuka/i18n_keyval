#include <catch2/catch_test_macros.hpp>
#include <fstream>
#include <i18n_keyval/util/file.hpp>

TEST_CASE("reading file", "[core]")
{
  const auto filepath = std::filesystem::path{"data/dummy.txt"};
  const auto content = i18n::util::read_file(filepath);

  REQUIRE(content == "dummy content\n");
}

TEST_CASE("reading a file preserves embedded NUL bytes", "[core]")
{
  // Regression test for F11: read_file used to open in text mode and
  // build a std::string from a NUL-terminated char*, so any file with an
  // embedded NUL byte before EOF was silently truncated at that byte
  // (verified: a 6-byte file with a NUL at byte 2 came back as a 2-byte
  // string before the fix).
  const auto filepath = std::filesystem::path{"read_file_embedded_nul_test.bin"};
  const std::string expected("AB\0CD\n", 6);

  {
    std::ofstream file(filepath, std::ios::binary);
    file.write(expected.data(), static_cast<std::streamsize>(expected.size()));
  }

  const auto content = i18n::util::read_file(filepath);
  std::filesystem::remove(filepath);

  REQUIRE(content == expected);
}

TEST_CASE("reading a missing file returns an empty string", "[core]")
{
  const auto content = i18n::util::read_file("data/this_file_does_not_exist.txt");

  REQUIRE(content == "");
}
