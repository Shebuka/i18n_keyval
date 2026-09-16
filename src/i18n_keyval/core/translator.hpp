#pragma once

#include <memory>
#include <string>
#include <type_traits>

namespace i18n
{
namespace detail
{
// Detects whether T implements with_locale(const std::string&) const -> T
// itself. Built-in translators do (see translators/*.hpp) because several
// of them wrap a type that can't be copied (sol::state, rapidjson::
// Document, pugi::xml_document), so "construct fresh from config" is the
// only way to produce an independent snapshot. A custom translator
// written against the older two-method contract (set_locale + translate
// only, as in a plain struct) won't have it, and shouldn't be required
// to: model<T> below falls back to copying such a T instead.
template <typename T, typename = void>
struct has_with_locale : std::false_type
{
};

template <typename T>
struct has_with_locale<T, std::void_t<decltype(std::declval<const T&>().with_locale(std::declval<const std::string&>()))>>
    : std::true_type
{
};
}  // namespace detail

class translator
{
 public:
  template <typename T>
  translator(T&& translator_) : _translator(std::make_shared<model<T>>(std::forward<T>(translator_)))
  {
  }

  void set_locale(const std::string& locale_)
  {
    _translator->set_locale(locale_);
  }

  // Returns a brand new translator wrapping a fresh, independent instance
  // of the same underlying type, already configured for `locale_`. Unlike
  // set_locale() above (which mutates the wrapped object's own internal
  // state in place), this never touches `this`, so a caller already
  // holding a shared_ptr to this translator instance -- e.g. one in the
  // middle of translate() on another thread -- is completely unaffected.
  // This is what makes it safe to publish via a single atomic pointer
  // swap instead of a lock (see registry).
  [[nodiscard]] std::shared_ptr<translator> with_locale(const std::string& locale_) const
  {
    return std::shared_ptr<translator>(new translator(_translator->with_locale(locale_)));
  }

  std::string translate(const char* composed_key_, const std::size_t length_) const
  {
    return _translator->translate(composed_key_, length_);
  }

  struct base
  {
    virtual ~base() = default;
    virtual void set_locale(const std::string& locale_) = 0;
    [[nodiscard]] virtual std::shared_ptr<base> with_locale(const std::string& locale_) const = 0;
    virtual std::string translate(const char* composed_key_, const std::size_t length_) const = 0;
  };

  template <typename T>
  struct model final : base
  {
    model(T object_) : _object(std::move(object_)) {}

    void set_locale(const std::string& locale_) override
    {
      _object.set_locale(locale_);
    }

    [[nodiscard]] std::shared_ptr<base> with_locale(const std::string& locale_) const override
    {
      if constexpr (detail::has_with_locale<T>::value)
      {
        return std::make_shared<model<T>>(_object.with_locale(locale_));
      }
      else
      {
        static_assert(std::is_copy_constructible_v<T>,
                       "A custom translator must be copyable, or implement "
                       "with_locale(const std::string&) const -> T itself, to support "
                       "concurrent set_locale()/translate() calls.");
        T copy{_object};
        copy.set_locale(locale_);
        return std::make_shared<model<T>>(std::move(copy));
      }
    }

    std::string translate(const char* composed_key_, const std::size_t length_) const override
    {
      return _object.translate(composed_key_, length_);
    }

   private:
    T _object;
  };

 private:
  explicit translator(std::shared_ptr<base> translator_) : _translator(std::move(translator_)) {}

  std::shared_ptr<base> _translator;
};
}  // namespace i18n
