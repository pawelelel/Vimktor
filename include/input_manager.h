#pragma once
// singleton ?
#include "common.h"

#ifdef PLATFORM_WINDOWS
#include "pcl.h"
#elif PLATFORM_LINUX
#include <ncurses.h>
#endif



#define KEY_ESCAPE 27 // in ncurses ESC and R_ALT have same int value
#define KEY_ENTER_VIMKTOR 13
class InputManager {
public:
  static InputManager &Get() {
    static InputManager _instance;
    return _instance;
  };
  void GetCmdBuffer() = delete;
  inline uint32_t GetChar() const noexcept { return inputCh; }

#ifdef PLATFORM_WINDOWS
  VimktorEvent_t GetEvent(Console *console, VimktorMode_t mode);
#elif PLATFORM_LINUX
  VimktorEvent_t GetEvent(WINDOW *win, VimktorMode_t mode);
#endif

  InputManager(const InputManager &) = delete;
  InputManager &operator=(const InputManager &) = delete;

private:
  int32_t inputCh;

#ifdef PLATFORM_WINDOWS
  VimktorEvent_t IsEscapePressed(Console *console);
#elif PLATFORM_LINUX
  VimktorEvent_t IsEscapePressed();
#endif

#ifdef PLATFORM_WINDOWS
  VimktorEvent_t HandleDeleteEvent(Console *console);
#elif PLATFORM_LINUX
  VimktorEvent_t HandleDeleteEvent(WINDOW *win);
#endif

  VimktorEvent_t GetInputFileExp();

#ifdef PLATFORM_WINDOWS
  VimktorEvent_t GetInputInsert(Console *console);
#elif PLATFORM_LINUX
  VimktorEvent_t GetInputInsert();
#endif



#ifdef PLATFORM_WINDOWS
  VimktorEvent_t GetInputNormal(Console *console);
#elif PLATFORM_LINUX
  VimktorEvent_t GetInputNormal(WINDOW *win);
#endif
  InputManager() { inputCh = 0; }
};
