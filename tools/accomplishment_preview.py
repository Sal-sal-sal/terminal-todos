"""Compact accomplishment section shared by the installed shell splash."""
from datetime import date
import unicodedata


def safe_title(value, width):
    result = ""
    used = 0
    for char in str(value):
        if unicodedata.category(char).startswith("C"):
            continue
        cells = 0 if unicodedata.combining(char) else (2 if unicodedata.east_asian_width(char) in "WF" else 1)
        if used + cells > width - 1:
            return result + "…"
        result += char
        used += cells
    return result


def archived(item, today):
    if item.get("done") is not True:
        return False
    completed_on = item.get("completed_on")
    return not isinstance(completed_on, str) or not completed_on or completed_on < today


def style(text, code, enabled):
    return f"\033[{code}m{text}\033[0m" if enabled else text


def render_row(item, enabled, width, is_archived):
    completed = item.get("done") is True
    marker = "✓" if completed else "○"
    if not enabled:
        marker = "[x]" if completed else "[ ]"
    title = safe_title(item["title"], max(2, width - 8))
    title_style = "9;32" if is_archived else "9;92" if completed else "1;37"
    return "  " + style(marker, "32" if completed else "33", enabled) \
        + "  " + style(title, title_style, enabled)


def render_accomplishments(items, enabled, width, today=None):
    if not isinstance(items, list):
        return []
    items = [item for item in items if isinstance(item, dict) and item.get("title")]
    if not items:
        return []
    today = today or date.today().isoformat()
    current = [item for item in items if not archived(item, today)]
    completed = [item for item in items if archived(item, today)]
    done_today = sum(item.get("done") is True for item in current)
    lines = ["", "  " + style("ACCOMPLISHMENTS", "1;33", enabled)
             + "  " + style(f"{done_today}/{len(current)} completed today", "90", enabled)]
    lines.extend(render_row(item, enabled, width, False) for item in current)
    if completed:
        lines.extend(("", "  " + style("ACCOMPLISH COMPLETED", "1;32", enabled)
                      + "  " + style(str(len(completed)), "90", enabled)))
        lines.extend(render_row(item, enabled, width, True) for item in completed)
    return lines
