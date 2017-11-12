import sys

from musicbox.score import Score


# パッシブブザーに配線されているGPIOピン
PIN = [18, 23]


def usage(progname: str):
    print(f'Usage: {progname} SCORE_FILE')
    exit(0)


def main():
    if len(sys.argv) != 2:
        usage(sys.argv[0])
    score_file = sys.argv[1]

    # スコアファイルの読み込み
    score = Score(score_file)
    print(score)


if __name__ == '__main__':
    main()
