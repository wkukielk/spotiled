#ifndef LEDAUX_ANIMATOR_HPP
#define LEDAUX_ANIMATOR_HPP

#include "ledaux.hpp"
#include "rgb.hpp"
#include "util.hpp"
#include <atomic>
#include <optional>
#include <vector>

namespace led {

struct AnimationSpec {
    std::vector<RGB> colors;
    uint32_t step_ms;
};

} // namespace led

template <typename T>
concept I_Animatable = I_LEDC<T> && requires(T t, led::AnimationSpec spec) {
    { t.start_animation(spec) } -> std::same_as<void>;
};

template <I_LEDC LEDC> class Animator {
  public:
    Animator() noexcept = default;
    ~Animator() noexcept;
    esp_err_t fade_to(RGB rgb, uint32_t duration_ms) noexcept;
    esp_err_t set_color(RGB rgb) noexcept;
    RGB get_current_color() const noexcept;
    void start_animation(led::AnimationSpec spec);
    void close() noexcept;

  protected:
    LEDC ledc{};

  private:
    void stop_animation() noexcept;
    static void animate_fn(void *self_ptr);

    led::AnimationSpec spec{};
    std::atomic<bool> is_animating{false};
    std::atomic<bool> stop_requested{false};
    util::EventGroup done_group{};
    std::optional<util::Thread> animation_thread{};
    static constexpr EventBits_t DONE_BIT = 1 << 0;
};

#include "animator.tpp"

#endif
