#include "runtime_data.hpp"

namespace master {
ConnectionErrorData::ConnectionErrorData(runtime::TaskID task_id)
    : runtime::Data<DataType>{DataType::CONNECTION_ERROR, task_id} {}

ErrorData::ErrorData(runtime::TaskID task_id)
    : runtime::Data<DataType>{DataType::ERROR, task_id} {}
} // namespace master
