#include "i18n_keyval/util/locale.hpp"

#include <cctype>

namespace i18n::util
{
bool is_valid_locale_name(const std::string& locale_) noexcept
{
  if (locale_.empty())
  {
    return false;
  }

  for (const char c : locale_)
  {
    if (std::isalnum(static_cast<unsigned char>(c)) == 0 && c != '-' && c != '_')
    {
      return false;
    }
  }

  return true;
}
}  // namespace i18n::util
