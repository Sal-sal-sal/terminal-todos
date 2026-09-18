"""Compact accomplishment section shared by the installed shell splash."""
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


def render_accomplishments(items, enabled, width):
    if not isinstance(items, list):
        return []
    items = [item for item in items if isinstance(item, dict) and item.get("title")]
    if not items:
        return []

    def style(text, code):
        return f"\033[{code}m{text}\033[0m" if enabled else text

    done = sum(item.get("done") is True for item in items)
    lines = ["", "  " + style("ACCOMPLISHMENTS", "1;33") + "  " + style(f"{done}/{len(items)} completed", "90")]
    for item in items:
        completed = item.get("done") is True
        title = safe_title(item["title"], max(2, width - 8))
        marker = "✓" if completed else "○"
        if not enabled:
            marker = "[x]" if completed else "[ ]"
        lines.append("  " + style(marker, "32" if completed else "33") + "  " + style(title, "9;90" if completed else "1;37"))
    return lines
