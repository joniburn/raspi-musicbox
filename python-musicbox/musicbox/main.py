import os.path
import sys
from subprocess import Popen, PIPE, TimeoutExpired
from struct import pack
from time import sleep
from datetime import datetime

from musicbox.score import Score


# パッシブブザーに配線されているGPIOピン
PIN = [18, 23]
# toneserver実行ファイル
TONESRV_EXEC = os.path.join('..', 'toneserver', 'toneserver')


def usage(progname: str):
    print(f'Usage: {progname} SCORE_FILE')
    exit(0)


def main():
    if len(sys.argv) != 2:
        usage(sys.argv[0])
    score_file = sys.argv[1]

    # スコアファイルの読み込み
    score = Score(score_file)

    with Popen([TONESRV_EXEC, '-o', f'{PIN[0]}'],
               bufsize=0, stdin=PIPE) as p:
        for freq, nexttime in score.build_timeline():
            print(f'freq={freq}, nexttime={nexttime}')
            p.stdin.write(pack('!f', freq))
            now = datetime.now()
            sleepsec = (nexttime - now).total_seconds()
            if sleepsec > 0:
                sleep(sleepsec)


if __name__ == '__main__':
    main()
