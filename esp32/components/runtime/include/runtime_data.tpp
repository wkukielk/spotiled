namespace runtime {
template <typename DataType>
Data<DataType>::Data(DataType data_type, TaskID sender_id)
    : sm::Data<DataType>{data_type}, sender_id{sender_id} {}

template <typename DataType>
TaskID Data<DataType>::get_sender_id() const noexcept {
    return sender_id;
}
} // namespace runtime
