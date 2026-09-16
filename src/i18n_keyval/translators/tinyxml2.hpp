#pragma once

#include <tinyxml2.h>

#include <filesystem>

#include "i18n_keyval/i18n.hpp"
#include "i18n_keyval/util/extension.hpp"
#include "i18n_keyval/util/locale.hpp"
#include "i18n_keyval/util/split_iterator.hpp"

namespace i18n::translators
{
class tinyxml2
{
 public:
  tinyxml2(std::filesystem::path directory_path_ = default_directory_name) : _directory_path(std::move(directory_path_))
  {
  }

  tinyxml2(const tinyxml2& rhs_)
  {
    _directory_path = rhs_._directory_path;
    rhs_._document.DeepCopy(&_document);
  }

  void set_locale(const std::string& locale_)
  {
    if (locale_.empty())
    {
      _document.Clear();
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
      _document.Clear();
      return;
    }

    const std::filesystem::path full_path = _directory_path / locale_ / (default_file_name + util::extension::xml);
    const auto& path_str = full_path.string();

    // A missing or unparseable locale file is not an error: fall back to
    // echoing the key untranslated, same as an empty/unset locale.
    auto res = _document.LoadFile(path_str.c_str());

    if (res != ::tinyxml2::XML_SUCCESS)
    {
      _document.Clear();
      return;
    }
  }

  // Builds a fresh instance instead of using the DeepCopy-based copy
  // constructor above: that constructor still duplicates the *currently*
  // loaded locale's data, which set_locale() is about to discard anyway.
  [[nodiscard]] tinyxml2 with_locale(const std::string& locale_) const
  {
    tinyxml2 copy{_directory_path};
    copy.set_locale(locale_);
    return copy;
  }

  std::string translate(const char* composed_key_, std::size_t length_) const noexcept
  {
    std::string_view view{composed_key_, length_};

    if (length_ == 0)
    {
      return std::string{view};
    }

    i18n::util::split_iterator<'/'> it{view};
    std::string_view key = *it;
    std::string key_str{key};
    ++it;

    const ::tinyxml2::XMLElement* root = _document.FirstChildElement(key_str.c_str());

    if (root == nullptr)
    {
      return std::string{view};
    }

    auto current_element = root->ToElement();

    for (; !(*it).empty(); ++it)
    {
      if (current_element == nullptr)
      {
        return std::string{view};
      }

      key = *it;
      key_str = std::string{key};
      auto next_element = current_element->FirstChildElement(key_str.c_str());

      if (next_element != nullptr)
      {
        current_element = next_element;
      }
      else
      {
        return std::string{view};
      }
    }

    if (current_element == nullptr || it.malformed())
    {
      return std::string{view};
    }

    // GetText() returns nullptr, not "", when the element has no direct
    // text child (e.g. an intermediate node like "animals" in
    // "animals/felines/cat", or an empty leaf element) -- constructing a
    // std::string from that would be undefined behavior.
    const char* text = current_element->GetText();

    if (text == nullptr)
    {
      return std::string{view};
    }

    return text;
  }

 private:
  std::filesystem::path _directory_path;
  ::tinyxml2::XMLDocument _document{};
};
}  // namespace i18n::translators
