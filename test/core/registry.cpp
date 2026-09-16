#include <atomic>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <i18n_keyval/core/registry.hpp>
#include <i18n_keyval/i18n.hpp>
#include <i18n_keyval/translators/basic.hpp>
#include <thread>
#include <vector>

TEST_CASE("registry", "[core]")
{
  i18n::registry::instance().set_locale("");
  auto locale = i18n::registry::instance().get_locale();

  REQUIRE(locale == "");

  i18n::registry::instance().set_locale("es");
  locale = i18n::registry::instance().get_locale();

  REQUIRE(locale == "es");
}

TEST_CASE("registry withstands concurrent set_locale/translate/get_locale", "[core]")
{
  // Regression test for F10: registry used to read and write its locale
  // and translator with no synchronization at all. This doesn't replace
  // ThreadSanitizer, which is how the fix was actually verified during
  // development (concurrent set_locale()/translate() reliably reported a
  // real data race before the fix, clean after) -- but it does exercise
  // the same entry points concurrently and checks that translate() and
  // get_locale() never observe anything but a value one of the writer
  // threads actually set, i.e. never a torn or partially-updated one.
  const i18n::translations translations{
      {"en", {{"moon", "Moon"}}},
      {"es", {{"moon", "Luna"}}},
  };

  i18n::initialize_translator(translations);

  std::atomic<bool> stop{false};
  std::atomic<int> unexpected{0};
  std::vector<std::thread> threads;

  for (int i = 0; i < 4; ++i)
  {
    threads.emplace_back(
        [&, i]
        {
          while (!stop.load(std::memory_order_relaxed))
          {
            i18n::registry::instance().set_locale(i % 2 == 0 ? "en" : "es");
          }
        });
  }

  for (int i = 0; i < 4; ++i)
  {
    threads.emplace_back(
        [&]
        {
          while (!stop.load(std::memory_order_relaxed))
          {
            const auto value = i18n::registry::instance().translate("moon", 4);
            const auto locale = i18n::registry::instance().get_locale();

            if (value != "Moon" && value != "Luna")
            {
              unexpected.fetch_add(1, std::memory_order_relaxed);
            }

            if (locale != "en" && locale != "es")
            {
              unexpected.fetch_add(1, std::memory_order_relaxed);
            }
          }
        });
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  stop.store(true, std::memory_order_relaxed);

  for (auto& thread : threads)
  {
    thread.join();
  }

  REQUIRE(unexpected.load() == 0);
}
