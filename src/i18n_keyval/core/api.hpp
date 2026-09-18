#pragma once

#include <filesystem>
#include <initializer_list>
#include <string>

#include "i18n_keyval/core/interpolate.hpp"
#include "i18n_keyval/core/registry.hpp"
#include "i18n_keyval/translators/basic.hpp"

namespace i18n
{
const std::filesystem::path default_directory_name{"translations"};

const std::string default_file_name = "translation";

[[nodiscard]] std::string t(std::string_view key_) noexcept;

// count-based pluralization: resolves to key_<category> (e.g. "_one",
// "_other", or one of ar's fuller set -- see core/plural.hpp) and
// auto-injects {{count}} as an interpolation parameter.
[[nodiscard]] std::string t(std::string_view key_, std::size_t count_) noexcept;

// Named and/or positional interpolation, no pluralization. A parameter's
// name can be anything a translation string references as {{name}},
// including a stringified index ("0", "1", ...) for positional-style
// use -- both are looked up the same way.
[[nodiscard]] std::string t(std::string_view key_, std::initializer_list<interpolation_param> params_) noexcept;

// Pluralization combined with interpolation. {{count}} is auto-injected
// same as the count-only overload above; `params_` is applied on top of
// it.
[[nodiscard]] std::string t(std::string_view key_, std::size_t count_,
                            std::initializer_list<interpolation_param> params_) noexcept;

template <typename T = translators::basic, typename... Args>
void initialize_translator(Args&&... args_)
{
  auto translator_ = std::make_shared<translator>(T{std::forward<Args>(args_)...});
  registry::instance().initialize_translator(std::move(translator_));
}

std::string locale() noexcept;

void set_locale(std::string locale_);
}  // namespace i18n

namespace i18n::literals
{
[[nodiscard]] inline std::string operator""_t(const char* nested_key_, std::size_t length_) noexcept
{
  return i18n::registry::instance().translate(nested_key_, length_);
}
}  // namespace i18n::literals
