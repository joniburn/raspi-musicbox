# スコアファイル

from datetime import datetime, timedelta
from typing import List, Tuple

from musicbox.scale import PITCH_FREQ_MAP


def _is_comment_line(line: str) -> bool:
    """コメント行であればTrueを返す。"""
    return line.startswith('#')


class ScoreParseError(Exception):
    """スコアファイルのパース失敗エラー"""
    def __init__(self, msg):
        super().__init__(msg)


class Note:
    """音階や長さを持った1つの音"""

    def __init__(self, note_line: str):
        cmd_args = note_line.split(' ')
        self.pitch = cmd_args[0]       # 音階
        self.step = int(cmd_args[1])   # 音長基準(何分音符か)
        self.nstep = int(cmd_args[2])  # 音長(step何個分か)
        if len(cmd_args) <= 3:
            self.duration = 100        # 音の伸ばし率(何%まで伸ばすか)
        else:
            self.duration = int(cmd_args[3])
        if self.duration <= 0 or self.duration > 100:
            raise ScoreParseError(f'duration value should be in range '
                                  f'(0, 100]: {self.duration}')

    def to_timeline(self, bar: float, t: datetime):
        if self.pitch == 'x':
            freq = 0  # 休符
        else:
            freq = PITCH_FREQ_MAP[self.pitch]
        notelen = bar / self.step * self.nstep
        if self.duration == 100:
            return [(freq, t + timedelta(seconds=notelen))]
        else:
            timeline = []
            notelen_on = notelen * self.duration / 100
            timeline.append((freq, t + timedelta(seconds=notelen_on)))
            timeline.append((0, t + timedelta(seconds=notelen)))
            return timeline


class ScoreBlock:
    """一定のBPMを持ったスコアのブロック"""

    def __init__(self, lines: List[str]):
        self._bpm = 0
        self._notes = []
        self._parse(lines)

    def _parse(self, lines: List[str]):
        bpm_parsed = False

        for line in lines:
            # コメント行と空行はスキップ
            if _is_comment_line(line) or not line:
                continue

            # bpmをパース
            if not bpm_parsed:
                self._parse_bpm(line)
                bpm_parsed = True
                continue

            self._notes.append(Note(line))

    def _parse_bpm(self, bpm_line: str):
        bpm_line_args = bpm_line.split(' ')
        if bpm_line_args[0] != 'bpm':
            raise ScoreParseError('no bpm line is provided at beggining '
                                  'of score block.')
        self._bpm = int(bpm_line_args[1])

    def build_timeline(self, start: datetime) -> List[Tuple[float, datetime]]:
        # bpmから各音符の長さを計算
        bar = 60.0 / self._bpm * 4  # 1小節の長さ。4分音符はこの長さの1/4, ...

        timeline = []
        cur = start
        for note in self._notes:
            items = note.to_timeline(bar, cur)
            for i in items:
                timeline.append(i)
            cur = items[-1][1]
        return timeline


class Score:
    """スコアファイルのパース結果を保持するクラス"""

    def __init__(self, fname):
        self._ntrack = 0
        self._parse(fname)

    def _parse(self, fname):
        # プロパティセクションとスコア本体の行を読み込む
        prop_section_lines = []
        score_section_lines = []
        with open(fname) as f:
            for line in f:
                line = line.rstrip()
                if line == '===':
                    break
                prop_section_lines.append(line)
            for line in f:
                line = line.rstrip()
                score_section_lines.append(line)

        # プロパティのパース
        for l in prop_section_lines:
            self._parse_prop_line(l)
        if not self._ntrack:
            raise ScoreParseError('no ntrack is provided.')

        # スコア本体のパース
        # 現在はブロック数1のみ
        self._score_blocks = [ScoreBlock(score_section_lines)]

    def _parse_prop_line(self, line: str):
        if _is_comment_line(line):
            return

        cmd_args = line.split(' ')
        if cmd_args[0] == 'ntrack':
            self._ntrack = int(cmd_args[1])

    def build_timeline(self):
        starttime = datetime.now()

        return self._score_blocks[0].build_timeline(starttime)
