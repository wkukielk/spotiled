#ifndef RUNTIME_SHARED_DATA_HPP
#define RUNTIME_SHARED_DATA_HPP

#include "state_machine.hpp"

namespace runtime {

enum class DataType { COLOR_DATA, CONNECTION_ERROR, ERROR };

struct ColorData : public sm::Data<DataType> {
    ColorData() : sm::Data<DataType>{DataType::ERROR} {}
};
struct ConnectionErrorData : public sm::Data<DataType> {
    ConnectionErrorData() : sm::Data<DataType>{DataType::CONNECTION_ERROR} {}
};
struct ErrorData : public sm::Data<DataType> {
    ErrorData() : sm::Data<DataType>{DataType::ERROR} {}
};

} // namespace runtime

#endif
