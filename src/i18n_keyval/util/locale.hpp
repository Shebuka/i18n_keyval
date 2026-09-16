#pragma once

#include <string>

namespace i18n::util
{
// Locale names are used to build a filesystem path
// (`directory_path / locale / file`). std::filesystem::path::operator/
// treats an absolute operand as replacing the whole path, and never
// normalizes or rejects "..", so an unvalidated locale string can escape
// the translations directory entirely. Restricting locale names to a
// simple identifier (letters, digits, '-', '_') prevents that.
[[nodiscard]] bool is_valid_locale_name(const std::string& locale_) noexcept;
}  // namespace i18n::util
