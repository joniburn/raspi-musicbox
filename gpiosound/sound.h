#ifndef SOUND_H__
#define SOUND_H__

#include "optparse.h"

// 音を出す処理のハンドラ定義
typedef struct {
  int (*init)(const options *opt);  // 初期化処理
  void (*setfreq)(float freq);  // 周波数セット処理
  void (*do_sound)(void);  // 音出力処理
} sound;

#endif /* SOUND_H__ */
