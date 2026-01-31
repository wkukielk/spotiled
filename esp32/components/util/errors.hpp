#ifndef UTIL_ERRORS_HPP
#define UTIL_ERRORS_HPP

#include <exception>
#include <string>

class BaseError : public std::exception {
  public:
    BaseError(const char *base, const char *msg) : msg{base} {
        this->msg += ": ";
        this->msg += msg;
    }
    const char *what() const noexcept override { return msg.c_str(); };

  private:
    std::string msg;
};

struct InitError : public BaseError {
    InitError(const char *msg) : BaseError("InitError", msg){};
    InitError() : BaseError("InitError", ""){};
};

struct NVSError : public BaseError {
    NVSError(const char *msg) : BaseError("NVSError", msg){};
    NVSError() : BaseError("InitError", ""){};
};

#endif