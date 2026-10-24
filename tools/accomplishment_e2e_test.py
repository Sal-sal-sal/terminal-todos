"""Exercise the real TUI and startup preview using disposable persisted data."""
import fcntl
import json
import os
from pathlib import Path
import pty
import select
import struct
import subprocess
import tempfile
import termios
import time
from datetime import date, timedelta

ROOT = Path(__file__).resolve().parents[1]


def drain(fd, seconds=0.3):
    result = b""
    end = time.monotonic() + seconds
    while time.monotonic() < end:
        if select.select([fd], [], [], max(0, end - time.monotonic()))[0]:
            try:
                result += os.read(fd, 65536)
            except OSError:
                break
    return result


def interact(path, actions):
    master, slave = pty.openpty()
    fcntl.ioctl(slave, termios.TIOCSWINSZ, struct.pack("HHHH", 24, 80, 0, 0))
    env = dict(os.environ, TERM="xterm-256color")
    process = subprocess.Popen([str(ROOT / "build/term-todos"), str(path)],
                               stdin=slave, stdout=slave, stderr=slave, env=env)
    os.close(slave)
    output = drain(master, 0.7)
    try:
        for action in actions:
            os.write(master, action.encode())
            output += drain(master)
        process.wait(timeout=5)
        assert process.returncode == 0, output.decode(errors="replace")
    finally:
        if process.poll() is None:
            process.kill()
            process.wait()
        os.close(master)
    return output


def main():
    with tempfile.TemporaryDirectory(prefix="accomplishments-test-") as directory:
        path = Path(directory) / "data.json"
        seed = {"collections": [{"id": 1, "name": "Inbox"}], "tasks": [], "habits": []}
        path.write_text(json.dumps(seed))
        output = interact(path, ["3", "n", "Learn Rust", "\r", " ", "n", "Read a book", "\r", "q"])
        data = json.loads(path.read_text())
        assert data["accomplishments"] == [
            {"title": "Learn Rust", "done": True, "completed_on": date.today().isoformat()},
            {"title": "Read a book", "done": False}]
        assert b"ACCOMPLISHMENTS" in output
        assert "✦".encode() in output, "completion animation did not render"
        assert "✧".encode() in output, "completion animation did not advance"
        assert data["collections"] == seed["collections"]
        env = dict(os.environ, TERM_TODOS_DATA=str(path), TERM_TODO_SPLASH_COLOR="always")
        preview = subprocess.check_output([str(ROOT / "tools/term-todo-splash")], env=env).decode()
        assert "\033[9;92mLearn Rust\033[0m" in preview
        assert "\033[1;37mRead a book\033[0m" in preview
        assert "1/2 completed today" in preview
        assert "ACCOMPLISH COMPLETED" not in preview

        data["accomplishments"][0]["completed_on"] = (date.today() - timedelta(days=1)).isoformat()
        path.write_text(json.dumps(data))
        archived_snapshot = subprocess.check_output([str(ROOT / "build/term-todos-snapshot"), str(path)])
        current_section, archived_section = archived_snapshot.decode().split("ACCOMPLISH COMPLETED", 1)
        assert "Read a book" in current_section and "Learn Rust" not in current_section
        assert "Learn Rust" in archived_section
        archived_preview = subprocess.check_output([str(ROOT / "tools/term-todo-splash")], env=env).decode()
        assert "ACCOMPLISH COMPLETED" in archived_preview
        assert "\033[9;32mLearn Rust\033[0m" in archived_preview
        interact(path, ["3", " ", "e", "\x1b[F", "\x7f" * 10, "Master Rust", "\r", "j", "d", "q"])
        data = json.loads(path.read_text())
        assert data["accomplishments"] == [{"title": "Master Rust", "done": False}], data
        snapshot = subprocess.check_output([str(ROOT / "build/term-todos-snapshot"), str(path)])
        assert b"Master Rust" in snapshot
        env["TERM_TODO_SPLASH_COLOR"] = "never"
        preview = subprocess.check_output([str(ROOT / "tools/term-todo-splash")], env=env).decode()
        assert "[ ]  Master Rust" in preview and "\033[" not in preview
        if os.environ.get("CHECK_INSTALLED_SPLASH"):
            env.pop("TERM_TODO_SPLASH_SHOWN", None)
            env.update(TERM_PROGRAM="kitty", TERM="xterm-256color")
            master, slave = pty.openpty()
            process = subprocess.Popen(["/bin/zsh", "-lic", "exit"],
                                       stdin=slave, stdout=slave, stderr=slave, env=env)
            os.close(slave)
            startup = drain(master, 10).decode(errors="replace")
            process.wait(timeout=5)
            os.close(master)
            assert "ACCOMPLISHMENTS" in startup and "Master Rust" in startup, startup
            assert "HABITS" in startup, startup
            print("PASS: installed preview appears alongside habits in a real Zsh startup")
        print("PASS: real TUI create/toggle/rename/delete, restart persistence, snapshot and preview styles")


if __name__ == "__main__":
    main()
