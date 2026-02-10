#include "CharVector.hpp"
#include <algorithm>
#include <cstring>

void CharVector::setup(size_t size) {
    buf.resize(size);
    pos = size > 0 ? buf.begin() : pos;
    if (pos)
        *(buf.end() - 1) = '\0';
}

int CharVector::extend(const void *data, size_t len) {
    if (!pos)
        return -1;
    size_t remaining = buf.end() - 1 - pos.value();
    size_t to_write = std::min(len, remaining);
    write(data, to_write);
    return to_write;
}

int CharVector::push(char c) {
    if (!pos || pos.value() == buf.end() - 1)
        return -1;
    *(pos.value()++) = c;
    return 0;
}

void CharVector::release(CharBuf &out) {
    auto temp = CharBuf{};
    buf.swap(temp);
    temp.swap(out);
    pos.reset();
}

void CharVector::clear() {
    buf.clear();
    pos.reset();
}

bool CharVector::is_setup() const noexcept { return pos.has_value(); }

void CharVector::write(const void *data, size_t len) {
    memcpy(&*pos.value(), data, len);
    pos.value() += len;
}
