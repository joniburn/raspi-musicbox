#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/timerfd.h>
#include <wiringPi.h>

#include "tone.h"

/**
 * 鳴らす音の周波数。
 * 0の場合は音を停止する。
 */
static uint16_t freq = 0;

/**
 * 現在の出力レベル。0か1の値を取る。
 */
static int level = 0;

int init_tone(int outpin) {
  // タイマー初期化
  int timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
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

void setfreq(uint16_t infreq) {
  freq = infreq;
}

void tone(int timerfd) {
  // TODO
  (void)timerfd;
  (void)level;
}
