import os.path
import sys
from multiprocessing import Process
from subprocess import Popen, PIPE
from struct import pack
from time import sleep
from datetime import datetime

from musicbox.score import Score


# パッシブブザーに配線されているGPIOピン
PIN = [27, 22, 18, 23, 25, 12, 5, 13]
# toneserver実行ファイル
TONESRV_EXEC = os.path.join('..', 'toneserver', 'toneserver')
# noiseserver実行ファイル
NOISESRV_EXEC = os.path.join('..', 'toneserver', 'noiseserver')


def usage(progname: str):
    print(f'Usage: {progname} SCORE_FILE')
    exit(0)


def playtrack(score: Score, track: int):
    exe = TONESRV_EXEC if track != 3 else NOISESRV_EXEC
    with Popen([exe, '-o', f'{PIN[track]}'],
               bufsize=0, stdin=PIPE) as p:
        for freq, nexttime in score.build_timeline(track):
            # print(f'freq={freq}, nexttime={nexttime}')
            p.stdin.write(pack('!f', freq))
            now = datetime.now()
            sleepsec = (nexttime - now).total_seconds()
            if sleepsec > 0:
                sleep(sleepsec)


def main():
    if len(sys.argv) != 2:
        usage(sys.argv[0])
    score_file = sys.argv[1]

    # スコアファイルの読み込み
    score = Score(score_file)

    # トラック数チェック
    if score.ntrack > len(PIN):
        raise Exception('no sufficient output pin is provided.')

    # トラック毎にtoneserverと通信するプロセスをfork
    processes = []
    for track in range(0, score.ntrack):
        p = Process(target=playtrack, args=(score, track))
        p.start()
        processes.append(p)
    for p in processes:
        p.join()
    print('All subprocesses finished.')


if __name__ == '__main__':
    main()
