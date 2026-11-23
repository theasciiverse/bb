#!/usr/bin/env python3
"""Records the catalog preview: BB at 100x30, without its music.

BB draws as fast as the terminal takes its frames, which in a recorder that
never waits means hundreds of frames a second (half a gigabyte in thirty
seconds). So this records the way a terminal sees it: BB runs in a pty,
its output goes through pyte (a terminal emulator), and every 1/25 s the
cells that changed since the last frame are written to an asciicast v3.
Reading through pyte is also what holds BB to a terminal's pace.

BB runs as hub's castrec.py runs titles: TERM=xterm-256color, COLORTERM,
UTF-8, a throwaway HOME and state folder, USER=player, no sound.

  asciiverse/record-preview.py ./bb asciiverse/bb.cast [SCRIPT [START]]
SCRIPT is ;-separated: w<seconds> waits, k<char> sends a key; ends with q.
Nothing before START seconds is written, so the preview can begin after
the "Precalculating data" screen. The defaults are the preview's (PREVIEW
and START below).
  asciiverse/record-preview.py ./bb survey.cast "w900" 0 records it all.
"""
import json, os, pty, select, signal, struct, sys, tempfile, termios, fcntl, time, shutil
import pyte

COLS, ROWS, FPS = 100, 30, 25.0
# a minute of the demo as it plays, from the BB title after the
# precalculation to the first 3D object
PREVIEW = "w87"
START = 28.0

class RepScreen(pyte.Screen):
    last = " "
    def draw(self, data):
        if data:
            self.last = data[-1]
        super().draw(data)
    def rep(self, n=1, *args, **kwargs):  # CSI n b, which ncurses uses
        super().draw(self.last * max(1, n))

class RepStream(pyte.Stream):
    csi = dict(pyte.Stream.csi, b="rep")

def sgr(ch):
    p = ["0"]
    if ch.bold: p.append("1")
    if ch.reverse: p.append("7")
    if ch.underscore: p.append("4")
    # pyte keeps dim as no attribute; BB's dim text stays plain
    return "\x1b[" + ";".join(p) + "m"

def main():
    binary, out = os.path.abspath(sys.argv[1]), os.path.abspath(sys.argv[2])
    script = sys.argv[3] if len(sys.argv) > 3 else PREVIEW
    begin = float(sys.argv[4]) if len(sys.argv) > 4 else START
    steps = []
    t = 0.0
    for s in script.split(";"):
        if s.startswith("w"):
            t += float(s[1:])
        elif s.startswith("k"):
            steps.append((t, s[1:]))
    end_at = t
    home = tempfile.mkdtemp(prefix="bb-rec-")
    env = {"PATH": os.path.dirname(binary) + ":/usr/bin:/bin", "HOME": home,
           "XDG_STATE_HOME": os.path.join(home, "state"), "USER": "player",
           "LOGNAME": "player", "TERM": "xterm-256color", "COLORTERM": "truecolor",
           "LANG": "C.UTF-8", "LC_ALL": "C.UTF-8", "COLUMNS": str(COLS), "LINES": str(ROWS),
           "PULSE_SERVER": "unix:/nonexistent", "PIPEWIRE_REMOTE": "/nonexistent"}
    pid, fd = pty.fork()
    if pid == 0:
        os.chdir(home)
        os.execve(binary, ["bb", "-nosound"], env)
    fcntl.ioctl(fd, termios.TIOCSWINSZ, struct.pack("HHHH", ROWS, COLS, 0, 0))
    os.kill(pid, signal.SIGWINCH)
    scr = RepScreen(COLS, ROWS)
    st = RepStream(scr)
    dec = __import__("codecs").getincrementaldecoder("utf-8")("replace")
    shown = {}
    def frame():
        o, last, cur = [], None, None
        for y in range(ROWS):
            line = scr.buffer[y]
            for x in range(COLS):
                ch = line[x]
                k = (ch.data, ch.bold, ch.reverse, ch.underscore)
                if shown.get((x, y)) != k:
                    shown[(x, y)] = k
                    s = sgr(ch)
                    if cur != (x, y):
                        o.append(f"\x1b[{y + 1};{x + 1}H")
                    if s != last:
                        o.append(s); last = s
                    o.append(ch.data or " ")
                    cur = (x + 1, y) if x + 1 < COLS else None
        return "".join(o)
    events = []
    start = time.monotonic()
    written = 0.0
    next_frame = 0.0
    first = True
    quit_sent = False
    alive = True
    while alive:
        now = time.monotonic() - start
        while steps and now >= steps[0][0]:
            os.write(fd, steps.pop(0)[1].encode())
        if now >= end_at and not quit_sent:
            os.write(fd, b"q"); quit_sent = True
        if quit_sent and now > end_at + 5:
            os.kill(pid, signal.SIGTERM)
        r, _, _ = select.select([fd], [], [], 0.01)
        if r:
            try:
                data = os.read(fd, 65536)
            except OSError:
                data = b""
            if not data:
                alive = False
            else:
                st.feed(dec.decode(data))
        now = time.monotonic() - start
        if now < begin:
            written = begin
            continue
        if now >= next_frame or not alive:
            f = frame()
            if first:
                f = "\x1b[?1049h\x1b[?25l\x1b[2J" + f; first = False
            if f:
                events.append([round(now - written, 3), "o", f])
                written = now
            next_frame = now + 1.0 / FPS
    os.waitpid(pid, 0)
    events.append([0.1, "o", "\x1b[0m\x1b[?25h\x1b[?1049l"])
    head = {"version": 3, "term": {"cols": COLS, "rows": ROWS, "type": "xterm-256color"},
            "timestamp": int(time.time()), "idle_time_limit": 2, "command": "bb -nosound",
            "title": "BB", "env": {"TERM": "xterm-256color", "COLORTERM": "truecolor", "LANG": "C.UTF-8"}}
    with open(out, "w") as f:
        f.write(json.dumps(head) + "\n")
        for e in events:
            f.write(json.dumps(e) + "\n")
    shutil.rmtree(home, ignore_errors=True)
    print(out, os.path.getsize(out) // 1024, "KB,", round(sum(e[0] for e in events), 1), "s")

main()
