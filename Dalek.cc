// Dalek
#include <sys/wait.h>
#include <unistd.h>

#include "base/jsonxx.h"
#include "http/httpd.h"
#include "reactor/EventLoop.h"

void usage() {
  printf(
      "Before you start! You should move Dalek to root floder, and create a "
      "conf.json!\n");
  printf("conf.json should is just like: \"worker : [number] \" \n");
  printf("and the port of Dalek: \"port: [port]\" \n");
  printf("./Dalek");
}

void version() {
  printf("Dalek version: 1.0.0\n");
  printf("Author: Pink\n");
  printf("Github: github.com/PinkDoc\n");
}

struct Conf {
  explicit Conf(const jsonxx::Object& o) {
    // assert((int)o.has<jsonxx::Number>("worker"));
    //  assert((int)o.has<jsonxx::Number("port")>);
    worker = o.get<jsonxx::Number>("worker");
    port = o.get<jsonxx::Number>("port");
    root = o.get<jsonxx::String>("root");
  }
  std::string root;
  int worker;
  int port;
};

// Kill workers and master
static void Exterminate(int signo) {
  kill(-getpid(), SIGINT);
  exit(1);
}

static void SavePid() {
  FILE* to_save = fopen("Dalek.pid", "a+");
  if (to_save == nullptr) {
    fprintf(stderr, "Dalek.pid not exist!\n");
    exit(1);
  }
  pid_t master_pid = getpid();
  fprintf(to_save, "%d\n", master_pid);
  fclose(to_save);
}

jsonxx::Object InitConf() {
  char buffer[1024];
  FILE* file = fopen("conf.json", "r");
  if (!file) {
    fprintf(stderr, "Dalek: Dalek can't find conf.json\n");
    fprintf(stderr, "Dalek: use ./Dalek --usage");
    exit(1);
  }
  struct stat file_stat;
  stat("conf.json", &file_stat);
  int ret = fread(static_cast<void*>(buffer), 1024, file_stat.st_size, file);
  if (ret < 0) {
    fprintf(stderr, "Dalek: Dalek can't read from conf.json\n");
  }
  std::string buf(buffer);
  jsonxx::Object obj;
  obj.parse(buf);
  return obj;
}

int main(int argc, char* argv[]) {
  if (argc > 1) {
    std::string_view arg(argv[1]);

    if (arg == "--usage") {
      usage();
    }

    if (arg == "--version") {
      version();
    }

    exit(1);
  }

  // Create a guard process
  pid_t son = fork();
  if (son == -1) {
    fprintf(stderr, "Dalek can't fork a child!\n");
    exit(1);
  }
  if (son != 0) exit(1);
  if (setsid() == -1) {
    fprintf(stderr, "Dalek can't set sid!\n");
  }

  pinkx::SyncLogger::init("Dalek.log");
  jsonxx::Object o(InitConf());
  Conf conf(o);

  chdir(conf.root.c_str());

  signal(SIGPIPE, SIG_IGN);  // Client closed
  signal(SIGINT, Exterminate);

  int port = conf.port;
  int numberOfWorker = conf.worker;

  SavePid();

  int newWorkers = 0;

  while (true) {
    if (newWorkers == numberOfWorker) {
      int n;
      wait(&n);  // Keep
    }
    pid_t w = fork();
    newWorkers++;
    if (w == 0) {
      break;
    }
  }

worker:

  pinkx::net::InetAddress address(port, false);
  pinkx::EventLoop looper;
  pinkx::TimerWheel timer(looper);
  pinkx::http::HttpServer Server(looper, timer, address);

  looper.loop();

}  // :)
