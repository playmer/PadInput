#include <SDL.h>

#include <windows.h>

#include "SendEvents.hpp"


void SendMouseEvent(int64_t aButton, ButtonEvent aEvent)
{
  INPUT inputUnion;
  inputUnion.type = INPUT_MOUSE;
  inputUnion.mi = MOUSEINPUT{};
  
  inputUnion.mi.dx = 0;
  inputUnion.mi.dy = 0;

  inputUnion.mi.mouseData = 0;

  switch (aButton)
  {
    case SDL_BUTTON_LEFT: inputUnion.mi.dwFlags = aEvent == ButtonEvent::Pressed ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP; break;
    case SDL_BUTTON_MIDDLE: inputUnion.mi.dwFlags = aEvent == ButtonEvent::Pressed ? MOUSEEVENTF_MIDDLEDOWN : MOUSEEVENTF_MIDDLEUP; break;
    case SDL_BUTTON_RIGHT: inputUnion.mi.dwFlags = aEvent == ButtonEvent::Pressed ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP; break;
    case SDL_BUTTON_X1: inputUnion.mi.dwFlags = aEvent == ButtonEvent::Pressed ? MOUSEEVENTF_XDOWN : MOUSEEVENTF_XUP; break;
    case SDL_BUTTON_X2: inputUnion.mi.dwFlags = aEvent == ButtonEvent::Pressed ? MOUSEEVENTF_XDOWN : MOUSEEVENTF_XUP; break;
  }

  inputUnion.mi.time = 0;
  inputUnion.mi.dwExtraInfo = NULL;

  SendInput(1, &inputUnion, sizeof(INPUT));
}

WORD TranslateKey(int64_t aButton); 

void SendKeyboardEvent(int64_t aButton, ButtonEvent aEvent)
{
  KEYBDINPUT input;
  
  INPUT inputUnion;
  inputUnion.type = INPUT_KEYBOARD;
  inputUnion.ki = KEYBDINPUT{};
  inputUnion.ki.time = 0;
  inputUnion.ki.dwExtraInfo = 0;
  inputUnion.ki.dwFlags = aEvent == ButtonEvent::Pressed ? 0 : KEYEVENTF_KEYUP;
  
  auto virtualKey = TranslateKey(aButton);
  inputUnion.ki.wVk = virtualKey;
  inputUnion.ki.wScan = MapVirtualKeyA(virtualKey, MAPVK_VK_TO_VSC); // hardware scan code for key

  SendInput(1, &inputUnion, sizeof(INPUT));
}

WORD TranslateKey(int64_t aButton)
{
  switch (aButton)
  {
    case SDLK_a: return 'A';
    case SDLK_b: return 'B';
    case SDLK_c: return 'C';
    case SDLK_d: return 'D';
    case SDLK_e: return 'E';
    case SDLK_f: return 'F';
    case SDLK_g: return 'G';
    case SDLK_h: return 'H';
    case SDLK_i: return 'I';
    case SDLK_j: return 'J';
    case SDLK_k: return 'K';
    case SDLK_l: return 'L';
    case SDLK_m: return 'M';
    case SDLK_n: return 'N';
    case SDLK_o: return 'O';
    case SDLK_p: return 'P';
    case SDLK_q: return 'Q';
    case SDLK_r: return 'R';
    case SDLK_s: return 'S';
    case SDLK_t: return 'T';
    case SDLK_u: return 'U';
    case SDLK_v: return 'V';
    case SDLK_w: return 'W';
    case SDLK_x: return 'X';
    case SDLK_y: return 'Y';
    case SDLK_z: return 'Z';
    case SDLK_1: return '1'; // N1
    case SDLK_2: return '2'; // N2
    case SDLK_3: return '3'; // N3
    case SDLK_4: return '4'; // N4
    case SDLK_5: return '5'; // N5
    case SDLK_6: return '6'; // N6
    case SDLK_7: return '7'; // N7
    case SDLK_8: return '8'; // N8
    case SDLK_9: return '9'; // N9
    case SDLK_0: return '0'; // N0
    case SDLK_RETURN: return VK_RETURN; // Return
    case SDLK_ESCAPE: return VK_ESCAPE; // Escape
    case SDLK_BACKSPACE: return VK_BACK; // Backspace
    case SDLK_TAB: return VK_TAB; // Tab
    case SDLK_SPACE: return VK_SPACE; // Space
    case SDLK_MINUS: return VK_OEM_MINUS; // Minus
    case SDLK_KP_PLUS: return VK_OEM_PLUS; // KP_Plus
    case SDLK_PERIOD: return VK_OEM_PERIOD; // Period
    case SDLK_PAUSE: return VK_PAUSE; // Pause
    //case SDLK_EQUALS: return "SDLK_EQUALS"; // Equals
    //case SDLK_LEFTBRACKET: return "SDLK_LEFTBRACKET"; // LeftBracket
    //case SDLK_RIGHTBRACKET: return "SDLK_RIGHTBRACKET"; // RightBracket
    //case SDLK_BACKSLASH: return "SDLK_BACKSLASH"; // Backslash
    //case SDLK_SEMICOLON: return "SDLK_SEMICOLON"; // Semicolon
    //case SDLK_QUOTE: return "SDLK_QUOTE"; // Apostrophe
    //case SDLK_BACKQUOTE: return "SDLK_BACKQUOTE"; // Grave
    case SDLK_COMMA: return VK_OEM_COMMA; // Comma
    //case SDLK_SLASH: return "SDLK_SLASH"; // ForwardSlash
    case SDLK_PRINTSCREEN: return VK_SNAPSHOT; // PrintScreen
    case SDLK_INSERT: return VK_INSERT; // Insert
    case SDLK_DELETE: return VK_DELETE; // Delete
    case SDLK_PAGEUP: return VK_PRIOR; // PageUp
    case SDLK_PAGEDOWN: return VK_NEXT; // PageDown
    case SDLK_HOME: return VK_HOME; // Home
    case SDLK_END: return VK_END; // End
    case SDLK_UP: return VK_UP; // Up
    case SDLK_DOWN: return VK_DOWN; // Down
    case SDLK_RIGHT: return VK_RIGHT; // Right
    case SDLK_LEFT: return VK_LEFT; // Left
    case SDLK_CAPSLOCK: return VK_CAPITAL; // Capslock
    case SDLK_SCROLLLOCK: return VK_SCROLL; // ScrollLock
    case SDLK_NUMLOCKCLEAR: return VK_NUMLOCK; // NumLock
    case SDLK_KP_DIVIDE: return VK_DIVIDE; // KP_Divide
    case SDLK_KP_MULTIPLY: return VK_MULTIPLY; // KP_Multiply
    case SDLK_KP_1: return VK_NUMPAD1; // KP_1
    case SDLK_KP_2: return VK_NUMPAD2; // KP_2
    case SDLK_KP_3: return VK_NUMPAD3; // KP_3
    case SDLK_KP_4: return VK_NUMPAD4; // KP_4
    case SDLK_KP_5: return VK_NUMPAD5; // KP_5
    case SDLK_KP_6: return VK_NUMPAD6; // KP_6
    case SDLK_KP_7: return VK_NUMPAD7; // KP_7
    case SDLK_KP_8: return VK_NUMPAD8; // KP_8
    case SDLK_KP_9: return VK_NUMPAD9; // KP_9
    case SDLK_KP_0: return VK_NUMPAD0; // KP_0
    //case SDLK_KP_PERIOD: return "SDLK_KP_PERIOD"; // KP_Period
    case SDLK_KP_ENTER: return VK_RETURN; // KP_Enter
    //case SDLK_KP_EQUALS: return "SDLK_KP_EQUALS"; // KP_Equals
    case SDLK_F1: return VK_F1; // F1
    case SDLK_F2: return VK_F2; // F2
    case SDLK_F3: return VK_F3; // F3
    case SDLK_F4: return VK_F4; // F4
    case SDLK_F5: return VK_F5; // F5
    case SDLK_F6: return VK_F6; // F6
    case SDLK_F7: return VK_F7; // F7
    case SDLK_F8: return VK_F8; // F8
    case SDLK_F9: return VK_F9; // F9
    case SDLK_F10: return VK_F10; // F10
    case SDLK_F11: return VK_F11; // F11
    case SDLK_F12: return VK_F12; // F12
    case SDLK_F13: return VK_F13; // F13
    case SDLK_F14: return VK_F14; // F14
    case SDLK_F15: return VK_F15; // F15
    case KMOD_CTRL: return VK_CONTROL; // Control
    case SDLK_LCTRL: return VK_LCONTROL; // LeftControl
    case SDLK_RCTRL: return VK_RCONTROL; // RightControl
    case KMOD_SHIFT: return VK_SHIFT; // Shift
    case SDLK_LSHIFT: return VK_LSHIFT; // LeftShift
    case SDLK_RSHIFT: return VK_RSHIFT; // RightShift
    case KMOD_ALT: return VK_MENU; // Alt
    case SDLK_LALT: return VK_LMENU; // LeftAlt
    case SDLK_RALT: return VK_RMENU; // RightAlt
  }
}