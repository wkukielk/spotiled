#ifndef UTIL_ERRORS_HPP
#define UTIL_ERRORS_HPP

#include "esp_err.h"
#include <exception>
#include <format>
#include <string>

class BaseError : public std::exception {
  public:
    BaseError(const char *base, const char *msg, int reason = 0) : msg{base} {
        this->msg = std::format("{}: {}, reason: {} - {}", base, msg, reason,
                                esp_err_to_name(reason));
    }
    const char *what() const noexcept override { return msg.c_str(); };

  private:
    std::string msg{};
};

struct InitError : public BaseError {
    InitError(int reason, const char *msg)
        : BaseError("InitError", msg, reason){};
    InitError(int reason = 0) : BaseError("InitError", "", reason){};
};

struct NVSError : public BaseError {
    NVSError(int reason, const char *msg)
        : BaseError("NVSError", msg, reason){};
    NVSError(int reason = 0) : BaseError("InitError", "", reason){};
};

struct NullError : public BaseError {
    NullError(int reason, const char *msg)
        : BaseError("NullError", msg, reason){};
    NullError(int reason) : BaseError("NullError", "", reason){};
};

struct ValueError : public BaseError {
    ValueError(int reason, const char *msg)
        : BaseError("ValueError", msg, reason){};
    ValueError(int reason) : BaseError("ValueError", "", reason){};
};

struct RuntimeError : public BaseError {
    RuntimeError(int reason, const char *msg)
        : BaseError("RuntimeError", msg, reason){};
    RuntimeError(int reason) : BaseError("RuntimeError", "", reason){};
};

struct MissingKeyError : public BaseError {
    MissingKeyError(int reason, const char *msg, const char *key)
        : BaseError("RuntimeError", std::format("{}, key: {}", msg, key).data(),
                    reason){};
    MissingKeyError(int reason) : BaseError("RuntimeError", "", reason){};
};

#endif