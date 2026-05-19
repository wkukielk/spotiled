#if !defined(SSTATE_MACHINE_HPP)
#include "state_machine.hpp"
#endif

template <typename DataType>
Data<DataType>::StateData(DataType type) : type{type} {}

template <typename DataType>
DataType Data<DataType>::get_type() const noexcept {
    return type;
}
