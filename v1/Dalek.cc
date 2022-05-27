// Dalek
#include <sys/wait.h>
#include <unistd.h>

#include "http/httpd.hpp"
#include "pson/v2/src/json_parser.hpp"
#include "reactor/EventLoop.hpp"

struct conf {
  conf(pson::Object &o) 
  {
    worker = static_cast<int>(o["worker"].as<pson::Number>());
    port = static_cast<int>(o["port"].as<pson::Number>());
    root = o["root"].as_string();
  }
  std::string root;
  int worker;
  int port;
};

static void version() {
  printf("Dalek version: 1.0.0\n");
  printf("Author: Pink\n");
  printf("Github: github.com/PinkDoc\n");
}

// Kill workers and master
static void exterminate(int signo) {
  kill(-getpid(), SIGINT);
  exit(1);
}

static void save_pid() {
  FILE *to_save = fopen("Dalek.pid", "a+");
  if (to_save == nullptr) {
    fprintf(stderr, "Dalek.pid not exist!\n");
    exit(1);
  }
  pid_t master_pid = getpid();
  fprintf(to_save, "%d\n", master_pid);
  fclose(to_save);
}

pson::Value init_conf() {
  char buffer[65535];
  bzero(buffer, 65535);

  FILE *file = fopen("conf.json", "r");

  if (!file) {
    fprintf(stderr, "Dalek: Dalek can't find conf.json\n");
    fprintf(stderr, "Dalek: use ./Dalek --usage");
    fclose(file);
    exit(1);
  }

  struct stat file_stat;
  stat("conf.json", &file_stat);
  int ret = fread(static_cast<void *>(buffer), 65535, file_stat.st_size, file);
  if (ret < 0) {
    fprintf(stderr, "Dalek: Dalek can't read from conf.json\n");
  }

  pson::Value obj(pson::JSON_OBJECT);
  obj.parse(buffer, 65535);
  fclose(file);
  fprintf(stdout, "%d\n", obj.size());
  return obj;
}

int main(int argc, char *argv[]) {
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
  if (son != 0)
    exit(1);
  if (setsid() == -1) {
    fprintf(stderr, "Dalek can't set sid!\n");
  }

  pinkx::SyncLogger::init("Dalek.log");
  auto val = init_conf();
  conf conf(val.as_object());

  signal(SIGPIPE, SIG_IGN); // Client closed
  signal(SIGINT, exterminate);

  int port = conf.port;
  int numberOfWorker = conf.worker;

  save_pid();

  int newWorkers = 0;

  chdir(conf.root.c_str());

  while (true) {
    if (newWorkers == numberOfWorker) {
      int n;
      wait(&n); // Keep
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

} // :)
