#include <catch2/catch_test_macros.hpp>
#include <i18n_keyval/util/split.hpp>
#include <vector>

TEST_CASE("split", "[core]")
{
  std::vector<std::string> parts{};

  SECTION("splitting empty")
  {
    for (std::string_view part : i18n::util::split("", '.'))
    {
      parts.push_back(std::string(part));
    }

    REQUIRE(parts.empty());
  }

  SECTION("splitting one part")
  {
    for (std::string_view part : i18n::util::split("animals", '.'))
    {
      parts.push_back(std::string(part));
    }

    REQUIRE(parts.size() == 1);
    REQUIRE(parts[0] == "animals");
  }

  SECTION("splitting two parts")
  {
    for (std::string_view part : i18n::util::split("animals.felines", '.'))
    {
      parts.push_back(std::string(part));
    }

    REQUIRE(parts.size() == 2);
    REQUIRE(parts[0] == "animals");
    REQUIRE(parts[1] == "felines");
  }

  SECTION("splitting three parts")
  {
    for (std::string_view part : i18n::util::split("animals.felines.cat", '.'))
    {
      parts.push_back(std::string(part));
    }

    REQUIRE(parts.size() == 3);
    REQUIRE(parts[0] == "animals");
    REQUIRE(parts[1] == "felines");
    REQUIRE(parts[2] == "cat");
  }

  SECTION("splitting with custom delimiter")
  {
    for (std::string_view part : i18n::util::split("animals:felines:cat", ':'))
    {
      parts.push_back(std::string(part));
    }

    REQUIRE(parts.size() == 3);
    REQUIRE(parts[0] == "animals");
    REQUIRE(parts[1] == "felines");
    REQUIRE(parts[2] == "cat");
  }

  SECTION("leading delimiter produces a leading empty segment")
  {
    for (std::string_view part : i18n::util::split(".animals", '.'))
    {
      parts.push_back(std::string(part));
    }

    REQUIRE(parts.size() == 2);
    REQUIRE(parts[0] == "");
    REQUIRE(parts[1] == "animals");
  }

  SECTION("trailing delimiter produces a trailing empty segment")
  {
    for (std::string_view part : i18n::util::split("animals.", '.'))
    {
      parts.push_back(std::string(part));
    }

    REQUIRE(parts.size() == 2);
    REQUIRE(parts[0] == "animals");
    REQUIRE(parts[1] == "");
  }

  SECTION("duplicated delimiter produces an empty segment in between")
  {
    for (std::string_view part : i18n::util::split("animals..cat", '.'))
    {
      parts.push_back(std::string(part));
    }

    REQUIRE(parts.size() == 3);
    REQUIRE(parts[0] == "animals");
    REQUIRE(parts[1] == "");
    REQUIRE(parts[2] == "cat");
  }
}
