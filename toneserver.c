#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <limits.h>
#include <errno.h>
#include <argp.h>

#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#define MAX_EVENTS 2

typedef struct {
  int outpin;
} options;

static struct argp_option option_definitions[] = {
  { "outpin", 'o', "BCM", 0, "The output pin number(BCM).", 0 },
  { 0 }
};

/**
 * 文字列を整数にパースする。
 *
 * @param in  入力文字列
 * @param out 結果の書き込み先
 * @return
 *   正しくパースできた場合、0。
 *   エラーの場合、1。
 */
static int parse_int(const char *in, int *out) {
  char *endptr;
  int ret;

  // 空文字列はエラー
  if (*in == '\0') {
    fprintf(stderr, "empty string could not be parsed into a number.\n");
    return 1;
  }

  ret = strtol(in, &endptr, 10);

  // 整数としてパースできなかった
  if (*endptr != '\0') {
    fprintf(stderr, "number parse error: [%s]\n", in);
    return 1;
  }

  // 値が範囲外
  if (errno == ERANGE || ret > INT_MAX || ret < INT_MIN) {
    fprintf(stderr, "number range error: [%s]\n", in);
    return 1;
  }

  *out = ret;
  return 0;
}

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  options *dest = state->input;

  switch (key) {
  case 'o': {
    int value, ret;
    ret = parse_int(arg, &value);
    if (ret) {
      argp_error(state, "failed to parse outpin: [%s]", arg);
    }
    if (value <= 0) {
      argp_error(state, "pin number should be a positive integer: %d\n", value);
    }
    dest->outpin = value;
    break;
  }
  case ARGP_KEY_ARG:
    return 0;
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

int main(int argc, char *argv[]) {
  int ret;

  // 引数パース
  options opt;
  struct argp argp = {option_definitions, parse_opt, "", "", NULL, NULL, NULL};
  argp_parse(&argp, argc, argv, 0, NULL, &opt);

  printf("outpin=%d\n", opt.outpin);

  // タイマー初期化
  int timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
  if (timerfd == -1) {
    perror("timerfd_create");
    return EXIT_FAILURE;
  }

  // メインループ処理のためのepoll初期化
  int epollfd = epoll_create1(0);
  if (epollfd == -1) {
    perror("epoll_create1");
    return EXIT_FAILURE;
  }
  struct epoll_event ev1 = {EPOLLIN, {0}};
  ev1.data.fd = STDIN_FILENO;
  ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev1);
  if (ret == -1) {
    perror("epoll_ctl");
    return EXIT_FAILURE;
  }
  struct epoll_event ev2 = {EPOLLIN, {0}};
  ev1.data.fd = timerfd;
  ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, timerfd, &ev2);
  if (ret == -1) {
    perror("epoll_ctl");
    return EXIT_FAILURE;
  }

  // メインループ
  uint8_t stdin_buf[2] = {0, 0};
  int stdin_read_bytes = 0;
  while (1) {
    struct epoll_event events[MAX_EVENTS];
    int nfds;

    nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
    if (ret == -1) {
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
          exit(EXIT_SUCCESS);
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
