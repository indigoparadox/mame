// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Sony NEWS keyboard (high-level emulation).
 *
 * Sources:
 *
 *   https://github.com/NetBSD/src/blob/trunk/sys/dev/news/newskeymap.c
 *
 * TODO:
 *   - other languages (esp. Japanese)
 *   - other variations
 *   - dip switches
 */

#include "emu.h"
#include "news_kbd.h"

#include "machine/keyboard.ipp"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(NEWS_HLE_KBD, news_hle_kbd_device, "news_kbd_hle", "Sony NEWS Keyboard (HLE)")

news_hle_kbd_device::news_hle_kbd_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NEWS_HLE_KBD, tag, owner, clock)
	, device_matrix_keyboard_interface(mconfig, *this, "ROW0", "ROW1", "ROW2", "ROW3", "ROW4", "ROW5", "ROW6", "ROW7")
	, m_irq_out_cb(*this)
{
}

void news_hle_kbd_device::device_start()
{
	m_irq_out_cb.resolve_safe();
}

void news_hle_kbd_device::device_reset()
{
	m_fifo.clear();
	out_irq(false);

	reset_key_state();
	start_processing(attotime::from_hz(1'200));
}

void news_hle_kbd_device::key_make(u8 row, u8 column)
{
	LOG("key_make row %d col %d\n", row, column);

	push_key((row << 4) | column);
}

void news_hle_kbd_device::key_break(u8 row, u8 column)
{
	LOG("key_break row %d col %d\n", row, column);

	push_key(0x80 | (row << 4) | column);
}

void news_hle_kbd_device::push_key(u8 code)
{
	m_fifo.enqueue(code);

	out_irq(true);
}

void news_hle_kbd_device::out_irq(bool state)
{
	if (m_irq_out_state != state)
	{
		m_irq_out_state = state;
		m_irq_out_cb(state);
	}
}

u8 news_hle_kbd_device::data_r()
{
	if (m_fifo.empty())
		return 0;

	u8 const data = m_fifo.dequeue();

	if (m_fifo.empty())
		out_irq(false);

	return data;
}

INPUT_PORTS_START(news_hle_kbd_device)
	PORT_START("ROW0")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F1")           PORT_CODE(KEYCODE_F1)         PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F2")           PORT_CODE(KEYCODE_F2)         PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F3")           PORT_CODE(KEYCODE_F3)         PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F4")           PORT_CODE(KEYCODE_F4)         PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F5")           PORT_CODE(KEYCODE_F5)         PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F6")           PORT_CODE(KEYCODE_F6)         PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F7")           PORT_CODE(KEYCODE_F7)         PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F8")           PORT_CODE(KEYCODE_F8)         PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F9")           PORT_CODE(KEYCODE_F9)         PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F10")          PORT_CODE(KEYCODE_F10)        PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc")          PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')

	PORT_START("ROW1")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_0)          PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Backspace")    PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(8)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab")          PORT_CODE(KEYCODE_TAB)        PORT_CHAR(9)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T')

	PORT_START("ROW2")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Delete")       PORT_CODE(KEYCODE_DEL)        PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Control")      PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_A)          PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_D)          PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_G)          PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_H)          PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_J)          PORT_CHAR('j') PORT_CHAR('J')

	PORT_START("ROW3")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_K)          PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_L)          PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Return")       PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L Shift")      PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_X)          PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_C)          PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_V)          PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')

	PORT_START("ROW4")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R Shift")      PORT_CODE(KEYCODE_RSHIFT)     PORT_CHAR(UCHAR_MAMEKEY(RSHIFT))
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Alternate")    PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Caps Lock")    PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Nfer") // muhenkan?
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Space")        PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Xfer") // henkan?
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) // eisu?
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) // kana?
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) // Execute?
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 7")         PORT_CODE(KEYCODE_7_PAD)      PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 8")         PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 9")         PORT_CODE(KEYCODE_9_PAD)      PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP -")         PORT_CODE(KEYCODE_MINUS_PAD)  PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 4")         PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR(UCHAR_MAMEKEY(4_PAD))

	PORT_START("ROW5")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 5")         PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 6")         PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP +")         PORT_CODE(KEYCODE_PLUS_PAD)   PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 1")         PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 2")         PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 3")         PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) // Separator?
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 0")         PORT_CODE(KEYCODE_0_PAD)      PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Up")           PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP .")         PORT_CODE(KEYCODE_DEL_PAD)    PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Enter")        PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left")         PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Down")         PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right")        PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP *")         PORT_CODE(KEYCODE_ASTERISK)   PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP /")         PORT_CODE(KEYCODE_SLASH_PAD)

	PORT_START("ROW6")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP Tab")
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F11")          PORT_CODE(KEYCODE_F11)        PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F12")          PORT_CODE(KEYCODE_F12)        PORT_CHAR(UCHAR_MAMEKEY(F12))
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Help")
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Insert")       PORT_CODE(KEYCODE_INSERT)     PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Clear")
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Prior")        PORT_CODE(KEYCODE_PGUP)       PORT_CHAR(UCHAR_MAMEKEY(PGUP))
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Next")         PORT_CODE(KEYCODE_PGDN)       PORT_CHAR(UCHAR_MAMEKEY(PGDN))
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("ROW7")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

ioport_constructor news_hle_kbd_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(news_hle_kbd_device);
}
