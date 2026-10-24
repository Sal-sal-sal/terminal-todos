"""Check that wrapped names keep the heatmap reachable and preserve streak data."""

from datetime import date, timedelta
import fcntl
import json
import os
from pathlib import Path
import pty
import re
import select
import struct
import subprocess
import tempfile
import termios
import time


ROOT = Path(__file__).resolve().parents[1]


def drain(fd, seconds):
    output = b""
    deadline = time.monotonic() + seconds
    while time.monotonic() < deadline:
        if not select.select([fd], [], [], max(0, deadline - time.monotonic()))[0]:
            break
        try:
            output += os.read(fd, 65536)
        except OSError:
            break
    return output


def screen_output(path, columns=90, actions=(b"1", b"q"), *, raw=False, truecolor=False,
                  rows=26):
    master, slave = pty.openpty()
    fcntl.ioctl(slave, termios.TIOCSWINSZ, struct.pack("HHHH", rows, columns, 0, 0))
    env = dict(os.environ, TERM="xterm-256color")
    if truecolor:
        env.pop("NO_COLOR", None)
        env["COLORTERM"] = "truecolor"
    process = subprocess.Popen(
        [str(ROOT / "build/term-todos"), str(path)],
        stdin=slave, stdout=slave, stderr=slave,
        env=env,
    )
    os.close(slave)
    output = drain(master, 0.5)
    try:
        for key in actions:
            os.write(master, key)
            output += drain(master, 0.4)
        deadline = time.monotonic() + 5
        while process.poll() is None and time.monotonic() < deadline:
            output += drain(master, 0.1)
        process.wait(timeout=1)
        assert process.returncode == 0, output.decode(errors="replace")
    finally:
        if process.poll() is None:
            process.kill()
            process.wait()
        os.close(master)
    return output if raw else re.sub(rb"\x1b\[[0-9;?]*[A-Za-z]", b"", output)


def main():
    today = date.today()
    history = {day.isoformat(): True for day in
               (today - timedelta(days=1), today)}
    habits = [{
        "id": 1,
        "name": "I'm [Reading] in [My Room] at [8:00] " + "practice " * 12,
        "history": history,
    }]
    habits.extend({"id": index, "name": "Long habit " + "work " * 16, "history": {}}
                  for index in range(2, 14))
    with tempfile.TemporaryDirectory(prefix="term-todos-habits-") as directory:
        path = Path(directory) / "data.json"
        seed = {"collections": [{"id": 1, "name": "Inbox"}],
                "tasks": [], "habits": habits}
        path.write_text(json.dumps(seed))
        before = json.loads(path.read_text())
        for columns in (80, 90):
            output = screen_output(path, columns, (b"1", b"q"))
            assert b"2 day streak" in output and b"best streak: 2d" in output
            assert b"I'm Reading in My Room" in output, "highlighted name was mangled"
            scrolled = screen_output(path, columns, (b"1", b"\x1b[F", b"q"))
            assert all(word in scrolled for word in (b"Mon", b"Wed", b"Fri", b"Sun", b"Less")), \
                f"heatmap cannot be reached below a long title at {columns} columns"
        assert json.loads(path.read_text()) == before, "viewing habits changed history"
        screen_output(path, 80, (b"1", b" ", b"q"))
        toggled = json.loads(path.read_text())["habits"]
        assert today.isoformat() not in toggled[0]["history"]
        assert toggled[0]["history"][(today - timedelta(days=1)).isoformat()] is True
        screen_output(path, 80, (b"1", b" ", b"q"))
        restored = json.loads(path.read_text())["habits"]
        assert [habit["history"] for habit in restored] == [habit["history"] for habit in habits]
    print("PASS: heatmap reachable at 80/90 columns; names, toggles, and streak history intact")


if __name__ == "__main__":
    main()
