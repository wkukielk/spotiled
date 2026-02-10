#ifndef CHAR_VECTOR_HPP
#define CHAR_VECTOR_HPP

#include "straux.hpp"
#include <optional>

class CharVector {
  public:
    void setup(size_t size);
    int extend(const void *data, size_t len);
    int push(char c);
    void release(CharBuf &out);
    void clear();
    bool is_setup() const noexcept;

  private:
    void write(const void *data, size_t len);
    CharBuf buf{};
    std::optional<CharBuf::iterator> pos{};
};

#endif
