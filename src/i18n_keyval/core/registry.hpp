#pragma once

#include <memory>
#include <string>

#include "i18n_keyval/core/translator.hpp"

namespace i18n
{
class registry
{
 public:
  registry();
  registry(const registry&) = delete;
  registry& operator=(const registry&) = delete;

  void initialize_translator(std::shared_ptr<translator> translator_);
  [[nodiscard]] std::string translate(const char* composed_key_, const std::size_t length_) noexcept;
  void set_locale(std::string locale_);
  [[nodiscard]] std::string get_locale() const;
  [[nodiscard]] static registry& instance() noexcept;

 private:
  // Bundles the active locale with the translator instance that was built
  // for it. Once published, a snapshot is never mutated again -- a new
  // locale means building an entirely new snapshot (see translator::
  // with_locale) and swapping the pointer, never editing the one readers
  // might currently be using. That's what makes it safe to publish with
  // a single atomic pointer swap (via std::atomic_load/atomic_store)
  // instead of a lock: translate() always sees either the old snapshot
  // or the new one in full, never a partially-updated one.
  struct snapshot
  {
    std::string locale;
    std::shared_ptr<translator> translator_;
  };

  std::shared_ptr<const snapshot> _snapshot;
};
}  // namespace i18n
