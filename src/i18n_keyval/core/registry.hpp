#pragma once

#include <memory>
#include <mutex>
#include <string>

#include "i18n_keyval/core/translator.hpp"

namespace i18n
{
class registry
{
 public:
  registry() = default;
  registry(const registry&) = delete;
  registry& operator=(const registry&) = delete;

  void initialize_translator(std::shared_ptr<translator> translator_);
  [[nodiscard]] std::string translate(const char* composed_key_, const std::size_t length_) noexcept;
  void set_locale(std::string locale_);
  [[nodiscard]] std::string get_locale() const;
  [[nodiscard]] static registry& instance() noexcept;

 private:
  std::string _locale{};
  std::shared_ptr<translator> _translator = nullptr;
  mutable std::mutex _mutex;
};
}  // namespace i18n
