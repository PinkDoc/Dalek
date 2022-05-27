#pragma once

#include <string>
#include <stdio.h>
namespace core {
namespace imple {

class log_file {
private:
    FILE *file_;

public:
    log_file(const std::string &filename);

    log_file(const char *filename);

    ~log_file();

    void write(const char *logs, std::size_t len);

    void flush();
};

inline log_file::log_file(const std::string &filename) {
    file_ = fopen(filename.c_str(), "a+");
}

inline log_file::log_file(const char *filename) {
    file_ = fopen(filename, "a+");
}

inline void log_file::write(const char *log, std::size_t len) {
    fwrite_unlocked(log, len, 1, file_);
}

inline void log_file::flush() {
    fflush(file_);
}

inline log_file::~log_file() {
    fclose(file_);
}

}
} // namespace core::imple