// Dalek
#include <sys/wait.h>
#include <unistd.h>

#include "pson/Parser.hpp"
#include "http/httpd.hpp"
#include "reactor/EventLoop.hpp"


void version() {
  printf("Dalek version: 1.0.0\n");
  printf("Author: Pink\n");
  printf("Github: github.com/PinkDoc\n");
}

struct Conf {
  Conf(pson::Object& o) {
    worker = o.GetAsNumber("worker");
    port = o.GetAsNumber("port");
    root = o.GetAsString("root");
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

pson::Value* InitConf() {
  char buffer[65535];
  bzero(buffer, 65535);

  FILE* file = fopen("conf.json", "r");

  if (!file) {
    fprintf(stderr, "Dalek: Dalek can't find conf.json\n");
    fprintf(stderr, "Dalek: use ./Dalek --usage");
    exit(1);
  }

  struct stat file_stat;
  stat("conf.json", &file_stat);
  int ret = fread(static_cast<void*>(buffer), 65535, file_stat.st_size, file);
  if (ret < 0) {
    fprintf(stderr, "Dalek: Dalek can't read from conf.json\n");
  }

  pson::Value* obj = new pson::Value();
  pson::Parser parser(buffer, file_stat.st_size);
  bool parseOk = parser.Parse(*obj);
  return obj;
}

int main(int argc, char* argv[]) {
  if (argc > 1) {
    std::string_view arg(argv[1]);

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
  std::unique_ptr<pson::Value> val(InitConf());
  pson::Object o(val->AsObject());
  Conf conf(o);

  signal(SIGPIPE, SIG_IGN);  // Client closed
  signal(SIGINT, Exterminate);

  int port = conf.port;
  int numberOfWorker = conf.worker;

  SavePid();

  int newWorkers = 0;
  chdir(conf.root.c_str());
  
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
