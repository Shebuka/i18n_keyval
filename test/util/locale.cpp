#include <catch2/catch_test_macros.hpp>
#include <i18n_keyval/util/locale.hpp>

// Regression tests for F4: every file-based translator builds a
// filesystem path as `directory / locale / file`, and an unvalidated
// locale could previously escape that directory via an absolute path or
// "..", since std::filesystem::path::operator/ neither rejects nor
// normalizes either of those.
TEST_CASE("is_valid_locale_name", "[core]")
{
  SECTION("accepts simple identifiers")
  {
    REQUIRE(i18n::util::is_valid_locale_name("en"));
    REQUIRE(i18n::util::is_valid_locale_name("en-US"));
    REQUIRE(i18n::util::is_valid_locale_name("pt_BR"));
    REQUIRE(i18n::util::is_valid_locale_name("en123"));
  }

  SECTION("rejects an empty locale")
  {
    REQUIRE_FALSE(i18n::util::is_valid_locale_name(""));
  }

  SECTION("rejects path traversal")
  {
    REQUIRE_FALSE(i18n::util::is_valid_locale_name(".."));
    REQUIRE_FALSE(i18n::util::is_valid_locale_name("../other"));
    REQUIRE_FALSE(i18n::util::is_valid_locale_name("en/../../etc"));
  }

  SECTION("rejects absolute paths")
  {
    REQUIRE_FALSE(i18n::util::is_valid_locale_name("/etc/passwd"));
    REQUIRE_FALSE(i18n::util::is_valid_locale_name("/tmp/evil"));
  }

  SECTION("rejects embedded path separators")
  {
    REQUIRE_FALSE(i18n::util::is_valid_locale_name("en/us"));
    REQUIRE_FALSE(i18n::util::is_valid_locale_name("en\\us"));
  }
}
