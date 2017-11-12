#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/timerfd.h>
#include <wiringPi.h>

#include "tone.h"

/**
 * 出力レベルを定期的に切り替えるための時刻を保持する構造体。
 */
typedef struct {
  struct timespec prev;  // 前回の出力レベル切り替え時刻
  float prev_ns_f;  // 前回の出力レベル切り替え時刻のナノ秒部分の小数点以下
  float interval_ns;  // 出力レベルの切り替え間隔: 0の場合は音を停止する。
} tonetime_t;
static tonetime_t tonetime;

/**
 * 現在の出力レベル。0か1の値を取る。
 */
static int level = 0;

// GPIO出力ピン(BCM)
static int outpin;

// タイマーのファイルディスクリプタ
int timerfd;

int init_tone(int outpin_) {
  // タイマー初期化
  timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
  if (timerfd == -1) {
    perror("timerfd_create");
    exit(EXIT_FAILURE);
  }

  // GPIO初期化
  int ret = wiringPiSetupGpio();
  if (ret == -1) {
    perror("wiringPiSetupGpio");
    exit(EXIT_FAILURE);
  }
  pinMode(outpin, OUTPUT);
  outpin = outpin_;

  return timerfd;
}

void setfreq(float freq) {
  printf("DEBUG: setfreq(%f)\n", freq);
  if (freq == 0.0) {
    tonetime.interval_ns = 0.0;
  } else {
    int first_note = tonetime.interval_ns == 0.0;
    tonetime.interval_ns = 1000000000.0 / freq;
    printf("DEBUG: setfreq(): first_note=%d, interval_ns = %f\n", first_note, tonetime.interval_ns);
    int ret = clock_gettime(CLOCK_MONOTONIC, &tonetime.prev);
    tonetime.prev_ns_f = 0;
    if (ret == -1) {
      perror("clock_gettime");
      exit(EXIT_FAILURE);
    }
    if (first_note) {
      tone();
    }
  }
}

// GPIO出力レベルをトグルする
static inline void toggle() {
  // printf("%s", level ? "." : "|");
  level = !level;
  digitalWrite(outpin, level ? 1 : 0);
}

void tone() {
  int ret;

  // タイマー値の読み取り
  uint64_t timerval;
  read(timerfd, &timerval, sizeof(timerval));

  if (tonetime.interval_ns == 0.0) {
    printf("DEBUG: tone(): stop tone\n");
    if (level) {
      toggle();
    }
    return;
  }

  toggle();

  // タイマーが次に発火する時刻を計算する
  tonetime.prev_ns_f += tonetime.interval_ns;
  tonetime.prev.tv_nsec += (long) tonetime.prev_ns_f;
  tonetime.prev_ns_f -= (long) tonetime.prev_ns_f;
  tonetime.prev.tv_sec += tonetime.prev.tv_nsec / 1000000000;
  tonetime.prev.tv_nsec = tonetime.prev.tv_nsec % 1000000000;

  // タイマーをセット
  struct itimerspec timerspec = {{0}, {0}};
  timerspec.it_value = tonetime.prev;
  ret = timerfd_settime(timerfd, TFD_TIMER_ABSTIME, &timerspec, NULL);
  if (ret == -1) {
    perror("timerfd_settime");
    exit(EXIT_FAILURE);
  }
}
