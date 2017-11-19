#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <argp.h>

#include "optparse.h"

static struct argp_option option_definitions[] = {
  { "duty", 'd', "PERCENT", 0, "Duty ratio of tone.", 1 },
  { 0 }
};

static const char *ARGS_DOC =
  "tone OUTPIN\n"
  "noise OUTPIN";

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

    // -d, --duty
    case 'd': {
      int value, ret;
      ret = parse_int(arg, &value);
      if (ret) {
        argp_error(state, "failed to parse duty: [%s]", arg);
        return EINVAL;
      }
      if (value <= 0 || value >= 100) {
        argp_error(state, "duty ratio percentage should be "
                          "in range of (0, 100): %d\n", value);
        return EINVAL;
      }
      dest->dutyratio = (unsigned char) value;
      break;
    }

    // オプションでない引数
    case ARGP_KEY_ARG: {
      switch (state->arg_num) {

        // tone or noise
        case 0: {
          if (strncmp(arg, "tone", sizeof("tone")) == 0) {
            dest->mode = SOUND_MODE_TONE;
          } else if (strncmp(arg, "noise", sizeof("noise")) == 0) {
            dest->mode = SOUND_MODE_NOISE;
          } else {
            argp_error(state, "first argument should be \"tone\" or \"noise\": "
                              "[%s]\n", arg);
            return EINVAL;
          }
          break;
        }

        // OUTPIN
        case 1: {
          int value, ret;
          ret = parse_int(arg, &value);
          if (ret) {
            argp_error(state, "failed to parse outpin: [%s]", arg);
            return EINVAL;
          }
          if (value <= 0) {
            argp_error(state, "pin number should be a positive integer: %d\n", value);
            return EINVAL;
          }
          dest->outpin = value;
          break;
        }

        default: {
          // 引数3個以上はエラー
          return ARGP_ERR_UNKNOWN;
        }
      }
      break;
    }

    // 引数パース終了時に必須引数が指定されたかチェック
    case ARGP_KEY_SUCCESS: {
      if (dest->mode == SOUND_MODE_UNKNOWN) {
        argp_error(state, "neither \"tone\" nor \"noise\" is specified.\n");
        return EINVAL;
      }
      if (dest->outpin == 0) {
        argp_error(state, "OUTPIN is not specified.\n");
        return EINVAL;
      }
      break;
    }

    default: {
      return ARGP_ERR_UNKNOWN;
    }
  }
  return 0;
}

void parse_args(int argc, char **argv, options *opt) {
  // default
  opt->mode = SOUND_MODE_UNKNOWN;
  opt->outpin = 0;
  opt->dutyratio = 50;

  struct argp argp = {option_definitions, parse_opt, ARGS_DOC, 0,
                      NULL, NULL, NULL};
  argp_parse(&argp, argc, argv, 0, 0, opt);
}
