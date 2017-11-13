#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/timerfd.h>
#include <wiringPi.h>

#include "noise.h"

/**
 * 短周期ノイズか
 */
static int shortfreq = 0;

/**
 * ノイズ生成用レジスタの値
 */
static uint16_t reg = 0x8000;

/**
 * 出力レベルを定期的に切り替えるための時刻を保持する構造体。
 */
typedef struct {
  struct timespec prev;  // 前回の出力レベル切り替え時刻
  float prev_ns_f;  // 前回の出力レベル切り替え時刻のナノ秒部分の小数点以下
  float interval_ns;  // 出力レベルの切り替え間隔: 0の場合は音を停止する。
} noisetime_t;
static noisetime_t noisetime;

// GPIO出力ピン(BCM)
static int outpin;

// タイマーのファイルディスクリプタ
int timerfd;

int init_noise(int outpin_) {
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
    noisetime.interval_ns = 0.0;
  } else {
    int first_note = noisetime.interval_ns == 0.0;
    noisetime.interval_ns = 1000000000.0 / freq;
    printf("DEBUG: setfreq(): first_note=%d, interval_ns = %f\n", first_note, noisetime.interval_ns);
    int ret = clock_gettime(CLOCK_MONOTONIC, &noisetime.prev);
    noisetime.prev_ns_f = 0;
    if (ret == -1) {
      perror("clock_gettime");
      exit(EXIT_FAILURE);
    }
    if (first_note) {
      noise();
    }
  }
}

// GPIO出力レベルを設定する
static inline void setlevel() {
  // ファミコンノイズロジック http://d.hatena.ne.jp/aike/20121224
  reg >>= 1;
  reg |= ((reg ^ (reg >> (shortfreq ? 6 : 1))) & 1) << 15;
  digitalWrite(outpin, reg & 1);
}

void noise() {
  int ret;

  // タイマー値の読み取り
  uint64_t timerval;
  read(timerfd, &timerval, sizeof(timerval));

  if (noisetime.interval_ns == 0.0) {
    printf("DEBUG: noise(): stop tone\n");
      digitalWrite(outpin, 0);
    return;
  }

  setlevel();

  // タイマーが次に発火する時刻を計算する
  noisetime.prev_ns_f += noisetime.interval_ns;
  noisetime.prev.tv_nsec += (long) noisetime.prev_ns_f;
  noisetime.prev_ns_f -= (long) noisetime.prev_ns_f;
  noisetime.prev.tv_sec += noisetime.prev.tv_nsec / 1000000000;
  noisetime.prev.tv_nsec = noisetime.prev.tv_nsec % 1000000000;

  // タイマーをセット
  struct itimerspec timerspec = {{0}, {0}};
  timerspec.it_value = noisetime.prev;
  ret = timerfd_settime(timerfd, TFD_TIMER_ABSTIME, &timerspec, NULL);
  if (ret == -1) {
    perror("timerfd_settime");
    exit(EXIT_FAILURE);
  }
}
