#include "i18n_keyval/util/file.hpp"

#include <fstream>

namespace i18n::util
{
namespace
{
// Translation files are small, hand-authored data files; there is no
// legitimate reason for one to be huge. Capping the size bounds how much
// memory a single set_locale() call can be made to allocate.
constexpr std::streamoff max_file_size = 64 * 1024 * 1024;
}  // namespace

std::string read_file(const std::filesystem::path filepath)
{
  // Binary mode: "rt" text mode (the original mode) translates CRLF and,
  // more importantly, std::string{data} previously stopped at the first
  // embedded NUL byte, silently truncating any file that isn't pure text.
  std::ifstream file(filepath, std::ios::binary);

  if (!file)
  {
    return "";
  }

  file.seekg(0, std::ios::end);
  const std::streamoff length = file.tellg();

  // tellg() reports failure as -1 (it wraps ftell(), which does the same);
  // the previous implementation stored this in an *unsigned* long, so -1
  // became ULONG_MAX, `length + 1` wrapped back around to 0, and
  // `fread(data, 1, ULONG_MAX, file)` was then called against a
  // zero-byte allocation. Rejecting a negative or oversized length here
  // avoids both that overflow and an unbounded allocation from a huge file.
  if (length < 0 || length > max_file_size)
  {
    return "";
  }

  file.seekg(0, std::ios::beg);

  std::string result(static_cast<std::size_t>(length), '\0');
  file.read(result.data(), length);

  // The previous implementation never checked fread()'s return value.
  if (!file)
  {
    return "";
  }

  return result;
}
}  // namespace i18n::util
