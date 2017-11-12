#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#include "optparse.h"

#define MAX_EVENTS 2

static int initialize_timer();
static int initialize_epoll(int timerfd);

int main(int argc, char *argv[]) {
  // 引数パース
  options opt;
  parse_args(argc, argv, &opt);

  printf("outpin=%d\n", opt.outpin);

  // ファイルディスクリプタの初期化
  int timerfd = initialize_timer();
  int epollfd = initialize_epoll(timerfd);

  // メインループ
  uint8_t stdin_buf[2] = {0, 0};
  int stdin_read_bytes = 0;
  while (1) {
    struct epoll_event events[MAX_EVENTS];
    int nfds;

    nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
    assert(nfds <= MAX_EVENTS);
    if (nfds == -1) {
      perror("epoll_wait");
      return EXIT_FAILURE;
    }
    for (int i = 0; i < nfds; i++) {
      int fd = events[i].data.fd;
      if (fd == timerfd) {
        // do timer stuff
        printf("DEBUG: timer fire\n");
      } else if (fd == STDIN_FILENO) {
        // read stdin
        ssize_t read_bytes = read(STDIN_FILENO,
                                  stdin_buf + stdin_read_bytes,
                                  sizeof(stdin_buf) - stdin_read_bytes);
        if (read_bytes == -1) {
          perror("read");
          return EXIT_FAILURE;
        }
        if (read_bytes == 0) {
          // end of file
          return EXIT_SUCCESS;
        }
        stdin_read_bytes += read_bytes;
        printf("DEBUG: stdin_read_bytes=%d, stdin_buf=[%u, %u]\n", stdin_read_bytes, stdin_buf[0], stdin_buf[1]);

        if (stdin_read_bytes >= 2) {
          uint16_t in = *((uint16_t *) stdin_buf);
          in = ntohs(in);
          printf("DEBUG: in=%u\n", in);
          stdin_read_bytes = 0;
        }
      }
    }
  }

  return EXIT_SUCCESS;
}

// タイマー初期化
static int initialize_timer() {
  int timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
  if (timerfd == -1) {
    perror("timerfd_create");
    exit(EXIT_FAILURE);
  }
  return timerfd;
}

// メインループ処理のためのepoll初期化
static int initialize_epoll(int timerfd) {
  int ret;

  int epollfd = epoll_create1(0);
  if (epollfd == -1) {
    perror("epoll_create1");
    exit(EXIT_FAILURE);
  }

  // 監視対象fdその1: 標準入力
  struct epoll_event ev1 = {EPOLLIN, {0}};
  ev1.data.fd = STDIN_FILENO;
  ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev1);
  if (ret == -1) {
    perror("epoll_ctl");
    exit(EXIT_FAILURE);
  }

  // 監視対象fdその2: タイマー
  struct epoll_event ev2 = {EPOLLIN, {0}};
  ev1.data.fd = timerfd;
  ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, timerfd, &ev2);
  if (ret == -1) {
    perror("epoll_ctl");
    exit(EXIT_FAILURE);
  }
  return epollfd;
}
