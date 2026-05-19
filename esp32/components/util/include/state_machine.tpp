#if !defined(SSTATE_MACHINE_HPP)
#include "state_machine.hpp"
#endif

namespace sm {

template <typename DataType> Data<DataType>::Data(DataType type) : type{type} {}

template <typename DataType>
DataType Data<DataType>::get_type() const noexcept {
    return type;
}

} // namespace sm