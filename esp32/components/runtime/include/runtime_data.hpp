#ifndef RUNTIME_SHARED_DATA_HPP
#define RUNTIME_SHARED_DATA_HPP

#include "state_machine.hpp"

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

enum class DataType { CONNECTION_ERROR, ERROR };

struct ConnectionErrorData : public runtime::Data<DataType> {
    ConnectionErrorData(runtime::TaskID task_id);
};
struct ErrorData : public runtime::Data<DataType> {
    ErrorData(runtime::TaskID task_id);
};

} // namespace master

#include "runtime_data.tpp"

#endif
