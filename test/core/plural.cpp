#include <catch2/catch_test_macros.hpp>
#include <i18n_keyval/core/plural.hpp>
#include <string>

using i18n::plural_category;
using i18n::select_plural_category;

TEST_CASE("select_plural_category: ar has the full CLDR category set", "[core]")
{
  REQUIRE(select_plural_category("ar", 0) == plural_category::zero);
  REQUIRE(select_plural_category("ar", 1) == plural_category::one);
  REQUIRE(select_plural_category("ar", 2) == plural_category::two);
  REQUIRE(select_plural_category("ar", 3) == plural_category::few);
  REQUIRE(select_plural_category("ar", 10) == plural_category::few);
  REQUIRE(select_plural_category("ar", 11) == plural_category::many);
  REQUIRE(select_plural_category("ar", 99) == plural_category::many);
  REQUIRE(select_plural_category("ar", 100) == plural_category::other);
  REQUIRE(select_plural_category("ar", 101) == plural_category::other);
  REQUIRE(select_plural_category("ar", 103) == plural_category::few);
  REQUIRE(select_plural_category("ar", 111) == plural_category::many);
}

TEST_CASE("select_plural_category: fr treats both 0 and 1 as \"one\"", "[core]")
{
  REQUIRE(select_plural_category("fr", 0) == plural_category::one);
  REQUIRE(select_plural_category("fr", 1) == plural_category::one);
  REQUIRE(select_plural_category("fr", 2) == plural_category::other);
}

TEST_CASE("select_plural_category: en/es/it are a plain one/other split", "[core]")
{
  for (const auto* language : {"en", "es", "it"})
  {
    REQUIRE(select_plural_category(language, 0) == plural_category::other);
    REQUIRE(select_plural_category(language, 1) == plural_category::one);
    REQUIRE(select_plural_category(language, 2) == plural_category::other);
  }
}

TEST_CASE("select_plural_category: matches by language subtag, not the full locale tag", "[core]")
{
  REQUIRE(select_plural_category("en-US", 1) == plural_category::one);
  REQUIRE(select_plural_category("fr-CA", 0) == plural_category::one);
  REQUIRE(select_plural_category("ar-EG", 3) == plural_category::few);
}

TEST_CASE("select_plural_category: unlisted locales fall back to one/other", "[core]")
{
  REQUIRE(select_plural_category("de", 1) == plural_category::one);
  REQUIRE(select_plural_category("de", 2) == plural_category::other);
  REQUIRE(select_plural_category("", 1) == plural_category::one);
}

TEST_CASE("plural_category_suffix", "[core]")
{
  // std::string(...) rather than comparing std::string_view directly:
  // Catch2's StringMaker<std::string_view> specialization isn't always
  // linkable depending on how its own precompiled library detected C++
  // standard support versus this test binary, so every other test in
  // this suite that touches a string_view already avoids putting one
  // directly into a REQUIRE (see test/util/split.cpp).
  REQUIRE(std::string(i18n::plural_category_suffix(plural_category::zero)) == "_zero");
  REQUIRE(std::string(i18n::plural_category_suffix(plural_category::one)) == "_one");
  REQUIRE(std::string(i18n::plural_category_suffix(plural_category::two)) == "_two");
  REQUIRE(std::string(i18n::plural_category_suffix(plural_category::few)) == "_few");
  REQUIRE(std::string(i18n::plural_category_suffix(plural_category::many)) == "_many");
  REQUIRE(std::string(i18n::plural_category_suffix(plural_category::other)) == "_other");
}
