#include "include/vimktor.h"
#include "include/common.h"
#include "include/input_manager.h"
#include "include/sequence.h"
#include "include/vimktor_debug.h"
#include <cassert>
#include <cstdio>
#include <cwchar>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

#ifdef PLATFORM_WINDOWS
#include "pcl/pcl.h"
#elif PLATFORM_LINUX
#include <curses.h>
#endif

const unsigned int HELPER_HEIGHT = 2;
const unsigned int LINE_NUM_WIDTH = 5;

void Vimktor::Init() {
  InitCurses();
  unsigned int w, h;
#ifdef PLATFORM_WINDOWS
  getsize(console, &w, &h);
#elif PLATFORM_LINUX
  getmaxyx(m_window, h, w);
#endif
  m_sequence.SetPageDimensions(w - LINE_NUM_WIDTH, h - HELPER_HEIGHT);
  Debug::Log(std::format("max win w: {} , h{}", w, h));
#ifdef PLATFORM_WINDOWS
  refreshascii(console, m_window);
#elif PLATFORM_LINUX
  wrefresh(m_window);
#endif
}

void Vimktor::End() {
#ifdef PLATFORM_WINDOWS
  end(console);
#elif PLATFORM_LINUX
  endwin();
#endif
}

VimktorErr_t Vimktor::InitCurses() {
#ifdef PLATFORM_WINDOWS
  console = start();
  m_window = initascii(console);
  setinputblock(console, FALSE);
#elif PLATFORM_LINUX
  initscr();
  m_window = stdscr;
  keypad(m_window, TRUE);
  raw();
  nonl();
  set_escdelay(50);
  nodelay(stdscr, false);
  noecho();
  curs_set(1);
  init_color(COLOR, 0, 0, 0);
#endif
  return VIMKTOR_OK;
}

VimktorErr_t Vimktor::RenderWindow() {
  const auto txtDim = m_sequence.GetPageDimensions();
  if (txtDim.x == 0 || txtDim.y == 0)
    return MEMORY_ERROR;

  RenderText(LINE_NUM_WIDTH, 0, txtDim.x, txtDim.y);
  RenderLineNumber();
  RenderHelper();
  RenderCursor();

#ifdef PLATFORM_WINDOWS
  refreshascii(console, m_window);
#elif PLATFORM_LINUX
  wrefresh(m_window);
#endif

  return VIMKTOR_OK;
}

VimktorErr_t Vimktor::RenderLineNumber() {
  size_t first_nr = m_sequence.m_pagePos.y;
  size_t height = m_sequence.GetPageDimensions().y;
  for (unsigned int y = 0; y < height; y++) {
    if (first_nr + y >= m_sequence.Size()) {
#ifdef PLATFORM_WINDOWS
      setstringcursorascii(m_window, (char*)"    ", y, 1);
#elif PLATFORM_LINUX
      mvwprintw(m_window, y, 1, "    ");
#endif
    } else {
#ifdef PLATFORM_WINDOWS
      setstringcursorascii(m_window, (char*)"    ", y, 1);
      setstringformattedcursorascii(m_window, y, 1, "%ud", first_nr + y + 1);
#elif PLATFORM_LINUX
      mvwprintw(m_window, y, 1, "%u", first_nr + y + 1);
#endif
    }

    return VIMKTOR_OK;
  }
}

VimktorErr_t Vimktor::RenderCursor() {
  position_t cursor = m_sequence.GetRelativeCursorPos();

#ifdef PLATFORM_WINDOWS
  // todo implement
#elif PLATFORM_LINUX
  wmove(m_window, cursor.y, cursor.x + LINE_NUM_WIDTH);
#endif

  return VIMKTOR_OK;
}

VimktorErr_t Vimktor::RenderHelper() {
  position_t endPoint = GetEditorDimensions();
  position_t cursorPos = m_sequence.GetCursorPos();
  size_t x = endPoint.x - 6;
  size_t y = endPoint.y - HELPER_HEIGHT;
  // print cursor position
  // todo fix
  //Debug::Log(std::format("endpoint {}  x: {}", (std::string)endPoint, x));

#ifdef PLATFORM_WINDOWS
  setstringcursorascii(m_window,  "     ", y, x);
  setstringformattedcursorascii(m_window, y, x, "%ud:%d", cursorPos.y, cursorPos.x);
  setstringformattedcursorascii(m_window, y, 1, "%s  %s lines: %ud ", GetModeStr().c_str(), m_filename.c_str(), m_sequence.Size());
#elif PLATFORM_LINUX
  mvwprintw(m_window, y, x, "     ");

  mvwprintw(m_window, y, x, "%u:%d", cursorPos.y, cursorPos.x);

  // print activ mode and file name
  mvwprintw(m_window, y, 1, "%s  %s lines: %u ", GetModeStr().c_str(),
            m_filename.c_str(), m_sequence.Size());
#endif


  return VIMKTOR_OK;
}

VimktorErr_t Vimktor::RenderText(uint16_t x, uint16_t y, uint16_t width,
                                 uint16_t height) {

  for (uint16_t i_y = y; i_y < height; i_y++) {
    for (uint16_t i_x = x; i_x < width; i_x++) {

#ifdef PLATFORM_WINDOWS
#elif PLATFORM_LINUX
      wmove(m_window, i_y, i_x);
#endif

      char place = ' ';
      if (m_sequence.GetGlyphAtRel(i_x - x, i_y).has_value()) {
        const auto *temp = m_sequence.GetGlyphAtRel(i_x - x, i_y).value();
        place = temp->ch;
      }
#ifdef PLATFORM_WINDOWS
      setcharascii(m_window, place);
#elif PLATFORM_LINUX
      waddch(m_window, place);
#endif
    }
  }
  // TODO: add colors

  return VIMKTOR_OK;
}
VimktorErr_t Vimktor::GetInput() {
#ifdef PLATFORM_WINDOWS
  VimktorEvent_t event = InputManager::Get().GetEvent(console, m_mode);
#elif PLATFORM_LINUX
  VimktorEvent_t event = InputManager::Get().GetEvent(m_window, m_mode);
#endif
  HandleEvents(event);
  return VIMKTOR_OK;
}

VimktorErr_t Vimktor::HandleEvents(VimktorEvent_t event) {
  VimktorErr_t err = VIMKTOR_OK;
  switch (event) {

  case EV_NONE:

    break;
  case EV_CURSOR_DOWN:
    err = m_sequence.CursorMove(DOWN);
    break;
  case EV_CURSOR_UP:
    err = m_sequence.CursorMove(UP);
    break;
  case EV_CURSOR_RIGHT:
    err = m_sequence.CursorMove(RIGHT);
    break;
  case EV_CURSOR_LEFT:
    err = m_sequence.CursorMove(LEFT);
    break;
  case EV_CLOSE:
    m_mode = EXIT;
    break;
  case EV_ERASE_LINE:
    m_sequence.EraseLineCursor();
    break;
  case EV_MODE_NORMAL:
    m_mode = NORMAL;
    break;
  case EV_MODE_INSERT:
    m_mode = INSERT;
    break;
  case EV_MODE_INSERT_RIGHT:
    m_mode = INSERT;
    m_sequence.m_mode = m_mode;
    m_sequence.CursorMove(RIGHT);
    break;
  case EV_BACKSPACE:
    m_sequence.EraseCharCursor();
    break;
  case EV_INSERT_TEXT: {
    glyph_t gl = glyph_t(InputManager::Get().GetChar());
    m_sequence.InsertCharCursor(gl);
  } break;
  case EV_GO_TO_SOL:
    m_sequence.CursorMoveSol();
    break;
  case EV_GO_TO_NEXT_WORD:
    m_sequence.CursorMoveWordNext();
    break;
  case EV_GO_TO_EOL:
    m_sequence.CursorMoveEol();
    break;
  case EV_SAVE_FILE:
    WriteFile();
    HelperLog("saved to " + m_filename);
    break;
  case EV_NEW_LINE:
    m_sequence.AddNewLineCursor();
    break;
  case EV_GET_COMMAND:
    HandleCommands();
    break;
  case EV_FILE_EXPLORER:
    WriteFile();
    ExplorePath();
    break;
  case EV_ENTER_CURSOR_DIRECTORY:
    OpenFileCursor();
    break;
  }
  return VIMKTOR_OK;
}

VimktorErr_t Vimktor::HandleCommands() {
  std::string cmd;

  char16_t ch;

#ifdef PLATFORM_WINDOWS
  setinputblock(console, FALSE);
#elif PLATFORM_LINUX
  nodelay(m_window, 0);
#endif
  while (1) {
    HelperLog(":" + cmd);
#ifdef PLATFORM_WINDOWS
    refreshascii(console, m_window);
    ch = getchr(console);
#elif PLATFORM_LINUX
    wrefresh(m_window);
    ch = wgetch(m_window);
#endif


    if (ch == KEY_ESCAPE || ch == 13 || ch == KEY_ENTER) {
#ifdef PLATFORM_WINDOWS
      setinputblock(console, TRUE);
#elif PLATFORM_LINUX
      nodelay(m_window, 1);
#endif
      // ch = wgetch(m_window);
      break;
#ifdef PLATFORM_WINDOWS
      refreshascii(console, m_window);
#elif PLATFORM_LINUX
      wrefresh(stdscr);
#endif
    }
    if (ch == KEY_BACKSPACE) {
      if (cmd.size() > 0)
        cmd.pop_back();

    } else {
      cmd.push_back(ch);
    }
  }
#ifdef PLATFORM_WINDOWS
  setinputblock(console, FALSE);
#elif PLATFORM_LINUX
  nodelay(m_window, 0);
#endif

  // nodelay(m_window, 1);
  HelperLog("                                           ");
#ifdef PLATFORM_WINDOWS
  refreshascii(console, m_window);
#elif PLATFORM_LINUX
  wrefresh(stdscr);
#endif
  if (commandList.contains(cmd)) {
    HandleEvents(commandList[cmd]);
  }

  return VIMKTOR_OK;
}

VimktorErr_t Vimktor::OpenEmpty() { LoadFile(".vimktor_temp"); }

VimktorErr_t Vimktor::LoadFile(const std::string &fileName) {
  if (fileName == ".") {
    ExplorePath();
    return VIMKTOR_OK;
  }
  std::fstream file;
  m_filename = fileName;
  file.open(fileName, std::ios::in);
  if (!file.good()) {
    file.close();
    file.open(m_filename, std::ios::out | std::ios::in | std::fstream::trunc);
    return FILE_ERROR;
  }
  Debug::Log("plik zapisu: " + m_filename);
  m_sequence.LoadFile(file);
  file.close();
  return VIMKTOR_OK;
};

VimktorErr_t Vimktor::WriteFile(const std::string &fileName) {
  std::fstream file;
  file.open(fileName, std::ios::out);
  if (!file.good()) {
    return FILE_ERROR;
  }
  m_sequence.WriteFile(file);
  file.close();
  return VIMKTOR_OK;
};

VimktorErr_t Vimktor::WriteFile() {
  std::fstream file;
  file.open(m_filename, std::ios::out);
  if (!file.good()) {
    return FILE_ERROR;
  }
  m_sequence.WriteFile(file);
  file.close();
  return VIMKTOR_OK;
};

void Vimktor::Loop() {
  while (m_mode != EXIT) {
    RenderWindow();
    GetInput();
    m_sequence.m_mode = m_mode;
  }
}

position_t Vimktor::GetEditorDimensions() {

#ifdef PLATFORM_WINDOWS
  unsigned int width, height;
  getsize(console, &width, &height);
  return position_t(width, height);
#elif PLATFORM_LINUX
  return position_t(getmaxx(m_window), getmaxy(m_window));
#endif
}

void Vimktor::HelperLog(const std::string &msg) {
  position_t endPoint = GetEditorDimensions();
  size_t y = endPoint.y - 1;
  size_t x = 0;

#ifdef PLATFORM_WINDOWS
  setstringformattedcursorascii(m_window, y, x, "%s", msg.c_str());
#elif PLATFORM_LINUX
  wmove(m_window, y, x);
  clrtoeol();
  mvwprintw(m_window, y, x, "%s", msg.c_str());
#endif

}

Vimktor::CommandList_t Vimktor::commandList = {
    {"w", EV_SAVE_FILE},
    {"Explore", EV_FILE_EXPLORER},
    {"q", EV_CLOSE},
};

std::string Vimktor::GetModeStr() const {
  if (m_mode == VimktorMode_t::FILES)
    return "Files";
  if (m_mode == VimktorMode_t::INSERT)
    return "Insert";
  if (m_mode == VimktorMode_t::NORMAL)
    return "Normal";
  if (m_mode == VimktorMode_t::VISUAL)
    return "Visual";
}

VimktorErr_t Vimktor::ExplorePath() {
  m_sequence.m_cursorPos = position_t(0, 0);
  m_mode = FILES;
  HelperLog(std::filesystem::current_path().string());
  m_sequence.LoadCurrentDirectory();
}

VimktorErr_t Vimktor::ExplorePath(const std::string &path_str) {
  m_sequence.m_cursorPos = position_t(0, 0);
  m_mode = FILES;
  std::filesystem::current_path(path_str);
  HelperLog(std::filesystem::current_path().string());
  m_sequence.LoadCurrentDirectory();
}

VimktorErr_t Vimktor::OpenFileCursor() {
  auto path = std::filesystem::current_path();

  std::string path_str = path.string() + '/' + m_sequence.GetStringCursor();
  if (std::filesystem::is_directory(path_str) ||
      m_sequence.GetStringCursor() == "../" ||
      m_sequence.GetStringCursor() == "./") {
    ExplorePath(path_str);
  } else {
    LoadFile(m_sequence.GetStringCursor());
    m_mode = NORMAL;
  }
}
