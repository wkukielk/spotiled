#ifndef SSTATE_MACHINE_HPP
#define SSTATE_MACHINE_HPP

#include "util.hpp"

namespace sm {

template <typename DataType> class Data {
  public:
    Data(DataType type);
    DataType get_type() const noexcept;

  private:
    DataType type;
};

template <typename StateData, typename Data> class State {
  public:
    virtual ~State() noexcept {};
    virtual int perform(Data *data) = 0;
    virtual StateData get_next_state_data() const noexcept = 0;
};
} // namespace sm

template <typename Data> class I_StateMachine {
  public:
    virtual void notify(Data *data) = 0;
};

#include "state_machine.tpp"

#endif
