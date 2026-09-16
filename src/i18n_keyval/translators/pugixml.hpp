#pragma once

#include <filesystem>
#include <pugixml.hpp>

#include "i18n_keyval/util/extension.hpp"
#include "i18n_keyval/util/locale.hpp"

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

  std::string translate(const char* composed_key_, std::size_t length_) const noexcept
  {
    if (length_ == 0)
    {
      return std::string{composed_key_, length_};
    }

    // The key is evaluated as an XPath expression, and an arbitrary or
    // malformed one (as little as a single stray character) makes
    // select_nodes throw pugi::xpath_exception. This function is noexcept,
    // so letting that escape would call std::terminate over a single bad
    // key; treat it the same as "not found" instead.
    try
    {
      auto nodes = _document.select_nodes(composed_key_);

      if (nodes.empty())
      {
        return std::string{composed_key_, length_};
      }

      return nodes[0].node().first_child().value();
    }
    catch (const pugi::xpath_exception&)
    {
      return std::string{composed_key_, length_};
    }
  }

 private:
  pugi::xml_document _document;
  std::filesystem::path _directory_path;
};
}  // namespace i18n::translators
