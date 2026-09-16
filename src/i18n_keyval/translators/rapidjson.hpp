#pragma once

#include <filesystem>

#include "i18n_keyval/util/extension.hpp"
#include "i18n_keyval/util/file.hpp"
#include "i18n_keyval/util/locale.hpp"
#include "i18n_keyval/util/split_iterator.hpp"
#include "rapidjson/document.h"

namespace i18n::translators
{
class rapidjson
{
 public:
  rapidjson(std::filesystem::path directory_path_ = default_directory_name)
      : _directory_path(std::move(directory_path_))
  {
  }

  void set_locale(const std::string& locale_)
  {
    if (locale_.empty())
    {
      _document.SetObject();
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
      _document.SetObject();
      return;
    }

    const std::filesystem::path locale_directory = _directory_path / locale_;

    // An unsupported locale is not an error: fall back to echoing the key
    // untranslated, same as an empty/unset locale.
    if (!std::filesystem::exists(locale_directory) || !std::filesystem::is_directory(locale_directory))
    {
      _document.SetObject();
      return;
    }

    const std::filesystem::path full_path = locale_directory / (default_file_name + util::extension::json);

    // Parse(str, length) instead of Parse(str.c_str()): read_file() now
    // preserves embedded NUL bytes (F11), and Parse(const Ch*) would just
    // reintroduce the same truncation by treating the buffer as a
    // NUL-terminated C string.
    auto json_string = i18n::util::read_file(full_path);
    _document.Parse(json_string.c_str(), json_string.size());
  }

  std::string translate(const char* composed_key_, std::size_t length_) const noexcept
  {
    std::string_view view{composed_key_, length_};

    // FindMember() is only valid to call on an object value; rapidjson
    // guards this with RAPIDJSON_ASSERT, which is a plain assert() and
    // therefore compiled out under NDEBUG (i.e. every Release build this
    // project's own CMake produces). Without this check, a key like
    // "moon.cat" where "moon" is a string reads member-iterator/length
    // fields out of a union that actually holds string data -- verified
    // to segfault in Release, and to trip the assert (not a controlled
    // failure) otherwise.
    if (!_document.IsObject())
    {
      return std::string{view};
    }

    auto member_iterator = _document.MemberEnd();
    bool first_time = true;
    i18n::util::split_iterator it{view};

    for (; !(*it).empty(); ++it)
    {
      const auto key_str = std::string(*it);
      const auto& key = key_str.c_str();

      if (first_time)
      {
        member_iterator = _document.FindMember(key);
        first_time = false;

        if (member_iterator == _document.MemberEnd())
        {
          return std::string{view};
        }
      }
      else
      {
        if (!member_iterator->value.IsObject())
        {
          return std::string{view};
        }

        auto old_member_iterator = member_iterator;
        member_iterator = member_iterator->value.FindMember(key);

        if (member_iterator == old_member_iterator->value.MemberEnd())
        {
          return std::string{view};
        }
      }
    }

    if (it.malformed() || member_iterator == _document.MemberEnd() || member_iterator->value.IsNull() ||
        !member_iterator->value.IsString())
    {
      return std::string{view};
    }

    return member_iterator->value.GetString();
  }

 private:
  ::rapidjson::Document _document;
  std::filesystem::path _directory_path;
};
}  // namespace i18n::translators
