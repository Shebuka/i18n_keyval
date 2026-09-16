#pragma once

#include <filesystem>
#include <pugixml.hpp>

#include "i18n_keyval/util/extension.hpp"
#include "i18n_keyval/util/locale.hpp"
#include "i18n_keyval/util/split_iterator.hpp"

namespace i18n::translators
{
class pugixml
{
 public:
  pugixml(std::filesystem::path directory_path_ = default_directory_name) : _directory_path(std::move(directory_path_))
  {
  }

  void set_locale(const std::string& locale_)
  {
    if (locale_.empty())
    {
      _document.load_file("");
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
      _document.load_file("");
      return;
    }

    const std::filesystem::path full_path = _directory_path / locale_ / (default_file_name + util::extension::xml);

    // An unsupported locale is not an error: fall back to echoing the key
    // untranslated, same as an empty/unset locale.
    if (!std::filesystem::exists(full_path))
    {
      _document.load_file("");
      return;
    }

    _document.load_file(full_path.c_str());
  }

  // Builds a fresh instance instead of copying *this: pugi::xml_document
  // deliberately disables copying (it owns its own memory pool), so a
  // fresh instance loaded from the same _directory_path is the only way
  // to produce an independent snapshot for this translator.
  [[nodiscard]] pugixml with_locale(const std::string& locale_) const
  {
    pugixml copy{_directory_path};
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

    // Walk the key one '/'-separated segment at a time as a plain child
    // element name (mirroring translators::tinyxml2), instead of handing
    // it to select_nodes as an XPath expression: a key is only ever
    // supposed to be a path like "colors/black", but XPath accepts any
    // expression, which previously let a key select or exfiltrate nodes
    // outside the intended path (e.g. "//*[1]") and, incidentally, made
    // the parser throw on malformed input (F2).
    pugi::xml_node current_node = _document;
    i18n::util::split_iterator<'/'> it{view};

    for (; !(*it).empty(); ++it)
    {
      const std::string key{*it};
      current_node = current_node.child(key.c_str());

      if (!current_node)
      {
        return std::string{view};
      }
    }

    if (it.malformed())
    {
      return std::string{view};
    }

    return current_node.child_value();
  }

 private:
  pugi::xml_document _document;
  std::filesystem::path _directory_path;
};
}  // namespace i18n::translators
