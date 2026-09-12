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

Four targets are produced:

- `term-todos` - the interactive app.
- `term-todos-snapshot` - renders a fixed-size ASCII frame of each tab for
  non-interactive UI review.
- `term-todos-ui-event-test` - drives real FTXUI keyboard events without a TTY.
- `term-todos-state-test` - data-layer integration test
  (load, mutate, save, reload).

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
| `1`/`2` | switch tab                 |
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
| `d`     | delete                       |
| `[`/`]` | cycle priority               |
| `u`     | set due date                 |
| `f`     | search / filter              |
| `b`/Esc | return to collections        |

Habits:

| Key     | Action            |
|---------|-------------------|
| `j`/`k` | select habit      |
| `space` | toggle today      |
| `n`     | new habit         |
| `e`     | rename            |
| `d`     | delete            |

The due-date prompt accepts a full `YYYY-MM-DD`, an integer `N` (days from today), or a blank value to clear it.
Search matches task titles, labels, and collection names case-insensitively.

## Data format

```json
{
  "schema_version": 2,
  "collections": [
    { "id": 1, "name": "Inbox" }
  ],
  "tasks": [
    { "id": 12, "collection_id": 1,
      "title": "Fix login redirect loop",
      "status": "todo", "labels": ["bug", "auth"],
      "priority": "high", "due": "2026-07-22" }
  ],
  "habits": [
    { "id": 1, "name": "Drink water",
      "history": { "2026-07-15": true, "2026-07-14": true } }
  ]
}
```

`status` is `todo` / `in_progress` / `done`, and `priority` is `none` / `low` / `medium` / `high`.
Legacy files without `collections` load into an automatic `Inbox` and are written as schema version 2 after the next normal save.
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
