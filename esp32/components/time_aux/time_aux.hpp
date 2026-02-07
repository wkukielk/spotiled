#ifndef TIME_AUX
#define TIME_AUX

extern "C" {
#include "time_aux_core.hpp"
}
#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <time.h>

time_t date_to_time_t(int year, int month, int day);
std::string time_aux_get_time();
int time_aux_set_time(int year, int month, int day);
void time_aux_delay_ms(int64_t delay);

bool operator==(const struct tm &tm1, const struct tm &tm2);

class I_MiliTimer {
  public:
    virtual int64_t stop() = 0;
};

class I_Time {
  public:
    virtual std::unique_ptr<I_MiliTimer> get_timer() = 0;
    virtual void delay_ms(int64_t delay) = 0;
};

class MiliTimer : public I_MiliTimer {
  public:
    MiliTimer();
    int64_t stop() override;

  private:
    std::chrono::steady_clock::time_point prev_t;
    bool already_stopped = false;
};

class Time : public I_Time {
  public:
    std::unique_ptr<I_MiliTimer> get_timer() override;
    void delay_ms(int64_t delay) override;
};

#endif
