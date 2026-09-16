#pragma once

#include <filesystem>
#include <sol/sol.hpp>

#include "i18n_keyval/util/extension.hpp"
#include "i18n_keyval/util/file.hpp"
#include "i18n_keyval/util/locale.hpp"
#include "i18n_keyval/util/split_iterator.hpp"

namespace i18n::translators
{
class sol2
{
 public:
  sol2(std::filesystem::path directory_path_ = default_directory_name) : _directory_path(std::move(directory_path_)) {}

  void set_locale(const std::string& locale_)
  {
    if (locale_.empty())
    {
      _lua["translations"] = sol::lua_nil;
      return;
    }

    // Reject anything that isn't a simple identifier before it ever
    // reaches std::filesystem::path::operator/: an absolute path replaces
    // the whole directory, and ".." isn't normalized away, so a locale
    // string could otherwise be used to execute a Lua file from outside
    // the translations directory. Treated the same as "locale not found"
    // (fall back, don't throw): a locale is frequently derived from
    // untrusted input (user preference, Accept-Language), and an
    // uncaught exception here would be as much of a DoS as F1's.
    if (!util::is_valid_locale_name(locale_))
    {
      _lua["translations"] = sol::lua_nil;
      return;
    }

    const std::filesystem::path locale_directory = _directory_path / locale_;

    // An unsupported locale is not an error: fall back to echoing the key
    // untranslated, same as an empty/unset locale.
    if (!std::filesystem::exists(locale_directory) || !std::filesystem::is_directory(locale_directory))
    {
      _lua["translations"] = sol::lua_nil;
      return;
    }

    const std::filesystem::path full_path = locale_directory / (default_file_name + util::extension::lua);

    auto lua_string = i18n::util::read_file(full_path);
    _lua.script(lua_string);
  }

  std::string translate(const char* composed_key_, std::size_t length_) const noexcept
  {
    std::string_view view{composed_key_, length_};
    i18n::util::split_iterator it{view};
    auto current_object = _lua.get<sol::table>("translations");

    if (!current_object.valid())
    {
      return std::string{view};
    }

    for (; !(*it).empty(); ++it)
    {
      const std::string_view key = *it;
      const auto& value = current_object.get<sol::object>(key);

      if (value.valid() && value.is<sol::table>())
      {
        current_object = current_object.get<sol::table>(key);
      }
      else if (value.valid() && value.is<std::string>())
      {
        if (it.malformed())
        {
          return std::string{view};
        }

        return value.as<std::string>();
      }
      else
      {
        return std::string{view};
      }
    }

    return std::string{view};
  }

 private:
  sol::state _lua;
  std::filesystem::path _directory_path;
};
}  // namespace i18n::translators
