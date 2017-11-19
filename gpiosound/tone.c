#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <math.h>
#include <sys/timerfd.h>
#include <wiringPi.h>

#include "tone.h"

/**
 * 出力レベルを定期的に切り替えるための時刻を保持する構造体。
 */
typedef struct {
  struct timespec prev;  // 前回の出力レベル切り替え時刻
  float prev_ns_f;  // 前回の出力レベル切り替え時刻のナノ秒部分の小数点以下
  float interval_high_ns;  // 出力レベルの切り替え間隔: 出力レベルHIGHの保持期間
  float interval_low_ns;  // 出力レベルの切り替え間隔: 出力レベルLOWの保持期間
  int paused;  // 真の場合、音を止める
} tonetime_t;
static tonetime_t tonetime;

/**
 * 現在の出力レベル。0か1の値を取る。
 */
static int level = 0;

// GPIO出力ピン(BCM)
static int outpin;

// デューティ比
static unsigned char dutyratio;

// タイマーのファイルディスクリプタ
int timerfd;

int init_tone(const options *opt) {
  outpin = opt->outpin;
  dutyratio = opt->dutyratio;
  tonetime.paused = 1;

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

  return timerfd;
}

void setfreq(float freq) {
  printf("DEBUG: setfreq(%f)\n", (double) freq);
  if (freq == 0.0f) {
    tonetime.paused = 1;
  } else {
    tonetime.paused = 0;
    float period = 1000000000.0f / freq;
    tonetime.interval_high_ns = period / 100.0f * dutyratio;
    tonetime.interval_low_ns = period - tonetime.interval_high_ns;
    printf("DEBUG: setfreq(): interval_high_ns = %f, interval_low_ns = %f\n",
           (double) tonetime.interval_high_ns,
           (double) tonetime.interval_low_ns);
    int ret = clock_gettime(CLOCK_MONOTONIC, &tonetime.prev);
    tonetime.prev_ns_f = 0;
    if (ret == -1) {
      perror("clock_gettime");
      exit(EXIT_FAILURE);
    }
  }
  tone();
}

// GPIO出力レベルをトグルする
static inline void toggle(void) {
  // printf("%s", level ? "." : "|");
  level = !level;
  digitalWrite(outpin, level ? 1 : 0);
}

void tone(void) {
  int ret;

  // タイマー値の読み取り
  uint64_t timerval;
  read(timerfd, &timerval, sizeof(timerval));

  if (tonetime.paused) {
    printf("DEBUG: tone(): stop tone\n");
    if (level) {
      toggle();
    }
    return;
  }

  toggle();

  // タイマーが次に発火する時刻を計算する
  if (level) {
    tonetime.prev_ns_f += tonetime.interval_high_ns;
  } else {
    tonetime.prev_ns_f += tonetime.interval_low_ns;
  }
  tonetime.prev.tv_nsec += (long) tonetime.prev_ns_f;
  tonetime.prev_ns_f -= floorf(tonetime.prev_ns_f);
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
