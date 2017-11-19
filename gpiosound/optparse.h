#ifndef OPTPARSE_H__
#define OPTPARSE_H__

// コマンドライン引数のパース結果を格納する構造体
typedef struct {
  int outpin;
  int dutyratio;
} options;

/**
 * コマンドライン引数をパースする。
 *
 * 引数に不備がある場合はエラーを出力してプログラムを終了する。
 *
 * @param argc mainに渡されるargc
 * @param argv mainに渡されるargv
 * @param opt  パース結果の格納先
 */
extern void parse_args(int argc, char **argv, options *opt);

#endif /* OPTPARSE_H__ */
