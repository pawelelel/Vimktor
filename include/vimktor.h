#include "common.h"
#include "sequence.h"
#include <memory>
#include <unordered_map>

#ifdef PLATFORM_WINDOWS
#include "pcl.h"
#elif PLATFORM_LINUX
#include "curses.h"
#endif

class Vimktor {
  // methods
public:
  typedef std::unordered_map<std::string, VimktorEvent_t> CommandList_t;

#ifdef PLATFORM_WINDOWS
  Vimktor() : m_mode(NORMAL) {
    console = start();
    m_window = initascii(console);
  }
  Vimktor(AsciiScreen *window, std::fstream &file) : m_window(window), m_mode(NORMAL) {}
#elif PLATFORM_LINUX
  Vimktor() : m_window(stdscr), m_mode(NORMAL) {}
  Vimktor(WINDOW *window, std::fstream &file)
      : m_window(window), m_mode(NORMAL) {}
#endif



  inline Vimktor(std::string &fileName) { LoadFile(fileName); }

  void Init();
  void End();
  void Loop();
  // private:
  VimktorErr_t OpenEmpty();
  VimktorErr_t LoadFile(const std::string &fileName);
  VimktorErr_t WriteFile(const std::string &fileName);
  VimktorErr_t WriteFile();
  VimktorErr_t InitCurses();

  // renderer
  VimktorErr_t RenderLineNumber();
  VimktorErr_t RenderWindow();
  VimktorErr_t RenderHelper();
  void HelperLog(const std::string &msg);
  VimktorErr_t RenderCursor();
  VimktorErr_t RenderText(uint16_t x, uint16_t y, uint16_t width,
                          uint16_t height);
  position_t GetEditorDimensions();
  VimktorErr_t GetInput();

  VimktorErr_t HandleEvents(VimktorEvent_t event);
  VimktorErr_t HandleCommands();

  std::string GetModeStr() const;

  // file exploring
  VimktorErr_t ExplorePath();
  VimktorErr_t ExplorePath(const std::string &path_str);
  VimktorErr_t OpenFileCursor();

  // variables

#ifdef PLATFORM_WINDOWS
  AsciiScreen * m_window;
  Console * console;
#elif PLATFORM_LINUX
	WINDOW *m_window;
#endif
  Sequence m_sequence;
  VimktorMode_t m_mode;

private:
  static CommandList_t commandList;
  std::string m_filename;
};
