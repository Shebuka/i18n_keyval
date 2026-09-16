#include "i18n_keyval/core/registry.hpp"

namespace i18n
{
// Ideally this would use std::atomic<std::shared_ptr<const snapshot>>
// (C++20). That class template is standard-mandated, but its actual
// availability lags behind compiler/language support: as of this writing
// it isn't implemented by libc++ on Xcode's command line tools (it falls
// back to the primary std::atomic<T> template, which then fails to
// compile because shared_ptr isn't trivially copyable). The pre-C++20
// std::atomic_load/atomic_store free-function overloads for shared_ptr
// provide the same atomicity guarantee and are available since C++11 --
// used here instead, on every access to `_snapshot` without exception,
// since mixing atomic and non-atomic access to the same shared_ptr is a
// data race in itself.
registry::registry() : _snapshot(std::make_shared<const snapshot>()) {}

void registry::initialize_translator(std::shared_ptr<translator> translator_)
{
  const auto current = std::atomic_load(&_snapshot);

  // Safe to mutate in place: translator_ was just constructed by the
  // caller and isn't shared with (or reachable from) anything else yet.
  translator_->set_locale(current->locale);

  std::atomic_store(&_snapshot,
                     std::make_shared<const snapshot>(snapshot{current->locale, std::move(translator_)}));
}

std::string registry::translate(const char* composed_key_, const std::size_t length_) noexcept
{
  // The only synchronization translate() needs is this one atomic load:
  // the snapshot it gets is immutable from here on, so it can read from
  // it -- and other threads can concurrently run translate() on the same
  // or a different snapshot -- without any further locking at all.
  const auto current = std::atomic_load(&_snapshot);

  if (current->translator_ == nullptr)
  {
    return std::string{composed_key_, length_};
  }

  return current->translator_->translate(composed_key_, length_);
}

void registry::set_locale(std::string locale_)
{
  const auto current = std::atomic_load(&_snapshot);

  if (current->translator_ == nullptr)
  {
    std::atomic_store(&_snapshot, std::make_shared<const snapshot>(snapshot{std::move(locale_), nullptr}));
    return;
  }

  // Builds an entirely new, independent translator instance (see
  // translator::with_locale) instead of mutating the published one in
  // place. If this throws, `current` -- still published and possibly in
  // use by another thread's translate() call right now -- is completely
  // untouched: a stronger exception guarantee than just "the locale
  // string is left alone", since here the translator's own state can
  // never be observed half-updated either.
  //
  // Note: two set_locale() calls racing each other (or racing
  // initialize_translator()) is a last-write-wins, not a merge -- there
  // is no compare-and-swap retry loop here. That's an acceptable
  // trade-off, not a safety issue: which one should "win" is already an
  // application-level ambiguity, not something either implementation can
  // resolve, and this path is not the hot one translate() is optimized
  // for.
  auto new_translator = current->translator_->with_locale(locale_);
  std::atomic_store(
      &_snapshot, std::make_shared<const snapshot>(snapshot{std::move(locale_), std::move(new_translator)}));
}

std::string registry::get_locale() const
{
  return std::atomic_load(&_snapshot)->locale;
}

registry& registry::instance() noexcept
{
  static registry _instance;
  return _instance;
}
}  // namespace i18n
