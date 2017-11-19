// テスト用に レ の音の周波数を標準出力に書き込むプログラム

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: %s FREQ\n", argv[0]);
    return 0;
  }
  float freq = (float) atof(argv[1]);  // レの音の周波数
  uint32_t buf;

  memcpy(&buf, &freq, sizeof(float));
  buf = htonl(buf);

  write(1, &buf, sizeof(buf));

  return 0;
}
