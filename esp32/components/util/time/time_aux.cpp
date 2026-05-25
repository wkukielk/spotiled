#include "time_aux.hpp"
#include <sys/time.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "freertos/FreeRTOS.h"
#pragma GCC diagnostic pop

#include "freertos/task.h"

using namespace std::chrono;

time_t date_to_time_t(int year, int month, int day) {
    tm timeinfo{.tm_mday = day, .tm_mon = month - 1, .tm_year = year - 1900};
    return mktime(&timeinfo);
}

std::string time_aux_get_time() {
    auto now_tm = get_tm();
    char timebuf[25] = {};
    strftime(timebuf, 25, "%c", &now_tm);
    return std::string{timebuf};
}

int time_aux_set_time(int year, int month, int day) {
    time_t requested_time = date_to_time_t(year, month, day);
    timeval tv{.tv_sec = requested_time};
    return settimeofday(&tv, NULL);
}

void time_aux_delay_ms(int64_t delay) {
    vTaskDelay(delay / portTICK_PERIOD_MS);
}

MiliTimer::MiliTimer() { prev_t = steady_clock::now(); }

int64_t MiliTimer::stop() {
    if (already_stopped)
        throw std::runtime_error{"timer already stopped"};
    auto delta = duration_cast<milliseconds>(steady_clock::now() - prev_t);
    return delta.count();
}

bool operator==(const struct tm &tm1, const struct tm &tm2) {
    return tm1.tm_hour == tm2.tm_hour && tm1.tm_isdst == tm2.tm_isdst &&
           tm1.tm_mday == tm2.tm_mday && tm1.tm_min == tm2.tm_min &&
           tm1.tm_sec == tm2.tm_sec && tm1.tm_wday == tm2.tm_wday &&
           tm1.tm_yday == tm2.tm_yday && tm1.tm_year == tm2.tm_year;
}

std::unique_ptr<I_MiliTimer> Time::get_timer() {
    return std::make_unique<MiliTimer>();
};

void Time::delay_ms(int64_t delay) { vTaskDelay(delay / portTICK_PERIOD_MS); }