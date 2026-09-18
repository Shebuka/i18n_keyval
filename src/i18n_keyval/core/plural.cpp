#include "i18n_keyval/core/plural.hpp"

namespace i18n
{
namespace
{
// The part of a locale tag before the first '-' or '_', e.g. "en" from
// "en-US". CLDR plural rules are defined per language, not per region.
[[nodiscard]] std::string_view language_subtag(std::string_view locale) noexcept
{
  return locale.substr(0, locale.find_first_of("-_"));
}

[[nodiscard]] plural_category one_or_other(std::size_t count) noexcept
{
  return count == 1 ? plural_category::one : plural_category::other;
}

// fr: 0 and 1 both count as "one" -- distinct from en/es/it, where only 1
// does.
[[nodiscard]] plural_category french_category(std::size_t count) noexcept
{
  return (count == 0 || count == 1) ? plural_category::one : plural_category::other;
}

// ar: the only one of these five locales with more than two categories.
[[nodiscard]] plural_category arabic_category(std::size_t count) noexcept
{
  if (count == 0)
  {
    return plural_category::zero;
  }

  if (count == 1)
  {
    return plural_category::one;
  }

  if (count == 2)
  {
    return plural_category::two;
  }

  const auto mod100 = count % 100;

  if (mod100 >= 3 && mod100 <= 10)
  {
    return plural_category::few;
  }

  if (mod100 >= 11 && mod100 <= 99)
  {
    return plural_category::many;
  }

  return plural_category::other;
}
}  // namespace

plural_category select_plural_category(std::string_view locale, std::size_t count) noexcept
{
  const auto language = language_subtag(locale);

  if (language == "ar")
  {
    return arabic_category(count);
  }

  if (language == "fr")
  {
    return french_category(count);
  }

  // en, es, it, and every other/unrecognized locale all use the same
  // one/other split.
  return one_or_other(count);
}

std::string_view plural_category_suffix(plural_category category) noexcept
{
  switch (category)
  {
    case plural_category::zero:
      return "_zero";
    case plural_category::one:
      return "_one";
    case plural_category::two:
      return "_two";
    case plural_category::few:
      return "_few";
    case plural_category::many:
      return "_many";
    case plural_category::other:
      return "_other";
  }

  return "_other";
}
}  // namespace i18n
