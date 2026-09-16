#include "i18n_keyval/core/common.hpp"

#include <utility>

namespace i18n
{
i18n_exception::i18n_exception(std::string message_) : _message(std::move(message_)) {}

const char* i18n_exception::what() const noexcept
{
  return _message.c_str();
}

void throw_i18n_exception(std::string msg)
{
  I18N_THROW(i18n_exception(std::move(msg)));
}
}  // namespace i18n
