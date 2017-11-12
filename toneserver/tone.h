#ifndef TONE_H__
#define TONE_H__

#include <stdint.h>

/**
 * 音を鳴らすための初期化を行い、定期処理のためのタイマーを返す。
 *
 * @param outpin GPIO出力ピン(BCM)
 * @return タイマーのファイルディスクリプタ
 */
extern int init_tone(int outpin);

/**
 * 鳴らす音の周波数を設定する。
 *
 * 0を設定した場合、音を停止する。
 *
 * @param freq 周波数(Hz)
 */
extern void setfreq(uint16_t freq);

/**
 * GPIOの出力レベルをトグルし、タイマーをセットする。
 */
extern void tone();

#endif /* TONE_H__ */
