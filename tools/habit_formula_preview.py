"""Render explicit habit color tags and older positional brackets in the splash."""

from __future__ import annotations

import re
import unicodedata

LEGACY_COLORS = ("1;91", "1;92", "1;94")
TAG_COLORS = {
    "red": "1;91",
    "green": "1;92",
    "orange": "1;38;2;255;166;74",
    "blue": "1;94",
    "end": None,
}
TAG = re.compile(r"\[#(red|green|orange|blue|end)\]")


def explicit_parts(name: str) -> list[tuple[str, str | None]] | None:
    matches = list(TAG.finditer(name))
    if not matches:
        return None
    parts = []
    cursor = 0
    active = None
    for match in matches:
        parts.append((name[cursor:match.start()], active))
        active = TAG_COLORS[match.group(1)]
        cursor = match.end()
    parts.append((name[cursor:], active))
    return parts


def marked_parts(name: str) -> list[tuple[str, str | None]]:
    explicit = explicit_parts(name)
    if explicit is not None:
        return explicit
    if "[#" in name:
        return [(name, None)]
    cursor = 0
    segments = []
    for color in LEGACY_COLORS:
        opening = name.find("[", cursor)
        if opening < 0:
            break
        closing = name.find("]", opening + 1)
        nested = name.find("[", opening + 1)
        if closing <= opening + 1 or 0 <= nested < closing:
            break
        segments.extend(((name[cursor:opening], None), (name[opening + 1:closing], color)))
        cursor = closing + 1
    if segments:
        segments.append((name[cursor:], None))
    return segments or [(name, None)]


def cell_width(char: str) -> int:
    if unicodedata.category(char).startswith("C") or unicodedata.combining(char):
        return 0
    return 2 if unicodedata.east_asian_width(char) in "WF" else 1


def clip_cells(value: str, width: int) -> tuple[str, int]:
    result = ""
    used = 0
    for char in value:
        cells = cell_width(char)
        if used + cells > width:
            break
        if not unicodedata.category(char).startswith("C"):
            result += char
        used += cells
    return result, used


def render_habit_name(name: str, enabled: bool, width: int) -> str:
    width = max(0, width)
    segments = marked_parts(name)
    visible_length = sum(sum(cell_width(char) for char in part) for part, _ in segments)
    remaining = width - 1 if visible_length > width else width
    rendered = []
    for part, code in segments:
        if remaining <= 0:
            break
        clipped, used = clip_cells(part, remaining)
        if clipped:
            rendered.append(f"\033[{code}m{clipped}\033[0m" if code and enabled else clipped)
        remaining -= used
    if visible_length > width and width:
        rendered.append("…")
    return "".join(rendered)
