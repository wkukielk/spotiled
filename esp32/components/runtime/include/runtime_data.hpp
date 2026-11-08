#ifndef RUNTIME_SHARED_DATA_HPP
#define RUNTIME_SHARED_DATA_HPP

#include "state_machine.hpp"
#include <variant>

namespace runtime {

enum class TaskID { MASTER, WS_SERVER };

template <typename DataType> struct Data : public sm::Data<DataType> {
    Data(DataType data_type, TaskID sender_id);
    TaskID get_sender_id() const noexcept;

  protected:
    TaskID sender_id;
};

} // namespace runtime

namespace master {

enum class DataType {
    CONNECTION_ERROR,
    ERROR,
    REPROVISION_REQUEST,
    CONNECT_REQUEST,
    START_REQUEST
};

struct ConnectionErrorData : public runtime::Data<DataType> {
    ConnectionErrorData(runtime::TaskID task_id);
};
struct ErrorData : public runtime::Data<DataType> {
    ErrorData(runtime::TaskID task_id);
};
struct ReprovisionRequest : public runtime::Data<DataType> {
    ReprovisionRequest(runtime::TaskID task_id);
};
// Connect to WiFi.
struct ConnectRequest : public runtime::Data<DataType> {
    ConnectRequest(runtime::TaskID task_id);
};
// Resolve backend and start OTA/server/UDP; queued after a successful connect.
struct StartRequest : public runtime::Data<DataType> {
    StartRequest(runtime::TaskID task_id);
};

using Data = std::variant<ConnectionErrorData, ErrorData, ReprovisionRequest,
                          ConnectRequest, StartRequest>;

} // namespace master

namespace server {

enum class DataType { ACK, NACK };

struct ACK : public runtime::Data<DataType> {
    ACK(runtime::TaskID task_id);
};

struct NACK : public runtime::Data<DataType> {
    NACK(runtime::TaskID task_id);
};

using Data = std::variant<ACK, NACK>;

} // namespace server

#include "runtime_data.tpp"

#endif
