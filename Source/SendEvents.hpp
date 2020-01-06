enum class ButtonEvent
{
  None,
  Pressed,
  Released
};

void SendMouseEvent(int64_t aButton, ButtonEvent aEvent);
void SendKeyboardEvent(int64_t aButton, ButtonEvent aEvent);