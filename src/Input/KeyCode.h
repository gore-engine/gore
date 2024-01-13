#pragma once

namespace gore
{

enum class InputType
{
    Digital,
    Analog
};

enum class JoystickButtonCode
{
    DPadUp        = 0x0001, // DPad Up
    DPadDown      = 0x0002, // DPad Down
    DPadLeft      = 0x0004, // DPad Left
    DPadRight     = 0x0008, // DPad Right
    Start         = 0x0010, // Start / Menu / Options
    Back          = 0x0020, // Back / View / Share
    LeftThumb     = 0x0040, // Left Thumb Button / L3
    RightThumb    = 0x0080, // Right Thumb Button / R3
    LeftShoulder  = 0x0100, // Left Shoulder Button / LB / L1
    RightShoulder = 0x0200, // Right Shoulder Button / RB / R1
    ButtonSouth   = 0x1000, // A on Xbox, B on Nintendo, Cross on PS
    ButtonEast    = 0x2000, // B on Xbox, A on Nintendo, Circle on PS
    ButtonWest    = 0x4000, // X on Xbox, Y on Nintendo, Square on PS
    ButtonNorth   = 0x8000, // Y on Xbox, X on Nintendo, Triangle on PS

    Menu    = Start,
    Options = Start,
    View    = Back,
    Share   = Back,

    LB = LeftShoulder,
    L1 = LeftShoulder,
    RB = RightShoulder,
    R1 = RightShoulder,

    L3 = LeftThumb,
    R3 = RightThumb,

    Cross    = ButtonSouth,
    Circle   = ButtonEast,
    Square   = ButtonWest,
    Triangle = ButtonNorth,
};

enum class JoystickAxisCode
{
    LT,        // Left Trigger
    RT,        // Right Trigger
    LX,        // Left Thumb Horizontal
    LY,        // Left Thumb Vertical
    RX,        // Right Thumb Horizontal
    RY,        // Right Thumb Vertical
    ErrorAxis, // Doesn't exist

    L2 = LT,
    R2 = RT,
};

enum class KeyCode
{
    Backspace,
    Delete,
    Tab,
    Return,
    Esc,
    Space,
    Keypad0,
    Keypad1,
    Keypad2,
    Keypad3,
    Keypad4,
    Keypad5,
    Keypad6,
    Keypad7,
    Keypad8,
    Keypad9,
    KeypadPeriod,
    KeypadDivide,
    KeypadMultiply,
    KeypadMinus,
    KeypadPlus,
    KeypadEnter,
    Up,
    Down,
    Right,
    Left,
    Insert,
    Home,
    End,
    PageUp,
    PageDown,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    F13,
    F14,
    F15,
    Number0,
    Number1,
    Number2,
    Number3,
    Number4,
    Number5,
    Number6,
    Number7,
    Number8,
    Number9,
    Quote,
    Comma,
    Minus,
    Period,
    Slash,
    Semicolon,
    Equal,
    LeftBracket,
    Backslash,
    RightBracket,
    BackQuote,
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,
    NumLock,
    CapsLock,
    ScrollLock,
    RightShift,
    LeftShift,
    RightControl,
    LeftControl,
    RightAlt,
    LeftAlt,
    LeftWindows,
    RightWindows,
    Menu,
    ErrorKey,

    Count
};

enum class MouseButtonCode
{
    Button0,
    Button1,
    Button2,
    Button3,
    Button4,
    Button5,
    Button6,
    Button7,

    Count,

    Left   = Button0,
    Right  = Button1,
    Middle = Button2,
};

enum class MouseMovementCode
{
    X,
    Y,
    ScrollWheel,

    Count
};

} // namespace gore