# i18n_keyval

![build status](https://github.com/stefandevai/i18n_keyval/actions/workflows/linux.yml/badge.svg)
![build status](https://github.com/stefandevai/i18n_keyval/actions/workflows/macos.yml/badge.svg)
![build status](https://github.com/stefandevai/i18n_keyval/actions/workflows/windows.yml/badge.svg)

Easy to use and customizable internationalization library. The core library has no external dependencies. You can, however, use a custom translator to provide translated strings using xml, json, lua or any other method you want using an external library. Custom translators for some known libraries are already provided.

# Usage
## Default translator

You need to provide a `i18n::translations` object containing all your locales with their respective localized strings.

**Example:**
```cpp
#include <i18n_keyval/i18n.hpp>
#include <i18n_keyval/translators/basic.hpp>

using namespace i18n::literals;

i18n::translations translations{
  {"en", {{"moon", "Moon"}, {"sun", "Sun"}}},
  {"es", {{"moon", "Luna"}, {"sun", "Sol"}}},
};

i18n::initialize_translator(translations);

i18n::set_locale("en");
std::cout << "moon"_t << '\n';
std::cout << "sun"_t << '\n';

i18n::set_locale("es");
std::cout << i18n::t("moon") << '\n';
std::cout << i18n::t("sun") << '\n';
```

**Output**:
```
Moon
Sun
Luna
Sol
```

## nlohmann/json translator

Built-in [nlohmann/json](https://github.com/nlohmann/json) translator. You need to set up the library by yourself.

**Example:**
```jsonc
// translations/en/translation.json
{
    "moon": "Moon",
    "animals": {
        "cat": "Cat"
    }
}
```
```jsonc
// translations/es/translation.json
{
    "moon": "Luna",
    "animals": {
        "cat": "Gato"
    }
}
```
```cpp
#include <i18n_keyval/i18n.hpp>
#include <i18n_keyval/translators/nlohmann_json.hpp>

using namespace i18n::literals;

i18n::initialize_translator<i18n::translators::nlohmann_json>();

// Or provide a custom directory where the translator can find the locales
// i18n::initialize_translator<i18n::translators::nlohmann_json>("data/translations");

i18n::set_locale("en");
std::cout << "moon"_t << '\n';
std::cout << "animals.cat"_t << '\n';

i18n::set_locale("es");
std::cout << i18n::t("moon") << '\n';
std::cout << i18n::t("animals.cat") << '\n';
```

**Output**:
```
Moon
Cat
Luna
Gato
```

## Pluralization & interpolation

`i18n::t(key, count)` picks a translation key based on `count`'s CLDR plural
category (i18next-style), and `{{placeholder}}` occurrences in the result are
substituted from a parameter list. Named and positional parameters are the
same mechanism -- a positional parameter is just a name that happens to be a
stringified index (`"0"`, `"1"`, ...) -- so both can appear in the same call
and the same translation string.

Every plural-aware key requires an explicit category suffix -- there is no
bare-key fallback. Full CLDR cardinal-plural rules are implemented for `ar`,
`en`, `es`, `fr`, and `it` (matched by language subtag, e.g. `"en-US"` uses
`en`'s rule); `ar` is the only one of these five with more than a `_one`/
`_other` split. Every other locale falls back to that same `_one`/`_other`
split.

**Example:**
```cpp
i18n::translations translations{
  {"en", {
    {"unread_one", "{{name}} has {{count}} unread message"},
    {"unread_other", "{{name}} has {{count}} unread messages"},
  }},
};

i18n::initialize_translator(translations);
i18n::set_locale("en");

std::cout << i18n::t("unread", 1, {{"name", "Alice"}}) << '\n';
std::cout << i18n::t("unread", 5, {{"name", "Alice"}}) << '\n';
```

**Output**:
```
Alice has 1 unread message
Alice has 5 unread messages
```

## Custom translator
To use a custom translator, you have to provide any class that implements a `set_locale` and `translate` methods. This allows you to retrieve strings with other methods such as xml, lua, other json library or a custom container.

**Example:**
```cpp
#include <i18n_keyval/i18n.hpp>

using namespace i18n::literals;

struct translator
{
  std::string locale{};
  std::unordered_map<std::string, std::string> en_translations{{ "moon", "Moon" }};
  std::unordered_map<std::string, std::string> es_translations{{ "moon", "Luna" }};

  void set_locale(const std::string& locale)
  {
    this->locale = locale;
  }

  std::string translate(const char* key, const std::size_t length) const
  {
    if (locale == "en")
    {
      return en_translations.at(std::string(key));
    }
    else if (locale == "es")
    {
      return es_translations.at(std::string(key));
    }

    return std::string(key);
  }
};

i18n::initialize_translator<translator>();

i18n::set_locale("en");
std::cout << "moon"_t << '\n';

i18n::set_locale("es");
std::cout << i18n::t("moon") << '\n';
```

**Output**:
```
Moon
Luna
```

# Features
- String literal usage
- Free function usage
- Nested json
- i18next-style pluralization (full CLDR for `ar`/`en`/`es`/`fr`/`it`, `_one`/`_other` fallback for other locales)
- `{{placeholder}}` interpolation, named and positional

# Translators
- Default (using `unordered_map`)
- nlohman/json (json)
- rapidjson (json)
- sol2 (lua)
- pugixml (xml)
- tinyxml2 (xml)
