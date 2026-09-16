#include <catch2/catch_test_macros.hpp>
#include <i18n_keyval/core/common.hpp>
#include <string>

namespace
{
// Regression test for F12: i18n_exception used to store the message as a
// borrowed `const char*`. Every real call site happens to pass a string
// literal, so that was harmless in practice, but the constructor is
// public and takes a std::string now specifically so this case is safe:
// build the exception from a local buffer, then mutate that buffer
// before returning. This only stays correct if the exception copied (or
// moved) the message into its own storage instead of referencing the
// original buffer.
i18n::i18n_exception make_exception_outliving_its_source()
{
  std::string source = "dynamic message: locale 'xx' not found";
  i18n::i18n_exception exception{source};
  source.assign(source.size(), '#');
  return exception;
}
}  // namespace

TEST_CASE("i18n_exception owns its message", "[core]")
{
  const auto exception = make_exception_outliving_its_source();

  REQUIRE(std::string(exception.what()) == "dynamic message: locale 'xx' not found");
}

TEST_CASE("i18n_exception is a catchable std::exception", "[core]")
{
  const i18n::i18n_exception exception{"boom"};
  const std::exception& base = exception;

  REQUIRE(std::string(base.what()) == "boom");
}

TEST_CASE("throw_i18n_exception throws a catchable i18n_exception", "[core]")
{
  bool caught = false;

  try
  {
    i18n::throw_i18n_exception("boom");
  }
  catch (const i18n::i18n_exception& e)
  {
    caught = true;
    REQUIRE(std::string(e.what()) == "boom");
  }

  REQUIRE(caught);
}
