// Logger
#ifndef PINK_SYNC_LOGGER_H
#define PINK_SYNC_LOGGER_H

#include <assert.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>

#include <memory>
#include <mutex>

#include "noncopyable.h"

namespace pinkx {

class AppendFile : pinkx::noncopyable {
 private:
  FILE* file_;
  char buffer_[64 * 1024];
  off_t writtenBytes_;

  // Not thread safe
  size_t write(const char* logline, size_t len) {
    return fwrite_unlocked(logline, 1, len, file_);
  }

 public:
  explicit AppendFile(char* filename);

  ~AppendFile();

  void append(const char* logline, int len);

  void flush();

  FILE* file() const { return file_; }

  off_t writtenBytes() const { return writtenBytes_; }
};

AppendFile::AppendFile(char* filename)
    : file_(fopen(filename, "ae")), writtenBytes_(0) {
  assert(file_);
  setbuffer(file_, buffer_, sizeof buffer_);
}

AppendFile::~AppendFile() { fclose(file_); }

void AppendFile::append(const char* logline, int len) {
  size_t written = 0;

  while (written != len) {
    size_t remain = len - written;
    size_t n = write(logline + written, remain);
    if (n != remain) {
      int err = ferror(file_);
      if (err) {
        fprintf(stderr, "AppendFile::append() fail!");
        break;
      }
    }
    written += n;
  }

  writtenBytes_ += written;
}

void AppendFile::flush() { fflush(file_); }

//
//
// Sync logger:TRACE, DEBUG, INFO, WARN, ERROR, FATAL
//
//

class SyncLogger : pinkx::noncopyable {
 private:
  static std::unique_ptr<AppendFile> file_;
  static std::unique_ptr<std::mutex> mutex_;

  static void write(int level) {
    AppendFile* f = file_.get();
    fprintf(f->file(), LogLevel[level]);
  }

 public:
  enum Level { TRACE = 0, DEBUG, INFO, WARN, ERROR, FATAL };

  static char* LogLevel[6];

  static void init(char* filename) {
    file_ = std::move(std::unique_ptr<AppendFile>(new AppendFile(filename)));
  }

  static void append(int level, int line, const char* format, ...) {
    if (file_) {
      std::unique_lock<std::mutex> lock(*mutex_);
      time_t t = time(nullptr);
      FILE* log = file_.get()->file();
      struct tm tm = *localtime(&t);
      fprintf(log, "[%4d: %02d: %02d: %02d: %02d: %02d][pid: %5d][line: %d]",
              tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour,
              tm.tm_min, tm.tm_sec, getpid(), line);

      write(level);
      va_list args;
      va_start(args, format);
      vfprintf(log, format, args);
      va_end(args);
      fprintf(log, "\n");
    }
  }
};

std::unique_ptr<AppendFile> SyncLogger::file_ = nullptr;
std::unique_ptr<std::mutex> SyncLogger::mutex_ =
    std::unique_ptr<std::mutex>(new std::mutex());

char* SyncLogger::LogLevel[] = {
    "TRACE ", "DEBUG ", "INFO ", "WARN ", "ERROR ", "FATAL ",
};

/*
    P_LOG_ERROR("cPP number %d%s", 1,  " yes");
    P_LOG_FATAL("Ruster  %s", "NB");
    P_LOG_INFO(" java %s", "yes");
    P_LOG_TRACE("f");
    P_LOG_WARN("Golang  ");
    P_LOG_WARN("6666%d", 666666666666);
*/

#define P_LOG_TRACE(format...) \
  SyncLogger::append(SyncLogger::TRACE, __LINE__, format)
#define P_LOG_DEBUG(format...) \
  SyncLogger::append(SyncLogger::DEBUG, __LINE__, format)
#define P_LOG_INFO(format...) \
  SyncLogger::append(SyncLogger::INFO, __LINE__, format)
#define P_LOG_WARN(format...) \
  SyncLogger::append(SyncLogger::WARN, __LINE__, format)
#define P_LOG_ERROR(format...) \
  SyncLogger::append(SyncLogger::ERROR, __LINE__, format)
#define P_LOG_FATAL(format...) \
  SyncLogger::append(SyncLogger::FATAL, __LINE__, format)

}  // namespace pinkx
#endif