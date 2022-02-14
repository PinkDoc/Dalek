// Dalek
#include <sys/wait.h>
#include <unistd.h>

#include "http/httpd.h"
#include "reactor/EventLoop.h"

static void Exterminate(int signo) {
  kill(-getpid(), SIGINT);
  exit(1);
}

static void SavePid() {
  FILE* to_save = fopen("Dalek.pid", "a+");
  if (to_save == nullptr) {
    fprintf(stderr, "Dalek.pid not exist!\n");
    assert(false);
  }
  pid_t master_pid = getpid();
  fprintf(to_save, "%d\n", master_pid);
  fclose(to_save);
}

int main(int argc, char* argv[]) {
  pinkx::SyncLogger::init("Dalek.log");

  if (argc < 2) {
    fprintf(stderr, "Dalek need more arguments...\n");
    exit(1);
  }

  signal(SIGPIPE, SIG_IGN);  // Client closed
  signal(SIGINT, Exterminate);

  std::string_view arg(argv[1]);

  if (arg != "start") {
    fprintf(stderr, "Dalek get error argument!\n");
    exit(1);
  }

  int port = atoi(argv[2]);
  int numberOfWorker = atoi(argv[3]);

  SavePid();

  int newWorkers = 0;

  while (true) {
    if (newWorkers == numberOfWorker) {
      int n;
      wait(&n);  // Keep
    }
    pid_t w = fork();
    newWorkers++;
    if (w == 0) break;
  }

worker:

  pinkx::net::InetAddress address(port, false);
  pinkx::EventLoop looper;
  pinkx::TimerWheel timer(looper);
  pinkx::http::HttpServer Server(looper, timer, address);

  looper.loop();

}  // :)
