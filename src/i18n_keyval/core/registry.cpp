#include "i18n_keyval/core/registry.hpp"

namespace i18n
{
void registry::initialize_translator(std::shared_ptr<translator> translator_)
{
  std::scoped_lock lock{_mutex};
  _translator = std::move(translator_);
  _translator->set_locale(_locale);
}

std::string registry::translate(const char* composed_key_, const std::size_t length_) noexcept
{
  // Held for the whole call, not just to copy the shared_ptr: a
  // translator's set_locale() mutates its own internal state (e.g. the
  // parsed document), which translate() reads. Releasing the lock before
  // calling into the translator would still let a concurrent
  // set_locale() race with a translate() already in flight on the same
  // translator instance, only one level down from where registry's own
  // fields are protected.
  std::scoped_lock lock{_mutex};

  if (_translator == nullptr)
  {
    return std::string{composed_key_, length_};
  }

  return _translator->translate(composed_key_, length_);
}

void registry::set_locale(std::string locale_)
{
  std::scoped_lock lock{_mutex};

  if (_translator != nullptr)
  {
    // Update the translator first: if it throws, `_locale` is left
    // unmodified instead of pointing at a locale that was never loaded.
    _translator->set_locale(locale_);
  }

  _locale = std::move(locale_);
}

std::string registry::get_locale() const
{
  std::scoped_lock lock{_mutex};
  return _locale;
}

registry& registry::instance() noexcept
{
  static registry _instance;
  return _instance;
}
}  // namespace i18n
