#include "runtime_data.hpp"

namespace master {
ConnectionErrorData::ConnectionErrorData(runtime::TaskID task_id)
    : runtime::Data<DataType>{DataType::CONNECTION_ERROR, task_id} {}

ErrorData::ErrorData(runtime::TaskID task_id)
    : runtime::Data<DataType>{DataType::ERROR, task_id} {}

ReprovisionRequest::ReprovisionRequest(runtime::TaskID task_id)
    : runtime::Data<DataType>{DataType::REPROVISION_REQUEST, task_id} {}
} // namespace master

namespace server {

ACK::ACK(runtime::TaskID task_id)
    : runtime::Data<DataType>{DataType::ACK, task_id} {}

NACK::NACK(runtime::TaskID task_id)
    : runtime::Data<DataType>{DataType::NACK, task_id} {}
} // namespace server