# Goals

Press `4` or cycle with Tab to open the Goals map.
Each root goal branches into tasks, and every task can have further subtasks without a fixed depth limit.
The map uses connection lines, wrapping titles, a scrolling selection, and a description preview.
Completed nodes turn green.

```text
[ ] GOAL Become healthier
├─ [ ] Improve fitness
│  ├─ [ ] Plan a workout
│  └─ [ ] Run every morning
└─ [ ] Improve nutrition
```

| Key | Action |
| --- | --- |
| `n` | Create a root goal |
| `a` / `s` | Add a child task to the selected goal or task |
| `N` | Create a new daily habit below the selected node |
| `H` | Link an existing habit below the selected node |
| `j` / `k`, arrows | Select the next or previous visible node |
| Enter | Fold or unfold a branch |
| `h` / Left | Collapse a branch, or move to its parent |
| `l` / Right | Expand a branch |
| Space | Toggle task completion or today's linked habit completion |
| `e` | Rename a node |
| `D` | Edit or clear its description |
| `d` | Review deletion, then type the exact branch record count |
| `?` | Show the Goals keyboard guide |
| Esc | Cancel an open dialog without changes |

In the habit picker, select with Up/Down or PgUp/PgDn and press Enter.
You can also type a habit ID directly.
New daily habits also appear in Habits.
Existing links share the habit's daily history and streak, rather than copying them.
Task completion is independent of parent and child completion.

Goal edits and linked habit toggles save immediately using the existing atomic JSON writer.
Creating a daily habit and its goal link is one save operation.
Save failures leave changes in memory, show an error, and retry on the next change or clean exit with `q`.
Bundled read-only data is never saved.
Legacy JSON files without Goals remain supported.
The optional `goals` array stores flat records with stable IDs and parent IDs.
Invalid cycles, duplicate IDs, or missing parents fail loading instead of silently dropping branches.

Deleting a branch requires a review, Enter, and the exact number of affected goal/task records.
Linked habits and their history are not deleted with the branch.
Deleting a habit from Habits converts its goal links to ordinary tasks, retaining that day's completion state.

Run the dedicated checks with:

```sh
./build/term-todos-goals-test
PYTHONDONTWRITEBYTECODE=1 python3 tools/goals_e2e_test.py
```
