#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#include "optparse.h"
#include "tone.h"

#define MAX_EVENTS 2

static int init_epoll(int timerfd);

int main(int argc, char *argv[]) {
  // 引数パース
  options opt;
  parse_args(argc, argv, &opt);

  printf("outpin=%d\n", opt.outpin);

  // ファイルディスクリプタの初期化
  int timerfd = init_tone(opt.outpin);
  int epollfd = init_epoll(timerfd);

  // メインループ
  uint8_t stdin_buf[2] = {0, 0};
  ssize_t stdin_read_bytes = 0;
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

      // タイマーが発火: tone()を呼ぶ
      if (fd == timerfd) {
        tone(timerfd);
      }

      // 標準入力: 2バイト読んだ数値を周波数としてセットする
      else if (fd == STDIN_FILENO) {
        ssize_t read_bytes = read(STDIN_FILENO,
                                  stdin_buf + stdin_read_bytes,
                                  sizeof(stdin_buf) - stdin_read_bytes);
        if (read_bytes == -1) {
          // epollしてるのでEAGAINは起こらない
          perror("read");
          return EXIT_FAILURE;
        }
        if (read_bytes == 0) {
          // end of file
          printf("exitting by end of file.");
          return EXIT_SUCCESS;
        }
        stdin_read_bytes += read_bytes;

        if (stdin_read_bytes >= 2) {
          uint16_t in = *((uint16_t *) stdin_buf);
          in = ntohs(in);
          setfreq(in);
          stdin_read_bytes = 0;
        }
      }
    }
  }

  return EXIT_SUCCESS;
}

// メインループ処理のためのepoll初期化
static int init_epoll(int timerfd) {
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
