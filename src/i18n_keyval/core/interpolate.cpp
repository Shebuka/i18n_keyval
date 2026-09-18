#include "i18n_keyval/core/interpolate.hpp"

#include <algorithm>

namespace i18n
{
void interpolate(std::string& text, std::span<const interpolation_param> params) noexcept
{
  std::size_t search_from = 0;

  while (true)
  {
    const auto open = text.find("{{", search_from);

    if (open == std::string::npos)
    {
      return;
    }

    const auto close = text.find("}}", open + 2);

    if (close == std::string::npos)
    {
      // Unterminated "{{": nothing more to safely parse, leave the rest
      // of the string untouched rather than guessing.
      return;
    }

    const std::string_view placeholder{text.data() + open + 2, close - (open + 2)};

    const auto param =
        std::find_if(params.begin(), params.end(), [&](const auto& entry) { return entry.first == placeholder; });

    if (param == params.end())
    {
      // No matching value: leave this placeholder as-is and resume
      // scanning right after it, rather than looping on the same spot.
      search_from = close + 2;
      continue;
    }

    text.replace(open, (close + 2) - open, param->second);

    // Resume right after the inserted value, not from `open` again: if
    // the replacement value itself happened to contain "{{", re-scanning
    // it would either re-substitute it unexpectedly or, for a value that
    // referenced its own placeholder, loop forever.
    search_from = open + param->second.size();
  }
}
}  // namespace i18n
