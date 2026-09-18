#pragma once

#include <cstddef>
#include <string_view>

namespace i18n
{
enum class plural_category
{
  zero,
  one,
  two,
  few,
  many,
  other,
};

// Selects the CLDR cardinal-plural category for `count` in `locale`. Full
// CLDR rules are implemented for ar, en, es, fr, and it (matched by
// language subtag, e.g. "en-US" matches "en"'s rule); every other locale
// falls back to a plain one/other split (count == 1 -> one, else ->
// other), which is the same fallback en/es/it themselves use.
[[nodiscard]] plural_category select_plural_category(std::string_view locale, std::size_t count) noexcept;

// The translation-key suffix for a category, e.g. plural_category::one ->
// "_one". Every plural-aware key requires this suffix explicitly -- there
// is no bare-key alias for "one".
[[nodiscard]] std::string_view plural_category_suffix(plural_category category) noexcept;
}  // namespace i18n
