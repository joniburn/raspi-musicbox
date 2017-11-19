#ifndef TONE_H__
#define TONE_H__

#include "optparse.h"

/**
 * 音を鳴らすための初期化を行い、定期処理のためのタイマーを返す。
 *
 * @param options コマンドライン引数
 * @return タイマーのファイルディスクリプタ
 */
extern int init_tone(const options *options);

/**
 * 鳴らす音の周波数を設定する。
 *
 * 0を設定した場合、音を停止する。
 *
 * @param freq 周波数(Hz)
 */
extern void setfreq(float freq);

/**
 * GPIOの出力レベルをトグルし、タイマーをセットする。
 */
extern void tone(void);

#endif /* TONE_H__ */
