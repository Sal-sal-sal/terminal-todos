"""Verify two deletion confirmations and the typed history-record count."""

import argparse
from datetime import date, timedelta
import json
from pathlib import Path
import re
import subprocess
import tempfile
import time

from habit_autosave_e2e_test import ROOT, seed, send, session
from habit_view_e2e_test import drain


def fixture(path, records=3):
    payload = seed(path)
    payload["habits"][0]["name"] = "Read physics before breakfast"
    payload["habits"][0]["history"] = {
        (date.today() - timedelta(days=day)).isoformat(): True
        for day in range(1, records + 1)
    }
    payload["habits"].append({"id": 8, "name": "Keep this habit", "history": {}})
    path.write_text(json.dumps(payload))
    return payload


def run_case(binary, path, keys):
    with session(binary, path) as (process, master):
        output = send(master, b"1", *keys, b"q")
        deadline = time.monotonic() + 5
        while process.poll() is None and time.monotonic() < deadline:
            output += drain(master, 0.1)
        try:
            process.wait(timeout=1)
        except subprocess.TimeoutExpired:
            plain = re.sub(rb"\x1b\[[0-9;?]*[A-Za-z]", b"", output)
            raise AssertionError(f"TUI did not exit after {keys!r}: {plain[-5000:]!r}")
        assert process.returncode == 0, output.decode(errors="replace")
    return json.loads(path.read_text()), output


def test_cancellation(binary, path):
    cases = (
        (b"d", b"\x1b"),
        (b"d", b"\r", b"\x1b"),
        (b"d", b"\r", b"\r", b"\x1b"),
        (b"d", b"\r", b"99", b"\r", b"\x1b"),
        (b"d", b"dddd", b"\r", b"\r", b"\x1b"),
    )
    for keys in cases:
        before = fixture(path)
        saved, output = run_case(binary, path, keys)
        assert saved == before, "A habit was deleted without both confirmations"
        assert b"Delete habit" in output, "First warning was not shown"
        assert b"3 history records" in output, "Deletion count was not shown"
        if b"\r" in keys:
            assert b"Final confirmation" in output, "Second warning was not shown"


def test_confirmed_deletion(binary, path, records):
    before = fixture(path, records)
    saved, output = run_case(binary, path, (
        b"d", b"\r", str(records).encode(), b"\r",
    ))
    expected = dict(before, habits=before["habits"][1:])
    assert saved == expected, "Confirmed deletion affected unrelated records"
    assert b"Final confirmation" in output
    with session(binary, path) as (_, master):
        restarted = send(master, b"1")
        assert b"Keep this habit" in restarted
        assert b"Read physics" not in restarted, "Deleted habit returned after restart"


def test_empty_list(binary, path):
    before = fixture(path)
    before["habits"] = []
    path.write_text(json.dumps(before))
    saved, output = run_case(binary, path, (b"d",))
    assert saved == before
    assert b"Delete habit" not in output, "Empty list opened a deletion dialog"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", type=Path, default=ROOT / "build/term-todos")
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix="habit-delete-") as directory:
        path = Path(directory) / "data.json"
        test_cancellation(args.binary, path)
        for records in (3, 1, 0):
            test_confirmed_deletion(args.binary, path, records)
        test_empty_list(args.binary, path)
    print("PASS: deletion requires two confirmations and the exact count; Esc cancels safely")


if __name__ == "__main__":
    main()
