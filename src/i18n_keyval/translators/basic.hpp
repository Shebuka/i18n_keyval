#pragma once

#include <string>
#include <unordered_map>

namespace i18n
{
using translation_table = std::unordered_map<std::string, std::string>;
using translations = std::unordered_map<std::string, translation_table>;
}  // namespace i18n

namespace i18n::translators
{
class basic
{
 public:
  basic(translations translations_) : _translations(std::move(translations_)) {}

  void set_locale(const std::string& locale_)
  {
    // A locale with no matching table (including an empty/unset locale) is
    // not an error: translate() falls back to echoing the key untranslated.
    if (_translations.find(locale_) == _translations.end())
    {
      _values = nullptr;
      return;
    }

    _values = &_translations.at(locale_);
  }

  std::string translate(const char* key_, std::size_t length_) const noexcept
  {
    std::string_view view{key_, length_};

    if (_values == nullptr)
    {
      return std::string{view};
    }

    const auto& values = *_values;

    // unordered_map::find/at(const char*) implicitly build a std::string
    // via strlen(key_), ignoring length_ entirely. The API contract here
    // is (pointer, length) precisely because the caller (e.g. t() on a
    // std::string_view) may pass a buffer that isn't NUL-terminated right
    // at length_ -- strlen would then read past the end of it. Building
    // the key explicitly from the (pointer, length) pair respects the
    // bound the caller actually gave us.
    const auto it = values.find(std::string{view});

    if (it == values.end())
    {
      return std::string{view};
    }

    return it->second;
  }

 private:
  translations _translations;
  translation_table* _values = nullptr;
};
}  // namespace i18n::translators
