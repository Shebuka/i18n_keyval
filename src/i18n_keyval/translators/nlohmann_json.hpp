#pragma once

#include <filesystem>
#include <nlohmann/json.hpp>

#include "i18n_keyval/util/extension.hpp"
#include "i18n_keyval/util/file.hpp"
#include "i18n_keyval/util/locale.hpp"
#include "i18n_keyval/util/split.hpp"

namespace i18n::translators
{
class nlohmann_json
{
 public:
  nlohmann_json(std::filesystem::path directory_path_ = default_directory_name)
      : _directory_path(std::move(directory_path_))
  {
  }

  void set_locale(const std::string& locale_)
  {
    if (locale_.empty())
    {
      _object = nlohmann::json{};
      return;
    }

    // Reject anything that isn't a simple identifier before it ever
    // reaches std::filesystem::path::operator/: an absolute path replaces
    // the whole directory, and ".." isn't normalized away, so a locale
    // string could otherwise be used to read a file outside the
    // translations directory. Treated the same as "locale not found"
    // (fall back, don't throw): a locale is frequently derived from
    // untrusted input (user preference, Accept-Language), and an
    // uncaught exception here would be as much of a DoS as F1's.
    if (!util::is_valid_locale_name(locale_))
    {
      _object = nlohmann::json{};
      return;
    }

    const std::filesystem::path locale_directory = _directory_path / locale_;

    // An unsupported locale is not an error: fall back to echoing the key
    // untranslated, same as an empty/unset locale.
    if (!std::filesystem::exists(locale_directory) || !std::filesystem::is_directory(locale_directory))
    {
      _object = nlohmann::json{};
      return;
    }

    const std::filesystem::path full_path = locale_directory / (default_file_name + util::extension::json);

    auto json_string = i18n::util::read_file(full_path);
    _object = nlohmann::json::parse(std::move(json_string));
  }

  // Builds a fresh instance instead of copying *this: nlohmann::json is
  // copyable, but there is no reason to pay for a deep copy of the
  // *currently* loaded locale's data only to immediately replace it.
  [[nodiscard]] nlohmann_json with_locale(const std::string& locale_) const
  {
    nlohmann_json copy{_directory_path};
    copy.set_locale(locale_);
    return copy;
  }

  std::string translate(const char* composed_key_, std::size_t length_) const noexcept
  {
    std::string_view view{composed_key_, length_};
    auto* current_object = &_object;

    for (std::string_view key : i18n::util::split(view, '.'))
    {
      // An empty segment (leading/trailing/duplicated '.') makes the key
      // malformed; treat it as not found rather than looking it up.
      if (key.empty() || !current_object->contains(key))
      {
        return std::string{view};
      }

      current_object = &((*current_object)[key]);
    }

    if (current_object->is_null() || !current_object->is_string())
    {
      return std::string{view};
    }

    return current_object->get<std::string>();
  }

 private:
  nlohmann::json _object;
  std::filesystem::path _directory_path;
};
}  // namespace i18n::translators
