# term-todos

A terminal collections, Kanban, and habit tracker built in C++17 with [FTXUI](https://github.com/ArthurSonzogni/ftxui) and [nlohmann/json](https://github.com/nlohmann/json).

Tasks are grouped into collections, and every collection opens its own three-column board (Todo / In Progress / Done).
The app also includes a GitHub-style heatmap for daily habits, create/edit/delete controls, priorities, due dates, search, statistics, and CSV export.
State is persisted to a local JSON file.

```
╭────────────┬────────────┬──────────────────────────────────────╮
│ term-todos │   Habits   │   Tasks                     1/2 tabs │
╰────────────┴────────────┴──────────────────────────────────────╯
12 tasks  -  5 done  (41%)
╔══════════════════════╗╭──────────────────╮╭──────────────────╮
║Todo  4               ║│In Progress  3    ││Done  5           ║
║┃▸ #12 [H] Fix login  ║│   #8 [H] Persist ││   #5    Set up...│
║   #11 [M] Add CSV... ║│   #7 [M] Kanban..││   #4    Define...│
╚══════════════════════╝╰──────────────────╯╰──────────────────╯
```

## Build

Requires CMake 3.14+ and a C++17 compiler. Dependencies are fetched
automatically via FetchContent.

```sh
cmake -S . -B build
cmake --build build
```

The main build targets are:

- `term-todos` - the interactive app.
- `term-todos-snapshot` - renders a fixed-size ASCII frame of each tab for
  non-interactive UI review.
- `term-todos-ui-event-test` - drives real FTXUI keyboard events without a TTY.
- `term-todos-state-test` - data-layer integration test
  (load, mutate, save, reload).
- `term-todos-habit-markup-test` - color-tag and legacy-bracket parser test.
- `term-todos-cli-test` and `term-todos-async-test` - CLI and async regressions.

## Run

```sh
./build/term-todos                 # uses the user data file (see below)
./build/term-todos /path/to/data.json   # use a specific file (load + save)
```

On first run the app seeds its data file from the bundled mock
(`assets/data/mock.json`) so there is something to look at.

The installed `todo` command also supports non-interactive help and task listing:

```sh
todo help
todo -h
todo --help
todo list
todo list /path/to/data.json
```

## Habit heatmap shell splash

`tools/term-todo-splash` prints a compact, read-only habit summary and a 12-week aggregate heatmap above the normal shell prompt.
It reads the same data file as `term-todos` and does not start the interactive interface or modify habit data.

Install the command and enable the splash for new Zsh terminals:

```sh
install -m 0755 tools/term-todo-splash ~/.local/bin/term-todo-splash
install -m 0644 tools/accomplishment_preview.py ~/.local/bin/accomplishment_preview.py
install -m 0644 tools/habit_formula_preview.py ~/.local/bin/habit_formula_preview.py
term-todo-splash install
```

Manage it later with:

```sh
term-todo-splash show
term-todo-splash status
term-todo-splash uninstall
term-todo-splash install
```

The managed Zsh block is independent and appended without changing existing startup art, prompts, or shell behavior.
This lets the habit heatmap coexist with `epic-mode` or another terminal art tool.
Running `term-todo-splash uninstall` removes only the habit block, while `term-todo-splash install` restores it without changing the selected art.

### Where data lives

Resolved in this order:

1. A path passed as the first argument (handy for testing).
2. `$TERM_TODOS_DATA`
3. `$XDG_DATA_HOME/term-todos/data.json`
4. `~/.local/share/term-todos/data.json`

Changes are written back to the same file on quit. Writes are atomic
(temp file + rename), so a crash mid-write never corrupts the data.

## Keybindings

Global:

| Key     | Action                     |
|---------|----------------------------|
| `1`/`2`/`3`/`4` | switch tab             |
| `Tab`   | cycle tab                  |
| `?`     | toggle keybindings overlay |
| `x`     | export habits to CSV       |
| `q`     | quit (saves on exit)       |

Collections:

| Key           | Action                    |
|---------------|---------------------------|
| `j`/`k`       | select collection         |
| Enter/`l`     | open selected collection  |
| `n`           | new collection            |
| `e`           | rename collection         |
| `d`           | delete after confirmation |
| `f`           | find collections          |

Collection board:

| Key     | Action                       |
|---------|------------------------------|
| `h`/`l` | move card, or cross an empty column |
| `j`/`k` | select card up/down          |
| `n`     | new task                     |
| `e`     | edit title                   |
| `D`     | edit or clear description    |
| `d`     | delete                       |
| `s`     | create sub-todo              |
| `J`/`K` | select sub-todo              |
| `space` | toggle selected sub-todo     |
| `X`     | delete selected sub-todo     |
| `[`/`]` | cycle priority               |
| `u`     | set due date                 |
| `f`     | search / filter              |
| `b`/Esc | return to collections        |

Goals (tab `4`) provides a branching map of goals, tasks, nested subtasks, and linked daily habits.
See [Goals guide](docs/goals.md) for controls and saving behavior.

Accomplishments (tab `3`):

Use `n` to add a goal, `j`/`k` or arrows to select, `Space` or `Enter` to toggle completion, `e` to rename, and `d` to delete.
Press `q` to save and exit.
The terminal startup preview includes a gold `ACCOMPLISHMENTS` section with a completion count.
Completed titles turn green and are struck through.
They remain in the current list until the local calendar day ends, then appear under `ACCOMPLISH COMPLETED`.
The app updates this grouping automatically at midnight, and the completed date is saved with each item.
Terminals without ANSI styling show `[x]` and `[ ]` instead.
The section appears after you save your first accomplishment.

Habits:

| Key     | Action            |
|---------|-------------------|
| `j`/`k` | select habit      |
| `space` | toggle today      |
| `n`     | new habit         |
| `e`     | rename            |
| `D`     | edit or clear description |
| Enter   | switch between description and heatmap |
| PgUp / PgDn | scroll the habit details |
| Home / End | jump to the start or end of the details |
| `d`     | delete            |
| `?`     | show keys and color tags |

Habit descriptions are saved with the habit.
The right panel shows a short preview above the heatmap; press Enter for the full description, then Enter again to return to the heatmap.
Habit names wrap in full in the right panel, both above the heatmap and in description view.
Full descriptions have no line limit.
Use PgUp/PgDn to scroll long text, or Home/End to reach its beginning or end.
Press `D` to edit it, or submit a blank description to clear it.

Use color tags in habit names or descriptions:

```text
Read [#orange]one chapter[#end] in [#green]the library[#end]
Review [#red]important notes[#end] at [#blue]8:00[#end]
```

`[#red]`, `[#green]`, `[#orange]`, and `[#blue]` color the following text until another tag or `[#end]`.
The tags are hidden when displayed, including in the terminal startup preview.
Press `?` in the app for the complete keyboard and color guide.

The older positional square-bracket syntax still works in habit names:

```text
i'm [running] in [mountain] at [6:00]
```

The first bracketed part is light red for behavior, the second light green for place, and the third light blue for time.
The surrounding words can be anything, and one or two bracketed parts work too.
The brackets are editing markers and are hidden in the Habits tab and terminal startup preview.
Habits without bracketed parts remain plain text.
For an existing habit, press `e` and wrap the three parts in brackets.

The due-date prompt accepts a full `YYYY-MM-DD`, an integer `N` (days from today), or a blank value to clear it.
Search matches task titles, labels, and collection names case-insensitively.

## Data format

```json
{
  "schema_version": 6,
  "accomplishments": [{ "title": "Finish my project", "done": true,
    "completed_on": "2026-09-30" }],
  "collections": [
    { "id": 1, "name": "Inbox" }
  ],
  "tasks": [
    { "id": 12, "collection_id": 1,
      "title": "Fix login redirect loop",
      "description": "Reproduce with an expired session",
      "sub_todos": [
        { "title": "Add regression test", "done": false }
      ],
      "status": "todo", "labels": ["bug", "auth"],
      "priority": "high", "due": "2026-07-22" }
  ],
  "habits": [
    { "id": 1, "name": "Drink water",
      "description": "Drink a glass [#blue]after waking[#end]",
      "history": { "2026-07-15": true, "2026-07-14": true } }
  ]
}
```

`status` is `todo` / `in_progress` / `done`, and `priority` is `none` / `low` / `medium` / `high`.
Legacy files without `collections` load into an automatic `Inbox` and are written as the current schema version after the next normal save.
Habit history only needs to record days that were completed.

## Project layout

```
src/                 entry point: data path, seed, run loop
backend/
  models/            Collection, Task, Habit, date utilities
  controllers/       AppState and focused collection/task/habit mutations
  store/             JSON codecs, atomic load/save, and CSV export
UI/                  FTXUI views and event handlers for collections, boards, and habits
tools/               snapshot, UI-event, and state integration tests
assets/data/mock.json   bundled seed data
```

The codebase follows a single-source-of-truth rule: view code only reads `AppState`.
All mutations go through its methods, which set a dirty flag so the change is persisted on exit.
