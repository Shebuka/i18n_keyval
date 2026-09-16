#pragma once

#include <cstdio>
#include <exception>
#include <string>

#ifndef I18N_EXCEPTIONS
#define I18N_TRY
#define I18N_CATCH
#define I18N_THROW(exception) printf("i18n error: %s\n", exception.what());
#else
#define I18N_TRY try
#define I18N_CATCH \
  catch (const std::exception&) {}
#define I18N_THROW(exception) throw(exception)
#endif

namespace i18n
{
class i18n_exception : public std::exception
{
 public:
  explicit i18n_exception(std::string message_);
  [[nodiscard]] const char* what() const noexcept override;

 private:
  // Owns the message rather than storing a borrowed const char*: every
  // call site here passes a string literal (so it was harmless in
  // practice), but the constructor is public, and throw_i18n_exception
  // takes a std::string, so any caller building the message dynamically
  // (e.g. throw_i18n_exception(msg.c_str()) with `msg` a local
  // std::string, or by constructing i18n_exception directly) would leave
  // what() reading a dangling pointer once the temporary is destroyed.
  std::string _message;
};

void throw_i18n_exception(std::string msg);
}  // namespace i18n
