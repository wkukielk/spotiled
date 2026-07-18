#ifndef LEDAUX_ANIMATOR_HPP
#include "animator.hpp"
#endif

#include "freertos/task.h"

template <I_LEDC LEDC> Animator<LEDC>::~Animator() noexcept {
    if (is_animating.load())
        ESP_LOGW("Animator", "destroyed with animation still running");
}

template <I_LEDC LEDC> void Animator<LEDC>::close() noexcept {
    if (is_animating.load())
        stop_animation();
}

template <I_LEDC LEDC> esp_err_t Animator<LEDC>::fade_to(RGB rgb) noexcept {
    if (is_animating.load())
        stop_animation();
    return ledc.fade_to(rgb);
}

template <I_LEDC LEDC> esp_err_t Animator<LEDC>::set_color(RGB rgb) noexcept {
    if (is_animating.load())
        stop_animation();
    return ledc.set_color(rgb);
}

template <I_LEDC LEDC> RGB Animator<LEDC>::get_current_color() const noexcept {
    return ledc.get_current_color();
}

template <I_LEDC LEDC>
void Animator<LEDC>::start_animation(led::AnimationSpec new_spec) {
    if (is_animating.load())
        stop_animation();
    spec = std::move(new_spec);
    stop_requested.store(false);
    done_group.clear(DONE_BIT);
    animation_thread.emplace(util::thread_params{.fun = animate_fn,
                                                 .name = "anim",
                                                 .fun_parameters = this,
                                                 .join_group = done_group,
                                                 .join_bit = DONE_BIT});
    is_animating.store(true);
}

template <I_LEDC LEDC> void Animator<LEDC>::stop_animation() noexcept {
    stop_requested.store(true);
    animation_thread->join();
    is_animating.store(false);
}

template <I_LEDC LEDC> void Animator<LEDC>::animate_fn(void *self_ptr) {
    auto *self = static_cast<Animator *>(self_ptr);
    if (self->spec.colors.empty())
        return;
    size_t idx = 0;
    while (!self->stop_requested.load()) {
        self->ledc.set_color(self->spec.colors[idx]);
        idx = (idx + 1) % self->spec.colors.size();
        vTaskDelay(pdMS_TO_TICKS(self->spec.step_ms));
    }
}
