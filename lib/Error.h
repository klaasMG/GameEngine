#ifndef SUPERBUILD_ERROR_H
#define SUPERBUILD_ERROR_H

#include <iostream>
#include <string>

enum class ErrorType {
    OK,
    NOT_IMPLEMENTED,
    FILE_NOT_FOUND,
    FILE_IN_USE,
    FILE_DATA_ERROR,
    QUEUE_EMPTY,
};

inline std::string ErrorType_to_string(ErrorType type) {
    switch (type) {
        case ErrorType::OK: return "OK";
        case ErrorType::NOT_IMPLEMENTED: return "Not implemented";
        case ErrorType::FILE_NOT_FOUND: return "File not found";
        case ErrorType::FILE_IN_USE: return "File in use";
        case ErrorType::FILE_DATA_ERROR: return "File data error";
        case ErrorType::QUEUE_EMPTY: return "Queue is empty";
    }

    std::cerr << "Unhandled ErrorType\n";
    std::terminate();
}

template<typename Data>
class [[nodiscard]] Result {
public:
    Result() = delete;
    Result(const Result&) = delete;
    Result& operator=(const Result&) = delete;
    Result(Result&&) = default;
    Result& operator=(Result&&) = default;

    Result(const Data& data) {
        this->data = data;
        type = ErrorType::OK;
    }

    Result(const Data& data, ErrorType type) {
        if (type == ErrorType::OK) {
            std::cerr << "error cannot be OK\n";
            std::terminate();
        }
        this->data = data;
        this->type = type;
    }

    Result(ErrorType type) {
        if (type == ErrorType::OK) {
            std::cerr << "error cannot be OK\n";
            std::terminate();
        }
        this->type = type;
        this->data = Data{};
    }

    [[nodiscard]] ErrorType check_error() {
        is_error_checked = true;
        return type;
    }

    Data GetData() const {
        if (!is_error_checked) {
            std::cerr << "check error first\n";
            std::terminate();
        }
        return data;
    }

    Data Handle_Error() {
        if (!is_error_checked) {
            std::terminate();
        }
        is_error_handled = true;
        type = ErrorType::OK;
        return data;
    }

    ~Result() {
        if (!is_error_handled) {
            std::terminate();
        }
    }

private:
    bool is_error_handled = false;
    bool is_error_checked = false;
    ErrorType type = ErrorType::OK;
    Data data{};
};

#endif // SUPERBUILD_ERROR_H