#pragma once

#include <span>
#include <string>
#include <string_view>
#include <utility>

namespace i18n
{
// A single {{placeholder}} -> value substitution. `first` is looked up
// verbatim against whatever text appears between "{{" and "}}", so a
// named placeholder ("count", "name") and a positional one ("0", "1")
// are handled identically -- positional parameters are just named ones
// whose name happens to be a stringified index (see core/api.cpp).
using interpolation_param = std::pair<std::string_view, std::string_view>;

// Replaces every "{{placeholder}}" occurrence in `text` with the value
// of the first matching entry in `params`, in place. Every occurrence of
// a given placeholder is replaced, not just the first. A placeholder
// with no matching entry, or an unterminated "{{" with no matching "}}",
// is left untouched rather than being treated as an error.
void interpolate(std::string& text, std::span<const interpolation_param> params) noexcept;
}  // namespace i18n
