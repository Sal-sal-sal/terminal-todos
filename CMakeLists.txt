cmake_minimum_required(VERSION 3.14)

project(term-todos
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# --- Dependencies -----------------------------------------------------------

include(FetchContent)

# FTXUI - terminal UI components.
FetchContent_Declare(
    ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui.git
    GIT_TAG        v5.0.0
)
FetchContent_MakeAvailable(ftxui)

# nlohmann/json - header-only JSON.
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
)
FetchContent_MakeAvailable(json)

# --- App --------------------------------------------------------------------

# Path to the bundled mock data, resolved at build time relative to the source
# tree so the binary works regardless of the current working directory.
file(REAL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/assets/data" TERM_TODOS_DATA_DIR)

# Shared sources for every target that renders the UI.
set(TERM_TODOS_SOURCES
    backend/core/async_runner.cpp
    backend/models/task.cpp
    backend/models/habit.cpp
    backend/models/date.cpp
    backend/controllers/app_state.cpp
    backend/controllers/collection_state.cpp
    backend/controllers/task_state.cpp
    backend/controllers/habit_state.cpp
    backend/store/json_store.cpp
    backend/store/json_codecs.cpp

    UI/app_view.cpp
    UI/event_handler.cpp
    UI/events/task_events.cpp
    UI/events/habit_events.cpp
    UI/overlay.cpp
    UI/tab_bar.cpp
    UI/theme.cpp
    UI/tasks/board_view.cpp
    UI/tasks/collection_list.cpp
    UI/tasks/task_card.cpp
    UI/habits/habits_view.cpp
    UI/habits/heatmap.cpp
)

# Convenience function: same include dirs, compile defs, libs, and warnings for
# every term-todos* target.
function(term_todos_target_common name)
    target_include_directories(${name} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_compile_definitions(${name} PRIVATE TERM_TODOS_DATA_DIR="${TERM_TODOS_DATA_DIR}")
    target_link_libraries(${name} PRIVATE ftxui::component nlohmann_json::nlohmann_json)
    if(MSVC)
        target_compile_options(${name} PRIVATE /W4)
    else()
        target_compile_options(${name} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

add_executable(term-todos src/main.cpp src/cli.cpp ${TERM_TODOS_SOURCES})
term_todos_target_common(term-todos)

# --- Command-line parser and list-output regression test --------------------

add_executable(term-todos-cli-test
    tools/cli_test.cpp
    src/cli.cpp
    backend/models/task.cpp
)
term_todos_target_common(term-todos-cli-test)

# --- Non-interactive render harness (pixel-perfect UI review) -----------------

add_executable(term-todos-snapshot tools/snapshot.cpp ${TERM_TODOS_SOURCES})
term_todos_target_common(term-todos-snapshot)

# --- Deterministic keyboard/modal regression test --------------------------

add_executable(term-todos-ui-event-test tools/ui_event_test.cpp ${TERM_TODOS_SOURCES})
term_todos_target_common(term-todos-ui-event-test)

# --- Async, timeout, and error handling regression test -------------------

add_executable(term-todos-async-test tools/async_error_test.cpp ${TERM_TODOS_SOURCES})
term_todos_target_common(term-todos-async-test)

# --- Data-layer integration test --------------------------------------------

add_executable(term-todos-state-test
    tools/state_test.cpp

    backend/core/async_runner.cpp
    backend/models/task.cpp
    backend/models/habit.cpp
    backend/models/date.cpp
    backend/controllers/app_state.cpp
    backend/controllers/collection_state.cpp
    backend/controllers/task_state.cpp
    backend/controllers/habit_state.cpp
    backend/store/json_store.cpp
    backend/store/json_codecs.cpp
)
term_todos_target_common(term-todos-state-test)
