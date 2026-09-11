#include "UI/events/task_events.hpp"

namespace term_todos {

using namespace ftxui;

namespace {

bool handle_collection_event(AppState &state, const Event &event,
                             std::string &input) {
  if (event == Event::ArrowUp || event == Event::Character('k')) {
    state.navigate_collection(-1);
  } else if (event == Event::ArrowDown || event == Event::Character('j')) {
    state.navigate_collection(1);
  } else if (event == Event::Return || event == Event::ArrowRight ||
             event == Event::Character('l')) {
    state.open_selected_collection();
  } else if (event == Event::Character('n')) {
    input.clear();
    state.modal = AppState::ModalKind::AddCollection;
  } else if (event == Event::Character('e')) {
    const auto *collection = state.selected_collection();
    if (collection) {
      input = collection->name;
      state.edit_target_id = collection->id;
      state.modal = AppState::ModalKind::RenameCollection;
    }
  } else if (event == Event::Character('d')) {
    if (state.collections.size() <= 1) {
      state.status_message = "At least one collection is required";
    } else if (state.selected_collection()) {
      state.modal = AppState::ModalKind::DeleteCollection;
    }
  } else if (event == Event::Character('f')) {
    input = state.collection_query;
    state.modal = AppState::ModalKind::SearchCollections;
  } else if (event == Event::Escape) {
  } else {
    return false;
  }
  return true;
}

bool handle_board_event(AppState &state, const Event &event,
                        std::string &input) {
  if (event == Event::Escape || event == Event::Character('b')) {
    state.back_to_collections();

  } else if (event == Event::Escape || event == Event::Character('-')) {
    state.back_to_collections();
  } else if (event == Event::ArrowUp || event == Event::Character('k')) {
    state.navigate_card(-1);
  } else if (event == Event::ArrowDown || event == Event::Character('j')) {
    state.navigate_card(1);
  } else if (event == Event::ArrowLeft || event == Event::Character('h')) {
    state.move_task_left();
  } else if (event == Event::ArrowRight || event == Event::Character('l')) {
    state.move_task_right();
  } else if (event == Event::Character('n')) {
    input.clear();
    state.modal = AppState::ModalKind::AddTask;
  } else if (event == Event::Character('e')) {
    const Task *focused = state.focused_task();
    if (focused) {
      input = focused->title;
      state.edit_target_id = focused->id;
      state.modal = AppState::ModalKind::EditTask;
    }
  } else if (event == Event::Character('d')) {
    state.delete_focused_task();
  } else if (event == Event::Character('[') || event == Event::Character(']')) {
    state.cycle_focused_priority();
  } else if (event == Event::Character('u')) {
    const Task *focused = state.focused_task();
    if (focused) {
      input = focused->due_date;
      state.edit_target_id = focused->id;
      state.modal = AppState::ModalKind::SetDue;
    }
  } else if (event == Event::Character('f')) {
    input = state.search_query;
    state.modal = AppState::ModalKind::Search;
  } else {
    return false;
  }
  return true;
}

} // namespace

bool handle_task_event(AppState &state, const Event &event,
                       std::string &input) {
  return state.task_view == AppState::TaskView::Collections
             ? handle_collection_event(state, event, input)
             : handle_board_event(state, event, input);
}

} // namespace term_todos
