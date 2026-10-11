#ifndef RUNTIME_HELPERS_HPP
#include "runtime_helpers.hpp"
#endif

template <typename T_OTAClient>
void OTAThread<T_OTAClient>::ota_thread_fun(void *ota_thread_ptr) {
    auto ota_thread = (OTAThread<T_OTAClient> *)ota_thread_ptr;
    ota_thread->ota_client.update(); // TODO: reboot after successfull update
}

template <typename T_OTAClient>
OTAThread<T_OTAClient>::OTAThread(T_OTAClient &&ota_client,
                                  const util::JoinEventGroup &join_group)
    : PostponedThread{join_group}, ota_client{std::move(ota_client)} {}

template <typename T_OTAClient> void OTAThread<T_OTAClient>::start() {
    thread.emplace(util::thread_params{.fun = ota_thread_fun,
                                       .name = "ota",
                                       .stack_depth = 8192,
                                       .fun_parameters = this,
                                       .join_group = join_group.egroup,
                                       .join_bit = join_group.join_bit});
}
