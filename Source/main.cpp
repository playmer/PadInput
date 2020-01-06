#include <stdio.h>
#include <cstdint>
#include <iostream>
#include <array>
#include <filesystem>
#include <set>
#include <string>
#include <algorithm>
#include <iostream>
#include <vector>
#include <functional>
#include <tuple>
#include <charconv>
#include <fstream>

#include "SOIS/ImGuiSample.hpp"
#include "SOIS/ApplicationContext.hpp"

#include "imgui_node_editor/Source/crude_json.h"
#include "imgui_node_editor.h"

#include "imgui/imgui_stdlib.h"
#include "imgui/imgui_internal.h"

#include "nfd.h"

#include "SendEvents.hpp"

template <typename T,
          typename TIter = decltype(std::begin(std::declval<T>())),
          typename = decltype(std::end(std::declval<T>()))>
constexpr auto enumerate(T && iterable)
{
    struct iterator
    {
        size_t i;
        TIter iter;
        bool operator != (const iterator & other) const { return iter != other.iter; }
        void operator ++ () { ++i; ++iter; }
        auto operator * () const { return std::tie(i, *iter); }
    };
    struct iterable_wrapper
    {
        T iterable;
        auto begin() { return iterator{ 0, std::begin(iterable) }; }
        auto end() { return iterator{ 0, std::end(iterable) }; }
    };
    return iterable_wrapper{ std::forward<T>(iterable) };
}

struct SplitGenerator
{
  SplitGenerator(std::string_view aText, std::string_view aDelimiter)
    : mText{ aText }
    , mDelimiter{ aDelimiter }
    , mIndex{ 0 }
  {

  }

  std::string_view operator()()
  {
    std::string_view toReturn;

    if (mIndex > mText.size())
    {
      return toReturn;
    }
    
    const auto second = mText.find_first_of(mDelimiter, mIndex);

    if (mIndex != second)
    {
      toReturn = mText.substr(mIndex, second-mIndex);
    }

    //if (second == std::string_view::npos)
    //{
    //  break;
    //}

    mIndex = second + mDelimiter.size();

    return toReturn;
  }

  std::string_view mText;
  std::string_view mDelimiter;
  size_t mIndex;
};

std::string_view ChooseNewlineStyle(std::string_view aText)
{
  if (std::string_view::npos != aText.find_first_of("\r\n", 0))
  {
    return "\r\n";
  }
  else
  {
    return "\n";
  }
}


// Modified from SDL Documentation: https://wiki.libsdl.org/SDL_RWread
std::vector<char> file_read(const char* filename) {
  std::vector<char> toReturn;
  SDL_RWops* rw = SDL_RWFromFile(filename, "rb");

  if (rw == nullptr)
  {
    return toReturn;
  }

  Sint64 res_size = SDL_RWsize(rw);
  toReturn.resize(res_size + 1);

  Sint64 nb_read_total = 0, nb_read = 1;
  char* buf = toReturn.data();

  while (nb_read_total < res_size && nb_read != 0) 
  {
    nb_read = SDL_RWread(rw, buf, 1, (res_size - nb_read_total));
    nb_read_total += nb_read;
    buf += nb_read;
  }

  SDL_RWclose(rw);

  if (nb_read_total != res_size)
  {
    toReturn.resize(0);
    return toReturn;
  }

  toReturn[nb_read_total] = '\0';
  toReturn.resize(nb_read_total);

  return toReturn;
}

std::string GetImGuiIniPath()
{
  auto sdlIniPath = SDL_GetPrefPath("PlaymerTools", "PadInput");

  std::filesystem::path path{ sdlIniPath };
  SDL_free(sdlIniPath);

  path /= "imgui.ini";

  return path.u8string();
}

constexpr const char* ConvertToString(Uint8 aButton)
{
  switch (aButton)
  {
    case SDL_CONTROLLER_BUTTON_A: return "SDL_CONTROLLER_BUTTON_A";
    case SDL_CONTROLLER_BUTTON_B: return "SDL_CONTROLLER_BUTTON_B";
    case SDL_CONTROLLER_BUTTON_X: return "SDL_CONTROLLER_BUTTON_X";
    case SDL_CONTROLLER_BUTTON_Y: return "SDL_CONTROLLER_BUTTON_Y";
    case SDL_CONTROLLER_BUTTON_BACK: return "SDL_CONTROLLER_BUTTON_BACK";
    case SDL_CONTROLLER_BUTTON_GUIDE: return "SDL_CONTROLLER_BUTTON_GUIDE";
    case SDL_CONTROLLER_BUTTON_START: return "SDL_CONTROLLER_BUTTON_START";
    case SDL_CONTROLLER_BUTTON_LEFTSTICK: return "SDL_CONTROLLER_BUTTON_LEFTSTICK";
    case SDL_CONTROLLER_BUTTON_RIGHTSTICK: return "SDL_CONTROLLER_BUTTON_RIGHTSTICK";
    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: return "SDL_CONTROLLER_BUTTON_LEFTSHOULDER";
    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return  "SDL_CONTROLLER_BUTTON_RIGHTSHOULDER";
    case SDL_CONTROLLER_BUTTON_DPAD_UP: return  "SDL_CONTROLLER_BUTTON_DPAD_UP";
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN: return  "SDL_CONTROLLER_BUTTON_DPAD_DOWN";
    case SDL_CONTROLLER_BUTTON_DPAD_LEFT: return  "SDL_CONTROLLER_BUTTON_DPAD_LEFT";
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: return  "SDL_CONTROLLER_BUTTON_DPAD_RIGHT";
  }

  return "ERROR";
}

constexpr size_t ConvertToIndex(Uint8 aButton)
{
  switch (aButton)
  {
    case SDL_CONTROLLER_BUTTON_A: return 0;
    case SDL_CONTROLLER_BUTTON_B: return 1;
    case SDL_CONTROLLER_BUTTON_X: return 2;
    case SDL_CONTROLLER_BUTTON_Y: return 3;
    case SDL_CONTROLLER_BUTTON_BACK: return 4;
    case SDL_CONTROLLER_BUTTON_GUIDE: return 5;
    case SDL_CONTROLLER_BUTTON_START: return 6;
    case SDL_CONTROLLER_BUTTON_LEFTSTICK: return 7;
    case SDL_CONTROLLER_BUTTON_RIGHTSTICK: return 8;
    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: return 9;
    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return 10;
    case SDL_CONTROLLER_BUTTON_DPAD_UP: return 11;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN: return 12;
    case SDL_CONTROLLER_BUTTON_DPAD_LEFT: return 13;
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: return 14;
  }

  return static_cast<size_t>(-1);
}

Uint8 from_string_to_gamepad(std::string_view aValue)
{
  if (aValue == "SDL_CONTROLLER_BUTTON_A") return SDL_CONTROLLER_BUTTON_A;
  else if (aValue == "SDL_CONTROLLER_BUTTON_B") return SDL_CONTROLLER_BUTTON_B;
  else if (aValue == "SDL_CONTROLLER_BUTTON_X") return SDL_CONTROLLER_BUTTON_X;
  else if (aValue == "SDL_CONTROLLER_BUTTON_Y") return SDL_CONTROLLER_BUTTON_Y;
  else if (aValue == "SDL_CONTROLLER_BUTTON_BACK") return SDL_CONTROLLER_BUTTON_BACK;
  else if (aValue == "SDL_CONTROLLER_BUTTON_GUIDE") return SDL_CONTROLLER_BUTTON_GUIDE;
  else if (aValue == "SDL_CONTROLLER_BUTTON_START") return SDL_CONTROLLER_BUTTON_START;
  else if (aValue == "SDL_CONTROLLER_BUTTON_LEFTSTICK") return SDL_CONTROLLER_BUTTON_LEFTSTICK;
  else if (aValue == "SDL_CONTROLLER_BUTTON_RIGHTSTICK") return SDL_CONTROLLER_BUTTON_RIGHTSTICK;
  else if (aValue == "SDL_CONTROLLER_BUTTON_LEFTSHOULDER") return SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
  else if (aValue == "SDL_CONTROLLER_BUTTON_RIGHTSHOULDER") return SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
  else if (aValue == "SDL_CONTROLLER_BUTTON_DPAD_UP") return SDL_CONTROLLER_BUTTON_DPAD_UP;
  else if (aValue == "SDL_CONTROLLER_BUTTON_DPAD_DOWN") return SDL_CONTROLLER_BUTTON_DPAD_DOWN;
  else if (aValue == "SDL_CONTROLLER_BUTTON_DPAD_LEFT") return SDL_CONTROLLER_BUTTON_DPAD_LEFT;
  else if (aValue == "SDL_CONTROLLER_BUTTON_DPAD_RIGHT") return SDL_CONTROLLER_BUTTON_DPAD_RIGHT;
  else __debugbreak(); return 0;
}

Uint8 from_string_to_mouse(std::string_view aValue)
{
  if (aValue == "SDL_BUTTON_LEFT") return SDL_BUTTON_LEFT;
  else if (aValue == "SDL_BUTTON_RIGHT") return SDL_BUTTON_RIGHT;
  else if (aValue == "SDL_BUTTON_MIDDLE") return SDL_BUTTON_MIDDLE;
  else if (aValue == "SDL_BUTTON_X1") return SDL_BUTTON_X1;
  else if (aValue == "SDL_BUTTON_X2") return SDL_BUTTON_X2;
  else __debugbreak();  return 0;
}

int64_t from_string_to_keyboard(std::string_view aValue)
{
  if (aValue == "SDLK_a") return SDLK_a;
  else if (aValue == "SDLK_b") return SDLK_b;
  else if (aValue == "SDLK_c") return SDLK_c;
  else if (aValue == "SDLK_d") return SDLK_d;
  else if (aValue == "SDLK_e") return SDLK_e;
  else if (aValue == "SDLK_f") return SDLK_f;
  else if (aValue == "SDLK_g") return SDLK_g;
  else if (aValue == "SDLK_h") return SDLK_h;
  else if (aValue == "SDLK_i") return SDLK_i;
  else if (aValue == "SDLK_j") return SDLK_j;
  else if (aValue == "SDLK_k") return SDLK_k;
  else if (aValue == "SDLK_l") return SDLK_l;
  else if (aValue == "SDLK_m") return SDLK_m;
  else if (aValue == "SDLK_n") return SDLK_n;
  else if (aValue == "SDLK_o") return SDLK_o;
  else if (aValue == "SDLK_p") return SDLK_p;
  else if (aValue == "SDLK_q") return SDLK_q;
  else if (aValue == "SDLK_r") return SDLK_r;
  else if (aValue == "SDLK_s") return SDLK_s;
  else if (aValue == "SDLK_t") return SDLK_t;
  else if (aValue == "SDLK_u") return SDLK_u;
  else if (aValue == "SDLK_v") return SDLK_v;
  else if (aValue == "SDLK_w") return SDLK_w;
  else if (aValue == "SDLK_x") return SDLK_x;
  else if (aValue == "SDLK_y") return SDLK_y;
  else if (aValue == "SDLK_z") return SDLK_z;
  else if (aValue == "SDLK_1") return SDLK_1;
  else if (aValue == "SDLK_2") return SDLK_2;
  else if (aValue == "SDLK_3") return SDLK_3;
  else if (aValue == "SDLK_4") return SDLK_4;
  else if (aValue == "SDLK_5") return SDLK_5;
  else if (aValue == "SDLK_6") return SDLK_6;
  else if (aValue == "SDLK_7") return SDLK_7;
  else if (aValue == "SDLK_8") return SDLK_8;
  else if (aValue == "SDLK_9") return SDLK_9;
  else if (aValue == "SDLK_0") return SDLK_0;
  else if (aValue == "SDLK_RETURN") return SDLK_RETURN;
  else if (aValue == "SDLK_ESCAPE") return SDLK_ESCAPE;
  else if (aValue == "SDLK_BACKSPACE") return SDLK_BACKSPACE;
  else if (aValue == "SDLK_TAB") return SDLK_TAB;
  else if (aValue == "SDLK_SPACE") return SDLK_SPACE;
  else if (aValue == "SDLK_MINUS") return SDLK_MINUS;
  else if (aValue == "SDLK_KP_PLUS") return SDLK_KP_PLUS;
  else if (aValue == "SDLK_PERIOD") return SDLK_PERIOD;
  else if (aValue == "SDLK_PAUSE") return SDLK_PAUSE;
  else if (aValue == "SDLK_EQUALS") return SDLK_EQUALS;
  else if (aValue == "SDLK_LEFTBRACKET") return SDLK_LEFTBRACKET;
  else if (aValue == "SDLK_RIGHTBRACKET") return SDLK_RIGHTBRACKET;
  else if (aValue == "SDLK_BACKSLASH") return SDLK_BACKSLASH;
  else if (aValue == "SDLK_SEMICOLON") return SDLK_SEMICOLON;
  else if (aValue == "SDLK_QUOTE") return SDLK_QUOTE;
  else if (aValue == "SDLK_BACKQUOTE") return SDLK_BACKQUOTE;
  else if (aValue == "SDLK_COMMA") return SDLK_COMMA;
  else if (aValue == "SDLK_SLASH") return SDLK_SLASH;
  else if (aValue == "SDLK_PRINTSCREEN") return SDLK_PRINTSCREEN;
  else if (aValue == "SDLK_INSERT") return SDLK_INSERT;
  else if (aValue == "SDLK_DELETE") return SDLK_DELETE;
  else if (aValue == "SDLK_PAGEUP") return SDLK_PAGEUP;
  else if (aValue == "SDLK_PAGEDOWN") return SDLK_PAGEDOWN;
  else if (aValue == "SDLK_HOME") return SDLK_HOME;
  else if (aValue == "SDLK_END") return SDLK_END;
  else if (aValue == "SDLK_UP") return SDLK_UP;
  else if (aValue == "SDLK_DOWN") return SDLK_DOWN;
  else if (aValue == "SDLK_RIGHT") return SDLK_RIGHT;
  else if (aValue == "SDLK_LEFT") return SDLK_LEFT;
  else if (aValue == "SDLK_CAPSLOCK") return SDLK_CAPSLOCK;
  else if (aValue == "SDLK_SCROLLLOCK") return SDLK_SCROLLLOCK;
  else if (aValue == "SDLK_NUMLOCKCLEAR") return SDLK_NUMLOCKCLEAR;
  else if (aValue == "SDLK_KP_DIVIDE") return SDLK_KP_DIVIDE;
  else if (aValue == "SDLK_KP_MULTIPLY") return SDLK_KP_MULTIPLY;
  else if (aValue == "SDLK_KP_1") return SDLK_KP_1;
  else if (aValue == "SDLK_KP_2") return SDLK_KP_2;
  else if (aValue == "SDLK_KP_3") return SDLK_KP_3;
  else if (aValue == "SDLK_KP_4") return SDLK_KP_4;
  else if (aValue == "SDLK_KP_5") return SDLK_KP_5;
  else if (aValue == "SDLK_KP_6") return SDLK_KP_6;
  else if (aValue == "SDLK_KP_7") return SDLK_KP_7;
  else if (aValue == "SDLK_KP_8") return SDLK_KP_8;
  else if (aValue == "SDLK_KP_9") return SDLK_KP_9;
  else if (aValue == "SDLK_KP_0") return SDLK_KP_0;
  else if (aValue == "SDLK_KP_PERIOD") return SDLK_KP_PERIOD;
  else if (aValue == "SDLK_KP_ENTER") return SDLK_KP_ENTER;
  else if (aValue == "SDLK_KP_EQUALS") return SDLK_KP_EQUALS;
  else if (aValue == "SDLK_F1") return SDLK_F1;
  else if (aValue == "SDLK_F2") return SDLK_F2;
  else if (aValue == "SDLK_F3") return SDLK_F3;
  else if (aValue == "SDLK_F4") return SDLK_F4;
  else if (aValue == "SDLK_F5") return SDLK_F5;
  else if (aValue == "SDLK_F6") return SDLK_F6;
  else if (aValue == "SDLK_F7") return SDLK_F7;
  else if (aValue == "SDLK_F8") return SDLK_F8;
  else if (aValue == "SDLK_F9") return SDLK_F9;
  else if (aValue == "SDLK_F10") return SDLK_F10;
  else if (aValue == "SDLK_F11") return SDLK_F11;
  else if (aValue == "SDLK_F12") return SDLK_F12;
  else if (aValue == "SDLK_F13") return SDLK_F13;
  else if (aValue == "SDLK_F14") return SDLK_F14;
  else if (aValue == "SDLK_F15") return SDLK_F15;
  else if (aValue == "KMOD_CTRL") return KMOD_CTRL;
  else if (aValue == "SDLK_LCTRL") return SDLK_LCTRL;
  else if (aValue == "SDLK_RCTRL") return SDLK_RCTRL;
  else if (aValue == "KMOD_SHIFT") return KMOD_SHIFT;
  else if (aValue == "SDLK_LSHIFT") return SDLK_LSHIFT;
  else if (aValue == "SDLK_RSHIFT") return SDLK_RSHIFT;
  else if (aValue == "KMOD_ALT") return KMOD_ALT;
  else if (aValue == "SDLK_LALT") return SDLK_LALT;
  else if (aValue == "SDLK_RALT") return SDLK_RALT;
  else __debugbreak(); return 0;
}

constexpr auto endKeyIndex = ConvertToIndex(SDL_CONTROLLER_BUTTON_DPAD_RIGHT) + 1;

struct EventToSend
{
  enum class EventType
  {
    None,
    Keyboard,
    Mouse
  };

  EventToSend()
    : mType{EventType::None}
    , mKey{0}
  {

  }

  EventToSend(EventType aType, int64_t aKey)
    : mType{aType}
    , mKey{aKey}
  {

  }

  EventType mType;
  int64_t mKey;
};

const char* SdlMouseToString(Uint8 aButton)
{
  switch (aButton)
  {
    case SDL_BUTTON_LEFT:  return "SDL_BUTTON_LEFT";
    case SDL_BUTTON_RIGHT: return "SDL_BUTTON_RIGHT";
    case SDL_BUTTON_MIDDLE: return "SDL_BUTTON_MIDDLE";
    case SDL_BUTTON_X1: return "SDL_BUTTON_X1";
    case SDL_BUTTON_X2: return "SDL_BUTTON_X2";
  }

  return "ERROR";
}

constexpr const char* SdlScancodeToString(int64_t aScancode)
{
  switch (aScancode)
  {
    case SDLK_a: return "SDLK_a"; // A
    case SDLK_b: return "SDLK_b"; // B
    case SDLK_c: return "SDLK_c"; // C
    case SDLK_d: return "SDLK_d"; // D
    case SDLK_e: return "SDLK_e"; // E
    case SDLK_f: return "SDLK_f"; // F
    case SDLK_g: return "SDLK_g"; // G
    case SDLK_h: return "SDLK_h"; // H
    case SDLK_i: return "SDLK_i"; // I
    case SDLK_j: return "SDLK_j"; // J
    case SDLK_k: return "SDLK_k"; // K
    case SDLK_l: return "SDLK_l"; // L
    case SDLK_m: return "SDLK_m"; // M
    case SDLK_n: return "SDLK_n"; // N
    case SDLK_o: return "SDLK_o"; // O
    case SDLK_p: return "SDLK_p"; // P
    case SDLK_q: return "SDLK_q"; // Q
    case SDLK_r: return "SDLK_r"; // R
    case SDLK_s: return "SDLK_s"; // S
    case SDLK_t: return "SDLK_t"; // T
    case SDLK_u: return "SDLK_u"; // U
    case SDLK_v: return "SDLK_v"; // V
    case SDLK_w: return "SDLK_w"; // W
    case SDLK_x: return "SDLK_x"; // X
    case SDLK_y: return "SDLK_y"; // Y
    case SDLK_z: return "SDLK_z"; // Z
    case SDLK_1: return "SDLK_1"; // N1
    case SDLK_2: return "SDLK_2"; // N2
    case SDLK_3: return "SDLK_3"; // N3
    case SDLK_4: return "SDLK_4"; // N4
    case SDLK_5: return "SDLK_5"; // N5
    case SDLK_6: return "SDLK_6"; // N6
    case SDLK_7: return "SDLK_7"; // N7
    case SDLK_8: return "SDLK_8"; // N8
    case SDLK_9: return "SDLK_9"; // N9
    case SDLK_0: return "SDLK_0"; // N0
    case SDLK_RETURN: return "SDLK_RETURN"; // Return
    case SDLK_ESCAPE: return "SDLK_ESCAPE"; // Escape
    case SDLK_BACKSPACE: return "SDLK_BACKSPACE"; // Backspace
    case SDLK_TAB: return "SDLK_TAB"; // Tab
    case SDLK_SPACE: return "SDLK_SPACE"; // Space
    case SDLK_MINUS: return "SDLK_MINUS"; // Minus
    case SDLK_KP_PLUS: return "SDLK_KP_PLUS"; // KP_Plus
    case SDLK_PERIOD: return "SDLK_PERIOD"; // Period
    case SDLK_PAUSE: return "SDLK_PAUSE"; // Pause
    case SDLK_EQUALS: return "SDLK_EQUALS"; // Equals
    case SDLK_LEFTBRACKET: return "SDLK_LEFTBRACKET"; // LeftBracket
    case SDLK_RIGHTBRACKET: return "SDLK_RIGHTBRACKET"; // RightBracket
    case SDLK_BACKSLASH: return "SDLK_BACKSLASH"; // Backslash
    case SDLK_SEMICOLON: return "SDLK_SEMICOLON"; // Semicolon
    case SDLK_QUOTE: return "SDLK_QUOTE"; // Apostrophe
    case SDLK_BACKQUOTE: return "SDLK_BACKQUOTE"; // Grave
    case SDLK_COMMA: return "SDLK_COMMA"; // Comma
    case SDLK_SLASH: return "SDLK_SLASH"; // ForwardSlash
    case SDLK_PRINTSCREEN: return "SDLK_PRINTSCREEN"; // PrintScreen
    case SDLK_INSERT: return "SDLK_INSERT"; // Insert
    case SDLK_DELETE: return "SDLK_DELETE"; // Delete
    case SDLK_PAGEUP: return "SDLK_PAGEUP"; // PageUp
    case SDLK_PAGEDOWN: return "SDLK_PAGEDOWN"; // PageDown
    case SDLK_HOME: return "SDLK_HOME"; // Home
    case SDLK_END: return "SDLK_END"; // End
    case SDLK_UP: return "SDLK_UP"; // Up
    case SDLK_DOWN: return "SDLK_DOWN"; // Down
    case SDLK_RIGHT: return "SDLK_RIGHT"; // Right
    case SDLK_LEFT: return "SDLK_LEFT"; // Left
    case SDLK_CAPSLOCK: return "SDLK_CAPSLOCK"; // Capslock
    case SDLK_SCROLLLOCK: return "SDLK_SCROLLLOCK"; // ScrollLock
    case SDLK_NUMLOCKCLEAR: return "SDLK_NUMLOCKCLEAR"; // NumLock
    case SDLK_KP_DIVIDE: return "SDLK_KP_DIVIDE"; // KP_Divide
    case SDLK_KP_MULTIPLY: return "SDLK_KP_MULTIPLY"; // KP_Multiply
    case SDLK_KP_1: return "SDLK_KP_1"; // KP_1
    case SDLK_KP_2: return "SDLK_KP_2"; // KP_2
    case SDLK_KP_3: return "SDLK_KP_3"; // KP_3
    case SDLK_KP_4: return "SDLK_KP_4"; // KP_4
    case SDLK_KP_5: return "SDLK_KP_5"; // KP_5
    case SDLK_KP_6: return "SDLK_KP_6"; // KP_6
    case SDLK_KP_7: return "SDLK_KP_7"; // KP_7
    case SDLK_KP_8: return "SDLK_KP_8"; // KP_8
    case SDLK_KP_9: return "SDLK_KP_9"; // KP_9
    case SDLK_KP_0: return "SDLK_KP_0"; // KP_0
    case SDLK_KP_PERIOD: return "SDLK_KP_PERIOD"; // KP_Period
    case SDLK_KP_ENTER: return "SDLK_KP_ENTER"; // KP_Enter
    case SDLK_KP_EQUALS: return "SDLK_KP_EQUALS"; // KP_Equals
    case SDLK_F1: return "SDLK_F1"; // F1
    case SDLK_F2: return "SDLK_F2"; // F2
    case SDLK_F3: return "SDLK_F3"; // F3
    case SDLK_F4: return "SDLK_F4"; // F4
    case SDLK_F5: return "SDLK_F5"; // F5
    case SDLK_F6: return "SDLK_F6"; // F6
    case SDLK_F7: return "SDLK_F7"; // F7
    case SDLK_F8: return "SDLK_F8"; // F8
    case SDLK_F9: return "SDLK_F9"; // F9
    case SDLK_F10: return "SDLK_F10"; // F10
    case SDLK_F11: return "SDLK_F11"; // F11
    case SDLK_F12: return "SDLK_F12"; // F12
    case SDLK_F13: return "SDLK_F13"; // F13
    case SDLK_F14: return "SDLK_F14"; // F14
    case SDLK_F15: return "SDLK_F15"; // F15
    case KMOD_CTRL: return "KMOD_CTRL"; // Control
    case SDLK_LCTRL: return "SDLK_LCTRL"; // LeftControl
    case SDLK_RCTRL: return "SDLK_RCTRL"; // RightControl
    case KMOD_SHIFT: return "KMOD_SHIFT"; // Shift
    case SDLK_LSHIFT: return "SDLK_LSHIFT"; // LeftShift
    case SDLK_RSHIFT: return "SDLK_RSHIFT"; // RightShift
    case KMOD_ALT: return "KMOD_ALT"; // Alt
    case SDLK_LALT: return "SDLK_LALT"; // LeftAlt
    case SDLK_RALT: return "SDLK_RALT"; // RightAlt
  }
  
  return "ERROR";
}

class Controller
{
public:
  Controller()
  {
    __debugbreak();
  }

  Controller(SDL_GameController* aGameController)
    : mGameController{aGameController}
  {
    mName = SDL_GameControllerName(aGameController);
    mPreviousButtons.fill(false);
    mCurrentButtons.fill(false);
    
    LeftStick = glm::vec2{0.f,0.f};
    RightStick = glm::vec2{0.f,0.f};
    LeftTrigger = 0.f;
    RightTrigger = 0.f;
    Reset();
  }

  void Reset()
  {
    mPreviousButtons = mCurrentButtons;
  }

  float ToFloat(Sint16 aValue)
  {
    return (static_cast<float>(aValue + 32768.f) / 65535.f);
  }

  void HandleMotion(SDL_ControllerAxisEvent& aEvent)
  {
    switch (aEvent.axis)
    {
      case SDL_CONTROLLER_AXIS_LEFTX: LeftStick.x = 2.f * (ToFloat(aEvent.value) - .5f); return;
      case SDL_CONTROLLER_AXIS_LEFTY: LeftStick.y = 2.f * (ToFloat(aEvent.value) - .5f); return;
      case SDL_CONTROLLER_AXIS_RIGHTX: RightStick.x = 2.f * (ToFloat(aEvent.value) - .5f); return;
      case SDL_CONTROLLER_AXIS_RIGHTY: RightStick.y = 2.f * (ToFloat(aEvent.value) - .5f); return;
      case SDL_CONTROLLER_AXIS_TRIGGERLEFT: LeftTrigger = 2.f * (ToFloat(aEvent.value) - .5f); return;
      case SDL_CONTROLLER_AXIS_TRIGGERRIGHT: RightTrigger = 2.f * (ToFloat(aEvent.value) - .5f); return;
    }
  }
  
  void HandleButton(SDL_ControllerButtonEvent& aEvent)
  {
    mCurrentButtons[ConvertToIndex(aEvent.button)] = aEvent.state == SDL_PRESSED ? true : false;
    printf("%s %s\n", ConvertToString(aEvent.button), aEvent.state == SDL_PRESSED ? "Pressed" : "Released");
  }

  SDL_GameController* mGameController;
  const char* mName;
  
  std::array<bool, 15> mPreviousButtons;
  std::array<bool, 15> mCurrentButtons;

  glm::vec2 LeftStick;
  glm::vec2 RightStick;
  float LeftTrigger;
  float RightTrigger;
};

struct Controllers
{
  Controllers()
  {
    //for(int i = 0; i < SDL_NumJoysticks(); i++ ) 
    //{
    //  if (SDL_IsGameController(i))
    //  {
    //    auto pad = SDL_GameControllerOpen(i);
    //      
    //    if (pad)
    //    {
    //      auto name = SDL_GameControllerName(pad);
    //      auto instanceId = SDL_JoystickGetDeviceInstanceID(i);
    //
    //      printf("Added %s, %d\n", name, instanceId);
    //      mControllers.emplace(instanceId, Controller(pad));
    //    }
    //  }
    //}
  }

  void HandleEvent(SDL_Event& aEvent)
  {
    switch (aEvent.type)
    {
        // Joypad Events
      case SDL_CONTROLLERDEVICEADDED:
      {
        auto event = aEvent.cdevice;
        
        auto pad = SDL_GameControllerOpen(event.which);
          
        if (pad)
        {
          auto name = SDL_GameControllerName(pad);
          SDL_Joystick* joystick = SDL_GameControllerGetJoystick(pad);
          SDL_JoystickID instanceId = SDL_JoystickInstanceID(joystick);

          printf("Added %s, %d\n", name, instanceId);
          mControllers.emplace(instanceId, Controller(pad));
        }
      }
      case SDL_CONTROLLERDEVICEREMOVED:
      {
        auto event = aEvent.cdevice;
        
        //if (auto it = mControllers.find(event.which); 
        //    it != mControllers.end())
        //{
        //  mControllers.erase(it);
        //}

        printf("Removed %d\n", event.which);
        break;
      }
        // Controller Events
      case SDL_CONTROLLERAXISMOTION:
      {
        mControllers[aEvent.caxis.which].HandleMotion(aEvent.caxis);
        break;
      }
      case SDL_CONTROLLERBUTTONDOWN:
      case SDL_CONTROLLERBUTTONUP:
      {
        mControllers[aEvent.cbutton.which].HandleButton(aEvent.cbutton);
        break;
      }
      case SDL_CONTROLLERDEVICEREMAPPED:
      {
        puts("Remaped\n");
        break;
      }
      case SDL_MOUSEBUTTONUP:
      {
        mEventsReceivedThisFrame.emplace_back(EventToSend(EventToSend::EventType::Mouse, aEvent.button.button));
      }
      case SDL_KEYUP:
      {
        mEventsReceivedThisFrame.emplace_back(EventToSend(EventToSend::EventType::Keyboard, aEvent.key.keysym.sym));
      }
    }
  }

  void EndFrame()
  {
    for (auto& [key, controller] : mControllers)
    {
      controller.Reset();
    }

    mEventsReceivedThisFrame.clear();
  }

  std::map<SDL_JoystickID, Controller> mControllers;
  std::vector<EventToSend> mEventsReceivedThisFrame;
};


void ControllerEventHandler(SDL_Event& aEvent, void* aUserData)
{
  Controllers* controllers = static_cast<Controllers*>(aUserData);
  controllers->HandleEvent(aEvent);
}

struct ImGuiDockPlacement
{
  ImGuiDockPlacement()
  {
    constexpr ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;

    //ImGuiIO& io = ImGui::GetIO();

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGuiWindowFlags window_flags = /* ImGuiWindowFlags_NoMenuBar | */ ImGuiWindowFlags_NoDocking;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    bool openWindow = true;
    ImGui::Begin("MainWindow", &openWindow, window_flags);
    ImGui::PopStyleVar();
    ImGui::PopStyleVar(2);

    mDockspaceId = ImGui::GetID("MainDockSpace");
    ImGui::DockSpace(mDockspaceId, viewport->Size, dockspaceFlags);

    InitializeDockspaceLayoutOnce();

    ImGui::End();
  }

  void InitializeDockspaceLayoutOnce()
  {
    if (ImGui::DockBuilderGetNode(mDockspaceId) != nullptr)
    {
      return;
    }

    //ImGuiID dockspaceId = ImGui::GetID("MyDockSpace");
    //ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), dockspaceFlags);
    //
    //ImGuiID leftSide;
    //ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_::ImGuiDir_Right, .66f, &mGraphDock, &leftSide);
    //
    //ImGui::DockBuilderSplitNode(leftSide, ImGuiDir_::ImGuiDir_Down, .5f, &mCommonColumnsDock, &mTablesDock);

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::DockBuilderRemoveNode(mDockspaceId); // Clear out existing layout
    ImGui::DockBuilderSetNodeSize(mDockspaceId, viewport->Size);
    ImGui::DockBuilderAddNode(mDockspaceId, ImGuiDockNodeFlags_DockSpace); // Add empty node

    //ImGuiID dock_main_id = mDockspaceId; // This variable will track the document node, however we are not using it here as we aren't docking anything into it.
    mTablesDock = ImGui::DockBuilderSplitNode(mDockspaceId, ImGuiDir_Left, 0.20f, NULL, &mDockspaceId);
    mCommonColumnsDock = ImGui::DockBuilderSplitNode(mDockspaceId, ImGuiDir_Left, 0.20f, NULL, &mDockspaceId);

    //ImGui::DockBuilderDockWindow("Databases", leftDock1);
    //ImGui::DockBuilderDockWindow("Common Columns", leftDock2);
    //ImGui::DockBuilderDockWindow("Table Graph", mDockspaceId);

    mGraphDock = mDockspaceId;
    ImGui::DockBuilderFinish(mDockspaceId);
  }

  ImGuiID mDockspaceId;
  ImGuiID mCommonColumnsDock;
  ImGuiID mTablesDock;
  ImGuiID mGraphDock;
};

struct Profile
{
  std::array<EventToSend, endKeyIndex> mRedirectionMapping;
  
  // Mouse Related
  int mPixelsToMove = 20;
  bool mLeftStickMapToMouse = false;
  bool mRightStickMapToMouse = false;
  
  // Gamepad related
  float mLeftStickDeadZone = 0.0f;
  float mRightStickDeadZone = 0.0f;
  bool mLeftTriggerMapToButton = false;
  bool mRightTriggerMapToButton = false;

  void SaveToFile(std::string aFilename)
  {
    std::string buffer;

    for (auto [i, redirect] : enumerate(mRedirectionMapping))
    {
      // Name of the button
      buffer += ConvertToString(static_cast<Uint8>(i));
      buffer += ':';
      
      // Redirecting to type:
      buffer += redirect.mType == EventToSend::EventType::Mouse    ? "Mouse" :
                redirect.mType == EventToSend::EventType::Keyboard ? "Keyboard" :
                                                                     "None";
      buffer += ':';
      
      // Redirecting to button:
      buffer += redirect.mType == EventToSend::EventType::Mouse    ? SdlMouseToString(static_cast<Uint8>(redirect.mKey)) :
                redirect.mType == EventToSend::EventType::Keyboard ? SdlScancodeToString(redirect.mKey) :
                                                                     "0";
      buffer += '\n';
    }

    buffer += "PixelsToMove";
    buffer += ':';
    buffer += std::to_string(mPixelsToMove);
    buffer += '\n';
    
    buffer += "LeftStickMapToMouse";
    buffer += ':';
    buffer += mLeftStickMapToMouse ? "true" : "false";
    buffer += '\n';
    
    buffer += "RightStickMapToMouse";
    buffer += ':';
    buffer += mRightStickMapToMouse ? "true" : "false";
    buffer += '\n';
    
    buffer += "LeftStickDeadZone";
    buffer += ':';
    buffer += std::to_string(mLeftStickDeadZone);
    buffer += '\n';
    
    buffer += "RightStickDeadZone";
    buffer += ':';
    buffer += std::to_string(mRightStickDeadZone);
    buffer += '\n';
    
    buffer += "LeftTriggerMapToButton";
    buffer += ':';
    buffer += mLeftTriggerMapToButton ? "true" : "false";
    buffer += '\n';
    
    buffer += "RightTriggerMapToButton";
    buffer += ':';
    buffer += mRightTriggerMapToButton ? "true" : "false";
    buffer += '\n';

    std::ofstream out = std::ofstream{ aFilename };
    out << buffer;
  }

  template <typename tType>
  static void ReadLine(std::string_view aLine, std::string_view aName, tType& aValueToReadInto)
  {
    SplitGenerator splitter(aLine, ":");
    auto name = splitter();
    auto value = splitter();

    assert(name == aName);
    auto result = std::from_chars(value.data(), value.data() + value.size(), aValueToReadInto);
  }

  template <>
  static void ReadLine<bool>(std::string_view aLine, std::string_view aName, bool& aValueToReadInto)
  {
    SplitGenerator splitter(aLine, ":");
    auto name = splitter();
    auto value = splitter();

    assert(name == aName);
    assert(value == "false" || value == "true");

    aValueToReadInto = value == "true" ? true : false;
  }

  void ReadFromFile(std::string aFilename)
  {
    auto fileTextVec = file_read(aFilename.c_str());

    std::string_view text{ fileTextVec.data(), fileTextVec.size() };

    auto newLineDelimiter = ChooseNewlineStyle(text);

    SplitGenerator lineSplitter(text, newLineDelimiter);

    size_t i = 0;
    for (auto& redirect : mRedirectionMapping)
    {
      SplitGenerator splitter(lineSplitter(), ":");

      auto buttonName = splitter();
      auto redirectionType = splitter();
      auto redirectionKey = splitter();

      assert(i == from_string_to_gamepad(buttonName));
      
      redirect.mType = redirectionType == "Mouse"    ? EventToSend::EventType::Mouse:
                       redirectionType == "Keyboard" ? EventToSend::EventType::Keyboard:
                                                       EventToSend::EventType::None;
      
      redirect.mKey = redirectionType == "Mouse" ? from_string_to_mouse(redirectionKey) :
                      redirectionType == "Keyboard" ? from_string_to_keyboard(redirectionKey) :
                      0;

      ++i;
    }
    
    ReadLine(lineSplitter(), "PixelsToMove", mPixelsToMove);
    ReadLine(lineSplitter(), "LeftStickMapToMouse", mLeftStickMapToMouse);    
    ReadLine(lineSplitter(), "RightStickMapToMouse", mRightStickMapToMouse);    
    ReadLine(lineSplitter(), "LeftStickDeadZone", mLeftStickDeadZone);
    ReadLine(lineSplitter(), "RightStickDeadZone", mRightStickDeadZone);
    ReadLine(lineSplitter(), "LeftTriggerMapToButton", mLeftTriggerMapToButton);    
    ReadLine(lineSplitter(), "RightTriggerMapToButton", mRightTriggerMapToButton);
  }
};

void DoPadInputWork(Controller aController, std::vector<EventToSend> aEventsSendThisFrame, Profile& aProfile);
void DoRedirections(Controller& aController, Profile& aProfile);

std::vector<std::string> get_file_list()
{
  std::vector<std::string> toReturn;

  return toReturn;
}

int main(int, char**)
{
  SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");

  SOIS::ApplicationInitialization();
  
  auto iniPath = GetImGuiIniPath();

  Controllers controllers;

  SOIS::ApplicationContextConfig config;
  config.aBlocking = false;
  config.aHandler = &ControllerEventHandler;
  config.aUserData = &controllers;
  config.aIniFile = iniPath.c_str();
  config.aWindowName = "PadInput";

  SOIS::ApplicationContext context{config};
  SOIS::ImGuiSample sample;

  std::string lookingFor = "";
  SDL_JoystickID lookingForId = 0;

  Profile profile;
  
  if (!controllers.mControllers.empty())
  {
    auto& [key, controller] = *controllers.mControllers.begin();
    
    lookingForId = key;
    lookingFor = controller.mName;
  }

  while (context.Update())
  {
    static bool updateNow = true;

    if (updateNow)
    {
      context.SetCallbackInfo(&ControllerEventHandler, &controllers);
      updateNow = false;
    }

    if (false == (SDL_GetWindowFlags(context.mWindow) & SDL_WINDOW_INPUT_FOCUS))
    {
      SDL_GameControllerUpdate();
    }

    ImGuiDockPlacement dock;

    sample.Update();

    
    ImGui::Begin("Controller Info");
    
    if (ImGui::Button("Save###Save_Profile"))
    {
      profile.SaveToFile("profile.ini");
    } ImGui::SameLine();
    
    if (ImGui::Button("Save As###SaveAs_Profile"))
    {
      
    } ImGui::SameLine();

    if (ImGui::Button("Load###Load_Profile"))
    {
      profile.ReadFromFile("profile.ini");
    }

    if (ImGui::BeginCombo("Select Controller:", lookingFor.c_str()))
    {
      for (auto& [key, controller] : controllers.mControllers)
      {
        ImGui::PushID(key);
        if (ImGui::Selectable(controller.mName, lookingForId == key))
        {
          lookingFor = controller.mName;
          lookingForId = key;
        }
        ImGui::PopID();
      }

      ImGui::EndCombo();
    }

    auto it = controllers.mControllers.find(lookingForId);
    
    if (it != controllers.mControllers.end())
    {
      DoPadInputWork(it->second, controllers.mEventsReceivedThisFrame, profile);
    }

    ImGui::End();

    DoRedirections(it->second, profile);

    controllers.EndFrame();
  }

  return 0;
}

template<typename tReturn>
tReturn GetMousePosition()
{
  glm::i32vec2 toReturn;
  SDL_GetGlobalMouseState(&toReturn.x, &toReturn.y);
  return tReturn(toReturn);
}
  
void SetCursorPositionInScreenCoordinates(glm::i32vec2 aPosition)
{
  SDL_WarpMouseGlobal(aPosition.x, aPosition.y);
}

struct GamepadButtonImGuiIds
{
  GamepadButtonImGuiIds()
  {
  }

  GamepadButtonImGuiIds(size_t aButton)
    : cClearButtonId{}
    , cSetButtonId{}
    , cCheckBoxId{}
    , cButtonNameLabelId{}
    , cRedirectLabelId{}
  {
    std::string baseName = "###"; 
    baseName += ConvertToString(static_cast<Uint8>(aButton));

    cClearButtonId = baseName + "_ClearButtonId";
    cSetButtonId = baseName + "_SetButtonId";
    cCheckBoxId = baseName + "_CheckBoxId";
    cButtonNameLabelId = baseName + "_ButtonNameLabelId";
    cRedirectLabelId = baseName + "_RedirectLabelId";
  }

  std::string cClearButtonId;
  std::string cSetButtonId;
  std::string cCheckBoxId;
  std::string cButtonNameLabelId;
  std::string cRedirectLabelId;
};

std::array<GamepadButtonImGuiIds, endKeyIndex> FilledGamepadImGuiIds()
{
  std::array<GamepadButtonImGuiIds, endKeyIndex> toReturn;

  for (size_t i = 0; i < endKeyIndex; ++i)
  {
    toReturn[i] = GamepadButtonImGuiIds(i);
  }

  return toReturn;
}

// Horrible hack: https://github.com/ocornut/imgui/issues/1655#issuecomment-489174962
template <size_t N>
void SetColumnWidth(float aWidth)
{
  static unsigned short initial_column_spacing = 0;

  if (initial_column_spacing < 2) 
  {
      ImGui::SetColumnWidth(N, aWidth);
      initial_column_spacing++;
  }
}

bool OutsideDeadZone(glm::vec2 aStickPosition, float aDeadzone)
{
  return glm::length(aStickPosition) > aDeadzone;
}


void DoPadInputWork(Controller aController, std::vector<EventToSend> aEventsSendThisFrame, Profile& aProfile)
{
  static const std::array<GamepadButtonImGuiIds, endKeyIndex> imguiIds = FilledGamepadImGuiIds();

  // Set Related Statics (to be moved to a struct/class at some point)
  static bool currentlySetting = false;
  static size_t indexSetting = 0;

  // Set Related work:
  if (currentlySetting && (false == aEventsSendThisFrame.empty()))
  {
    currentlySetting = false;

    aProfile.mRedirectionMapping[indexSetting] = aEventsSendThisFrame.front();
  }

  // Outputing controller state
  {
    //////////////////////////////////////////////////////////////
    // Mouse Speed
    ImGui::DragInt("Pixels to move Mouse", &aProfile.mPixelsToMove, 1, 1, 200);

    //////////////////////////////////////////////////////////////
    // Left Stick
    ImGui::DragFloat2("Left Stick", &aController.LeftStick[0], 0.0f);
    ImGui::SameLine();
    ImGui::Checkbox("Map to Mouse###LeftMapToMouse", &aProfile.mLeftStickMapToMouse);
        
    if (ImGui::Button("Set Left Stick Deadzone"))
    {
      aProfile.mLeftStickDeadZone = glm::length(aController.LeftStick);
    } ImGui::SameLine();

    ImGui::DragFloat("Left Stick Deadzone", &aProfile.mLeftStickDeadZone, 0.001f, 0.f, 1.f);
    
    //////////////////////////////////////////////////////////////
    // Right Stick
    ImGui::DragFloat2("Right Stick", &aController.RightStick[0], 0.0f);
    ImGui::SameLine();
    ImGui::Checkbox("Map to Mouse###RightMapToMouse", &aProfile.mRightStickMapToMouse);
    
    if (ImGui::Button("Set Right Stick Deadzone"))
    {
      aProfile.mRightStickDeadZone = glm::length(aController.RightStick);
    } ImGui::SameLine();
    ImGui::DragFloat("Right Stick Deadzone", &aProfile.mRightStickDeadZone, 0.001f, 0.f, 1.f);

    
    //////////////////////////////////////////////////////////////
    // Left Trigger
    ImGui::DragFloat("Left Trigger", &aController.LeftTrigger, 0.0f);
    ImGui::SameLine();
    ImGui::Checkbox("Treat as Button", &aProfile.mLeftTriggerMapToButton);
    
    //////////////////////////////////////////////////////////////
    // Right Trigger
    ImGui::DragFloat("Right Trigger", &aController.RightTrigger, 0.0f);
    ImGui::SameLine();
    ImGui::Checkbox("Treat as Button", &aProfile.mRightTriggerMapToButton);
    
    //////////////////////////////////////////////////////////////
    // Buttons
    ImGui::Columns(5, "GamepadButtons"); // 4-ways, with border
    ImGui::Separator();
    ImGui::Text("Clear"); SetColumnWidth<0>(60.f); ImGui::NextColumn();
    ImGui::Text("Set"); SetColumnWidth<1>(40.f); ImGui::NextColumn();
    ImGui::Text("Pressed"); SetColumnWidth<2>(60.f); ImGui::NextColumn();
    ImGui::Text("Button Name"); ImGui::NextColumn();
    ImGui::Text("Redirected To"); ImGui::NextColumn();
    ImGui::Separator();

    for (size_t i = 0; i < endKeyIndex; ++i)
    {
      //////////////////////////////////////////////////////////////
      // Clear
      ImGui::PushID(imguiIds[i].cClearButtonId.c_str());
      if (ImGui::Button("Clear"))
      {
        aProfile.mRedirectionMapping[i] = EventToSend();
        
        // If the user clicks clear, we should also void out anything currently being set.
        currentlySetting = false;
      }
      ImGui::PopID();

      ImGui::NextColumn();
      
      //////////////////////////////////////////////////////////////
      // Set
      ImGui::PushID(imguiIds[i].cSetButtonId.c_str());
      if (ImGui::Button("Set"))
      {
        currentlySetting = true;
        indexSetting = i;
      }
      ImGui::PopID();
      
      ImGui::NextColumn();

      
      //////////////////////////////////////////////////////////////
      // Active
      //
      // We don't actually want ImGui to set this bool, but we can't have a non-interactable checkbox, so we'll just reset
      // the bool to it was prior to the call.
      bool previous = aController.mCurrentButtons[i];
      ImGui::Checkbox(imguiIds[i].cCheckBoxId.c_str(), &aController.mCurrentButtons[i]);
      aController.mCurrentButtons[i] = previous;
      ImGui::NextColumn();
      
      //////////////////////////////////////////////////////////////
      // Button Name
      auto buttonName = ConvertToString(static_cast<Uint8>(i));
      ImGui::LabelText(imguiIds[i].cButtonNameLabelId.c_str(), "%s", buttonName);
      ImGui::NextColumn();
      
      //////////////////////////////////////////////////////////////
      // Redirected To
      const char* buttonRedirectName = "None";

      auto& eventToSend = aProfile.mRedirectionMapping[i];

      switch (eventToSend.mType)
      {
        case EventToSend::EventType::None: break;
        case EventToSend::EventType::Keyboard: buttonRedirectName = SdlScancodeToString(eventToSend.mKey);  break;
        case EventToSend::EventType::Mouse: buttonRedirectName = SdlMouseToString(static_cast<Uint8>(eventToSend.mKey)); break;
      }

      ImGui::LabelText(imguiIds[i].cRedirectLabelId.c_str(), " Redirected to: %s", buttonRedirectName);
      ImGui::NextColumn();
    }
    
    ImGui::Columns(1);
    ImGui::Separator();
  }
}


void DoRedirections(Controller& aController, Profile& aProfile)
{
  // Mouse Related Locals
  glm::ivec2 mouseMovement = glm::ivec2{ 0,0 };

  if (/*aController.mCurrentButtons[ConvertToIndex(SDL_CONTROLLER_BUTTON_A)] 
    &&*/ aProfile.mLeftStickMapToMouse
    && OutsideDeadZone(aController.LeftStick, aProfile.mLeftStickDeadZone))
  {
    auto movementVector = aController.LeftStick * static_cast<float>(aProfile.mPixelsToMove);
    
    mouseMovement += movementVector;
  }

  if (/*aController.mCurrentButtons[ConvertToIndex(SDL_CONTROLLER_BUTTON_A)] 
    &&*/ aProfile.mRightStickMapToMouse 
    && OutsideDeadZone(aController.RightStick, aProfile.mRightStickDeadZone))
  {
    auto movementVector = aController.RightStick * static_cast<float>(aProfile.mPixelsToMove);

    mouseMovement += movementVector;
  }

  if (mouseMovement.x != 0 || mouseMovement.y != 0)
  {
    auto newPosition = glm::ivec2(GetMousePosition<glm::ivec2>()) + mouseMovement;
    SetCursorPositionInScreenCoordinates(newPosition);
  }
  
  for (size_t i = 0; i < endKeyIndex; ++i)
  {
    auto& redirect = aProfile.mRedirectionMapping[i];
    ButtonEvent event = ButtonEvent::None;

    // If it's now down, and it wasn't before, it was just pressed.
    if (aController.mCurrentButtons[i] && !aController.mPreviousButtons[i])
    {
      event = ButtonEvent::Pressed;
    }
    // If it's now up, but it was down before, it was just released.
    else if (!aController.mCurrentButtons[i] && aController.mPreviousButtons[i])
    {
      event = ButtonEvent::Released;
    }

    if (event != ButtonEvent::None)
    {
      switch (redirect.mType)
      {
        case EventToSend::EventType::Keyboard: SendKeyboardEvent(redirect.mKey, event);
        case EventToSend::EventType::Mouse: SendMouseEvent(redirect.mKey, event);
      }
    }
  }
}