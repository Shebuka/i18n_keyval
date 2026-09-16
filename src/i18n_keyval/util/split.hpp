#pragma once

#include <ranges>
#include <string_view>

namespace i18n::util
{
// Splits `value` on `delimiter`, yielding a range of std::string_view
// segments (built directly on std::views::split, no custom iterator).
// Unlike a hand-rolled splitter, this can't conflate "no more segments"
// with "an empty segment" the way util::split_iterator historically did
// (see F13): every element yielded here is a real segment produced by
// the standard library, including empty ones for a leading, trailing, or
// duplicated delimiter (e.g. "a." -> {"a", ""}, "a..b" -> {"a", "", "b"},
// ".a" -> {"", "a"}). Callers that need to reject a malformed key can
// just check whether any yielded segment is empty.
[[nodiscard]] inline auto split(std::string_view value, char delimiter)
{
  return value | std::views::split(delimiter) |
         std::views::transform([](auto&& segment) { return std::string_view(segment.begin(), segment.end()); });
}
}  // namespace i18n::util
