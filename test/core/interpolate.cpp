#include <catch2/catch_test_macros.hpp>
#include <i18n_keyval/core/interpolate.hpp>
#include <string>
#include <vector>

TEST_CASE("interpolate: named parameter", "[core]")
{
  std::string text = "Hello {{name}}!";
  i18n::interpolate(text, std::vector<i18n::interpolation_param>{{"name", "Alice"}});

  REQUIRE(text == "Hello Alice!");
}

TEST_CASE("interpolate: positional parameter via a stringified index", "[core]")
{
  std::string text = "{{1}} depends on {{0}}";
  i18n::interpolate(text, std::vector<i18n::interpolation_param>{{"0", "core"}, {"1", "api"}});

  REQUIRE(text == "api depends on core");
}

TEST_CASE("interpolate: named and positional parameters mixed in one string", "[core]")
{
  std::string text = "{{name}} completed task {{0}} of {{1}}";
  i18n::interpolate(text,
                     std::vector<i18n::interpolation_param>{{"name", "Alice"}, {"0", "3"}, {"1", "10"}});

  REQUIRE(text == "Alice completed task 3 of 10");
}

TEST_CASE("interpolate: every occurrence of a placeholder is replaced, not just the first", "[core]")
{
  std::string text = "{{count}} of {{count}} done";
  i18n::interpolate(text, std::vector<i18n::interpolation_param>{{"count", "3"}});

  REQUIRE(text == "3 of 3 done");
}

TEST_CASE("interpolate: an unmatched placeholder is left untouched", "[core]")
{
  std::string text = "Hello {{name}}, you have {{count}} messages";
  i18n::interpolate(text, std::vector<i18n::interpolation_param>{{"count", "3"}});

  REQUIRE(text == "Hello {{name}}, you have 3 messages");
}

TEST_CASE("interpolate: an unterminated \"{{\" does not crash or loop", "[core]")
{
  std::string text = "broken {{ placeholder with no closing brace";
  i18n::interpolate(text, std::vector<i18n::interpolation_param>{{"placeholder", "value"}});

  REQUIRE(text == "broken {{ placeholder with no closing brace");
}

TEST_CASE("interpolate: no placeholders and no params are both fine", "[core]")
{
  std::string text = "nothing to substitute here";
  i18n::interpolate(text, {});

  REQUIRE(text == "nothing to substitute here");
}

TEST_CASE("interpolate: a replacement value containing \"{{\" is not re-scanned", "[core]")
{
  std::string text = "value is {{a}}";
  i18n::interpolate(text, std::vector<i18n::interpolation_param>{{"a", "{{b}}"}});

  REQUIRE(text == "value is {{b}}");
}
