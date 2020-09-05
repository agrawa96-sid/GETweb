/**
 * This file parses the command line arguments and correctly
 * starts your server. You should not need to edit this file
 */

#include <unistd.h>

#include <sys/resource.h>

#include <sys/types.h>

#include <sys/wait.h>

#include <csignal>

#include <cstdio>

#include <iostream>

#include "server.hh"

#include "socket.hh"

#include "tcp.hh"

#include "tls.hh"

enum concurrency_mode {
  E_NO_CONCURRENCY = 0,
    E_FORK_PER_REQUEST = 'f',
    E_THREAD_PER_REQUEST = 't',
    E_THREAD_POOL = 'p'
};

extern "C"
void signal_handler(int signal) {
  exit(0);
}

extern "C"
void zomb(int sig) {
  while (waitpid(-1, NULL, WNOHANG) > 0) {
    sig = sig;
  }
}

void zombhandler() {
  // Handle Zombie Processes
  struct sigaction zombie;
  zombie.sa_handler = zomb;
  sigemptyset(&zombie.sa_mask);
  zombie.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, & zombie, NULL)) {
    perror("sigaction");
    exit(2);
  }
}

int main(int argc, char ** argv) {
  /*    struct rlimit mem_limit = { .rlim_cur = 40960000, .rlim_max = 91280000 };
      struct rlimit cpu_limit = { .rlim_cur = 300, .rlim_max = 600 };
      struct rlimit nproc_limit = { .rlim_cur = 50, .rlim_max = 100 };
      if (setrlimit(RLIMIT_AS, &mem_limit)) {
          perror("Couldn't set memory limit\n");
      }
      if (setrlimit(RLIMIT_CPU, &cpu_limit)) {
          perror("Couldn't set CPU limit\n");
      }
      if (setrlimit(RLIMIT_NPROC, &nproc_limit)) {
          perror("Couldn't set NPROC limit\n");
      }
  */
  struct sigaction sa;
  sa.sa_handler = signal_handler;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGINT, & sa, NULL);
  enum concurrency_mode mode = E_NO_CONCURRENCY;
  zombhandler();
  char use_https = 0;
  int port_no = 0;
  int num_threads = 0;   // for use when running in pool of threads mode

  char usage[] = "USAGE: myhttpd [-f|-t|-pNUM_THREADS] [-s] [-h] PORT_NO\n";

  if (argc == 1) {
    fputs(usage, stdout);
    return 0;
  }

  int c;
  while ((c = getopt(argc, argv, "hftp:s")) != -1) {
    switch (c) {
    case 'h':
      fputs(usage, stdout);
      return 0;
    case 'f':
    case 't':
    case 'p':
      if (mode != E_NO_CONCURRENCY) {
        fputs("Multiple concurrency modes specified\n", stdout);
        fputs(usage, stderr);
        return -1;
      }
      mode = (enum concurrency_mode) c;
      if (mode == E_THREAD_POOL) {
        num_threads = stoi(std::string(optarg));
      }
      break;
    case 's':
      use_https = 1;
      break;
    case '?':
      if (isprint(optopt)) {
        std::cerr << "Unknown option: -" << static_cast < char > (optopt) <<
          std::endl;
      } else {
        std::cerr << "Unknown option" << std::endl;
      }
      // Fall through
    default:
      fputs(usage, stderr);
      return 1;
    }
  }

  if (optind > argc) {
    std::cerr << "Extra arguments were specified" << std::endl;
    fputs(usage, stderr);
    return 1;
  } else if (optind == argc) {
    std::cerr << "Port number must be specified" << std::endl;
    return 1;
  }

  port_no = atoi(argv[optind]);
  printf("%d %d %d %d\n", mode, use_https, port_no, num_threads);

  SocketAcceptor * acceptor;
  if (use_https) {
    acceptor = new TLSSocketAcceptor(port_no);
  } else {
    acceptor = new TCPSocketAcceptor(port_no);
  }
  Server server(*acceptor);
  switch (mode) {
  case E_FORK_PER_REQUEST:
    server.run_fork();
    break;
  case E_THREAD_PER_REQUEST:
    server.run_thread();
    break;
  case E_THREAD_POOL:
    server.run_thread_pool(num_threads);
    break;
  default:
    server.run_linear();
    break;
  }
  delete acceptor;
}
