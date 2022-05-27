#pragma once

#include "log_file.hpp"

#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include <cstdint>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <functional>
#include <memory>
#include <string>
#include <mutex>
#include <thread>
#include <vector>
#include <condition_variable>

namespace core {
namespace imple {

enum LogLevel { INFO,
                WARN,
                ERROR };

char *const to_string(LogLevel l) {
    switch (l) {
    case INFO:
        return "INFO";
        break;
    case WARN:
        return "WARN";
        break;
    case ERROR:
        return "ERROR";
        break;
    }
    return "NON-LEVEL";
}

class buffer {
private:
    std::vector<char> buffer_;
    std::size_t end_index_;

    enum {
        buffer_size_ = 1024 * 1024 * 4,
    };

public:
    buffer(const buffer &) = delete;

    buffer &operator=(const buffer &) = delete;

    buffer();

    char *peek();

    std::size_t remain() const;

    std::size_t size() const;

    void append(const char *log, std::size_t len);

    void reset();
};

class logging {
private:
    std::mutex mutex_;
    std::condition_variable condition_;
    std::vector<std::unique_ptr<buffer>> buffers_; // Write to file

    std::unique_ptr<buffer> current_buf_;
    std::unique_ptr<buffer> next_buf_;

    std::unique_ptr<std::thread> log_thread_;

    bool stop_;

    log_file file_;

public:
    logging(const logging &) = delete;

    logging &operator=(const logging &) = delete;

    logging(const char *name);

    ~logging();

    void append(const char *log, std::size_t len);

    void thread_loop();

    void run();
};

class logstream {
private:
    constexpr static std::size_t BufferSize = 4096;

    char stack_buffer_[BufferSize];
    std::size_t end_;

    std::size_t remain() const {
        return BufferSize - end_;
    }

public:
    logstream() :
        end_(0) {
    }

    void append(char *s, std::size_t len) {
        std::size_t l = std::min(len, remain());
        std::memcpy(stack_buffer_ + end_, s, l);
        end_ += l;
    }

    logstream &operator<<(bool b) {
        const char *s = b ? "true" : "false";
        std::size_t l = b ? 4 : 5;
        append(const_cast<char *>(s), l);
        return *this;
    }

    logstream &operator<<(char c) {
        if (remain() >= 1) {
            stack_buffer_[end_++] = c;
        }
        return *this;
    }

    logstream &operator<<(unsigned char c) {
        *this << static_cast<char>(c);
        return *this;
    }

    logstream &operator<<(int n) {
        auto r = snprintf(stack_buffer_ + end_, remain(), "%d", n);
        end_ += r;
        return *this;
    }

    logstream &operator<<(unsigned int n) {
        auto r = snprintf(stack_buffer_ + end_, remain(), "%u", n);
        end_ += r;
        return *this;
    }

    logstream &operator<<(long n) {
        auto r = snprintf(stack_buffer_ + end_, remain(), "%ld", n);
        end_ += r;
        return *this;
    }

    logstream &operator<<(char *s) {
        if (s) {
            append(s, strlen(s));
        } else {
            append("null", 4);
        }
        return *this;
    }

    logstream &operator<<(const char *s) {
        *this << const_cast<char *>(s);
        return *this;
    }

    logstream &operator<<(const std::string &s) {
        append(const_cast<char *>(s.c_str()), s.size());
        return *this;
    }

    char *peek() {
        return stack_buffer_;
    }
    std::size_t size() {
        return end_;
    }
};

class logline {
private:
    logstream stream_;

    void append_time() {
        auto t = time(nullptr);
        auto ts = ctime(&t);
        stream_ << "[Time :";
        stream_.append(ts, strlen(ts) - 1);
        stream_ << "]";
    }

public:
    logline(LogLevel l, char *filename, int line) {
        stream_ << "[" << to_string(l) << "]"
                << "[File : " << filename << "]"
                << "[Line : " << line << "]";
        append_time();
    }

    logstream &stream() {
        return stream_;
    }
};

inline buffer::buffer() :
    buffer_(buffer_size_),
    end_index_(0) {
}

inline char *buffer::peek() {
    return buffer_.data();
}

inline std::size_t buffer::remain() const {
    return buffer_size_ - end_index_;
}

inline void buffer::append(const char *log, std::size_t len) {
    memcpy(buffer_.data() + end_index_, log, len);
    end_index_ += len;
}

inline std::size_t buffer::size() const {
    return end_index_;
}

inline void buffer::reset() {
    end_index_ = 0;
}

inline logging::logging(const char *filename) :
    current_buf_(new buffer()),
    next_buf_(new buffer()),
    stop_(false),
    file_(filename) {
}

inline logging::~logging() {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        stop_ = true;
    }

    file_.flush();
    condition_.notify_all();
    log_thread_->join(); // Wait for dead

    if (current_buf_)
        file_.write(current_buf_->peek(), current_buf_->size());
    if (next_buf_)
        file_.write(next_buf_->peek(), next_buf_->size());
    for (auto &i : buffers_)
        if (i)
            file_.write(i->peek(), i->size());
    file_.flush();
}

inline void logging::run() {
    log_thread_.reset(new std::thread(std::bind(&logging::thread_loop, this)));
}

inline void logging::append(const char *log, std::size_t len) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (current_buf_->remain() > len) {
        current_buf_->append(log, len);
    } else {
        buffers_.push_back(std::move(current_buf_));

        if (!next_buf_) {
            next_buf_->reset();
            current_buf_ = std::move(next_buf_);
        } else {
            current_buf_.reset(new buffer());
        }

        current_buf_->append(log, len);
        condition_.notify_all();
    }
}

inline void logging::thread_loop() {
    std::unique_ptr<buffer> buffer1(new buffer());
    std::unique_ptr<buffer> buffer2(new buffer());
    std::vector<std::unique_ptr<buffer>> bufferToWrite;
    while (!stop_) {
        {
            std::unique_lock<std::mutex> lock(mutex_);

            if (stop_) break;

            if (buffers_.empty())
                condition_.wait(lock);

            buffers_.push_back(std::move(current_buf_));
            current_buf_ = std::move(buffer1);
            bufferToWrite.swap(buffers_);

            if (!next_buf_) {
                next_buf_ = std::move(buffer2);
            }
        } // End

        for (const auto &i : bufferToWrite) {
            // Write to file
            file_.write(i->peek(), i->size());
        }

        if (!buffer1) {
            buffer1 = std::move(bufferToWrite.back());
            bufferToWrite.pop_back();
            buffer1->reset();
        }

        if (!buffer2) {
            buffer2 = std::move(bufferToWrite.back());
            bufferToWrite.pop_back();
            buffer2->reset();
        }

        bufferToWrite.clear();
        // flush
        file_.flush();
    }
}

} // namespace imple

inline std::unique_ptr<imple::logging> loggingPtr;

struct Dalek_logger {
    bool operator==(imple::logstream &l) {
        l.append("\n", 1);
        loggingPtr->append(l.peek(), l.size());
        return true;
    }
};
} // namespace core

#define Dalek_LoggerInit(filename)                                  \
    do {                                                            \
        core::loggingPtr.reset(new core::imple::logging(filename)); \
        core::loggingPtr->run();                                    \
    } while (0)

#define DLOG(level) \
    core::Dalek_logger() == core::imple::logline(level, __FILE__, __LINE__).stream()
#define DLOG_WARN \
    DLOG(core::imple::WARN)
#define DLOG_INFO \
    DLOG(core::imple::INFO)
#define DLOG_ERROR \
    DLOG(core::imple::ERROR)
