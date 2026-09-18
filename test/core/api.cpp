#include <catch2/catch_test_macros.hpp>
#include <i18n_keyval/core/api.hpp>
#include <i18n_keyval/translators/basic.hpp>
#include <string>
#include <unordered_map>

struct translator
{
  std::string locale{};
  std::unordered_map<std::string, std::string> en_translations{{"moon", "Moon"}};
  std::unordered_map<std::string, std::string> es_translations{{"moon", "Luna"}};

  void set_locale(const std::string& locale_)
  {
    locale = locale_;
  }

  std::string translate(const char* key, std::size_t) const
  {
    if (locale == "en")
    {
      return en_translations.at(std::string(key));
    }
    else if (locale == "es")
    {
      return es_translations.at(std::string(key));
    }

    return std::string(key);
  }
};

TEST_CASE("i18n", "[core]")
{
  i18n::set_locale("");
  auto locale = i18n::locale();

  REQUIRE(locale == "");

  i18n::set_locale("es");
  locale = i18n::locale();

  REQUIRE(locale == "es");
}

TEST_CASE("custom translator", "[core]")
{
  using namespace i18n::literals;

  i18n::initialize_translator<translator>();

  i18n::set_locale("en");
  REQUIRE("moon"_t == "Moon");

  i18n::set_locale("es");
  REQUIRE("moon"_t == "Luna");
}

TEST_CASE("t(key, params): named and positional interpolation, no pluralization", "[core]")
{
  const i18n::translations translations{
      {"en", {{"greeting", "Hello {{name}}, item {{0}} of {{1}}"}}},
  };

  i18n::set_locale("en");
  i18n::initialize_translator(translations);

  REQUIRE(i18n::t("greeting", {{"name", "Alice"}, {"0", "3"}, {"1", "10"}}) == "Hello Alice, item 3 of 10");
}

TEST_CASE("t(key, count, params): pluralization combined with interpolation", "[core]")
{
  const i18n::translations translations{
      {"en",
       {
           {"unread_one", "{{name}} has {{count}} unread message"},
           {"unread_other", "{{name}} has {{count}} unread messages"},
       }},
  };

  i18n::set_locale("en");
  i18n::initialize_translator(translations);

  REQUIRE(i18n::t("unread", 1, {{"name", "Alice"}}) == "Alice has 1 unread message");
  REQUIRE(i18n::t("unread", 5, {{"name", "Alice"}}) == "Alice has 5 unread messages");
}

TEST_CASE("t(key, count): the full ar category set is reachable through the public API, not just select_plural_category", "[core]")
{
  const i18n::translations translations{
      {"ar",
       {
           {"dogs_zero", "zero"},
           {"dogs_one", "one"},
           {"dogs_two", "two"},
           {"dogs_few", "few"},
           {"dogs_many", "many"},
           {"dogs_other", "other"},
       }},
  };

  i18n::set_locale("ar");
  i18n::initialize_translator(translations);

  REQUIRE(i18n::t("dogs", 0) == "zero");
  REQUIRE(i18n::t("dogs", 1) == "one");
  REQUIRE(i18n::t("dogs", 2) == "two");
  REQUIRE(i18n::t("dogs", 5) == "few");
  REQUIRE(i18n::t("dogs", 15) == "many");
  REQUIRE(i18n::t("dogs", 100) == "other");
}

TEST_CASE("t(key, count): a locale outside the five with full CLDR rules falls back to one/other", "[core]")
{
  const i18n::translations translations{
      {"de",
       {
           {"dogs_one", "one dog"},
           {"dogs_other", "many dogs"},
       }},
  };

  i18n::set_locale("de");
  i18n::initialize_translator(translations);

  REQUIRE(i18n::t("dogs", 1) == "one dog");
  REQUIRE(i18n::t("dogs", 0) == "many dogs");
  REQUIRE(i18n::t("dogs", 5) == "many dogs");
}
