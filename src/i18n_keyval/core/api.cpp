#include "i18n_keyval/core/api.hpp"

#include <string>
#include <vector>

#include "i18n_keyval/core/plural.hpp"

namespace
{
[[nodiscard]] std::string pluralize_key(std::string_view key_, std::size_t count_)
{
  const auto category = i18n::select_plural_category(i18n::registry::instance().get_locale(), count_);
  return std::string(key_) += i18n::plural_category_suffix(category);
}

[[nodiscard]] std::string translate_with_params(std::string_view key_,
                                                 std::span<const i18n::interpolation_param> params_)
{
  auto translation = i18n::registry::instance().translate(key_.data(), key_.size());
  i18n::interpolate(translation, params_);
  return translation;
}
}  // namespace

namespace i18n
{
std::string t(std::string_view key_) noexcept
{
  return registry::instance().translate(key_.data(), key_.size());
}

std::string t(std::string_view key_, std::size_t count_) noexcept
{
  const auto plural_key = pluralize_key(key_, count_);
  const auto count_str = std::to_string(count_);

  return translate_with_params(plural_key, std::vector<interpolation_param>{{"count", count_str}});
}

std::string t(std::string_view key_, std::initializer_list<interpolation_param> params_) noexcept
{
  return translate_with_params(key_, params_);
}

std::string t(std::string_view key_, std::size_t count_, std::initializer_list<interpolation_param> params_) noexcept
{
  const auto plural_key = pluralize_key(key_, count_);
  const auto count_str = std::to_string(count_);

  std::vector<interpolation_param> all_params;
  all_params.reserve(params_.size() + 1);
  all_params.emplace_back("count", count_str);
  all_params.insert(all_params.end(), params_.begin(), params_.end());

  return translate_with_params(plural_key, all_params);
}

void set_locale(std::string locale_)
{
  registry::instance().set_locale(std::move(locale_));
}

std::string locale() noexcept
{
  return registry::instance().get_locale();
}
}  // namespace i18n
