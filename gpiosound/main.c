#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#include "optparse.h"
#include "sound.h"
#include "tone.h"
#include "noise.h"
#include "log.h"

#define MAX_EVENTS 2

static int init_epoll(int timerfd);

int main(int argc, char *argv[]) {
  // 引数パース
  options opt;
  parse_args(argc, argv, &opt);

  printf("outpin=%d, dutyratio=%d\n", opt.outpin, opt.dutyratio);

  sound s;
  switch (opt.mode) {
    case SOUND_MODE_TONE: {
      s = sound_tone;
      break;
    }
    case SOUND_MODE_NOISE: {
      s = sound_noise;
      break;
    }
    case SOUND_MODE_UNKNOWN:  // fall through
    default: {
      printf("unknown play mode.\n");
      return EXIT_FAILURE;
    }
  }

  // ファイルディスクリプタの初期化
  int timerfd = s.init(&opt);
  int epollfd = init_epoll(timerfd);

  // 標準入力をノンブロックに設定する
  int cur = fcntl(STDIN_FILENO, F_GETFL);
  if (cur == -1) {
    perror("fcntl");
    return EXIT_FAILURE;
  }
  int ret = fcntl(STDIN_FILENO, F_SETFL, cur | O_NONBLOCK);
  if (ret == -1) {
    perror("fcntl");
    return EXIT_FAILURE;
  }

  // メインループ
  uint32_t stdin_buf = 0;
  size_t stdin_read_bytes = 0;
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
        s.do_sound();
      }

      // 標準入力: 2バイト読んだ数値を周波数としてセットする
      else if (fd == STDIN_FILENO) {
        ssize_t read_bytes = read(STDIN_FILENO,
                                  ((uint8_t *) &stdin_buf) + stdin_read_bytes,
                                  sizeof(stdin_buf) - stdin_read_bytes);
        if (read_bytes == -1) {
          if (errno == EAGAIN) {
            continue;
          }
          perror("read");
          return EXIT_FAILURE;
        }
        if (read_bytes == 0) {
          // end of file
          printf("exitting by end of file.\n");
          return EXIT_SUCCESS;
        }
        stdin_read_bytes += (size_t) read_bytes;
        debug("stdin_read_bytes=%u, stdin_buf=[%u, %u, %u, %u]\n", stdin_read_bytes,
              ((uint8_t *) &stdin_buf)[0], ((uint8_t *) &stdin_buf)[1],
              ((uint8_t *) &stdin_buf)[2], ((uint8_t *) &stdin_buf)[3]);

        if (stdin_read_bytes >= 4) {
          // floatのバイトオーダー変換
          float in;
          uint32_t converted = ntohl(stdin_buf);
          memcpy(&in, &converted, sizeof(stdin_buf));
          debug("converted=%ul, in=%f\n", converted, (double) in);
          s.setfreq(in);
          stdin_read_bytes = 0;
          stdin_buf = 0;
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
  struct epoll_event ev1 = {EPOLLIN | EPOLLET, {0}};
  ev1.data.fd = STDIN_FILENO;
  ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev1);
  if (ret == -1) {
    perror("epoll_ctl");
    exit(EXIT_FAILURE);
  }

  // 監視対象fdその2: タイマー
  struct epoll_event ev2 = {EPOLLIN | EPOLLET, {0}};
  ev2.data.fd = timerfd;
  ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, timerfd, &ev2);
  if (ret == -1) {
    perror("epoll_ctl");
    exit(EXIT_FAILURE);
  }
  return epollfd;
}
