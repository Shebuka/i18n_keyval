#pragma once

#include <cstdint>
#include <iterator>
#include <string>

namespace i18n::util
{
template <char delimeter = '.'>
class split_iterator
{
 public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = char;
  using pointer = const char*;
  using reference = const char&;

  split_iterator(std::string_view value_) : _value(value_)
  {
    operator++();
  }

  split_iterator(const split_iterator& other) = default;
  split_iterator& operator=(const split_iterator& rhs) = default;

  std::string_view operator*() const
  {
    return _value.substr(_from, _length);
  }

  // True once a segment produced by the iterator was empty because of a
  // leading, trailing, or duplicated delimiter (e.g. "a..b", "a.", ".a").
  // Such a key does not name a well-formed path and callers should treat it
  // as "not found" rather than trusting whatever partial path was walked.
  [[nodiscard]] bool malformed() const
  {
    return _malformed;
  }

  split_iterator& operator++()
  {
    if (_index >= _value.size())
    {
      _from = 0;
      _length = 0;
      return *this;
    }

    const auto initial_index = _index;

    for (; _index < _value.size(); ++_index)
    {
      if (_value[_index] == delimeter)
      {
        _from = initial_index;
        _length = _index - initial_index;
        ++_index;

        if (_length == 0 || _index == _value.size())
        {
          _malformed = true;
        }

        return *this;
      }
    }

    _from = initial_index;
    _length = _index - initial_index;
    return *this;
  }

  split_iterator operator++(int)
  {
    split_iterator temp(*this);
    operator++();
    return temp;
  }

  bool operator==(const split_iterator& rhs) const
  {
    return _value == rhs._value && _from == rhs._from && _length == rhs._length;
  }

  bool operator!=(const split_iterator& rhs) const
  {
    return !(*this == rhs);
  }

 private:
  std::string_view _value;
  std::size_t _from = 0;
  std::size_t _length = 0;
  std::size_t _index = 0;
  bool _malformed = false;
};

}  // namespace i18n::util
