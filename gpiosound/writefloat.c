// テスト用に レ の音の周波数を標準出力に書き込むプログラム

#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

int main(void) {
  float freq = 587.3295515f;  // レの音の周波数
  uint32_t buf;

  memcpy(&buf, &freq, sizeof(float));
  buf = htonl(buf);

  write(1, &buf, sizeof(buf));

  return 0;
}
