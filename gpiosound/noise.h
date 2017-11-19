#ifndef NOISE_H__
#define NOISE_H__

/**
 * ノイズ音を鳴らすための初期化を行い、定期処理のためのタイマーを返す。
 *
 * @param outpin GPIO出力ピン(BCM)
 * @return タイマーのファイルディスクリプタ
 */
extern int init_noise(int outpin);

/**
 * 鳴らすノイズ音の周波数を設定する。
 *
 * 0を設定した場合、ノイズ音を停止する。
 *
 * @param freq 周波数(Hz)
 */
extern void setfreq(float freq);

/**
 * GPIOの出力レベルをトグルし、タイマーをセットする。
 */
extern void noise();

#endif /* NOISE_H__ */
