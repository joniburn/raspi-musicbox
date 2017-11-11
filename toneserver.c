#include <stdio.h>
#include <stdlib.h>

#include <limits.h>
#include <errno.h>
#include <argp.h>

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
  // 引数パース
  options opt;
  struct argp argp = {option_definitions, parse_opt, "", "", NULL, NULL, NULL};
  argp_parse(&argp, argc, argv, 0, NULL, &opt);

  printf("outpin=%d\n", opt.outpin);
  return EXIT_SUCCESS;
}
