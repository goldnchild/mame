// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    System 23 keyboard emulation

*********************************************************************/


//#define VERBOSE 1

#include "emu.h"
#include "system23_kbd.h"
#include "cpu/mcs48/mcs48.h"

#include "logmacro.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define I8048_TAG       "i8048"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SYSTEM23_KEYBOARD, system23_kbd_device, "system23_kbd", "Datamaster System23 Keyboard")


//-------------------------------------------------
//  ROM( system23_kbd )
//-------------------------------------------------

ROM_START(system23_kbd)
	ROM_REGION(0x400, "i8048", 0)
	ROM_LOAD("kbd_3e88d3bf_8048.bin", 0x0000, 0x0400, CRC(3e88d3bf) SHA1(04884f5d43a940c76bc4d53d2dbd970b80f11fa6))
ROM_END



//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *system23_kbd_device::device_rom_region() const
{
	return ROM_NAME( system23_kbd );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void system23_kbd_device::device_add_mconfig(machine_config &config)
{
//  I8048(config, m_maincpu, MCS48_LC_CLOCK(IND_U(47), CAP_P(20.7)));
	I8048(config, m_maincpu, 4'350'000);//Oscillation between 4.17 and 4.35 MHz
	m_maincpu->bus_in_cb().set(FUNC(system23_kbd_device::bus_r));
	m_maincpu->bus_out_cb().set(FUNC(system23_kbd_device::bus_w));
	m_maincpu->p1_out_cb().set(FUNC(system23_kbd_device::p1_w));
	m_maincpu->p2_out_cb().set(FUNC(system23_kbd_device::p2_w));
	m_maincpu->p2_in_cb().set(FUNC(system23_kbd_device::p2_r));
	m_maincpu->t0_in_cb().set(FUNC(system23_kbd_device::t0_r));
	m_maincpu->t1_in_cb().set(FUNC(system23_kbd_device::t1_r));
}


//-------------------------------------------------
//  INPUT_PORTS( system23_kbd )
//-------------------------------------------------

INPUT_PORTS_START( system23_kbd )
	PORT_START("MD00")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) //PORT_CODE(KEYCODE_PAD) PORT_CHAR('.') //4a dot from pad
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Field +") PORT_CODE(KEYCODE_PLUS_PAD) //55 4d
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR('3') //43
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR('6') //46
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Field -") PORT_CODE(KEYCODE_ENTER_PAD) //55 4e
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR('9') //49
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR('-') //4c
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) // No scan code
	PORT_START("MD01")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0_PAD) PORT_CHAR('0') //40
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR('2') //42
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR('1') //41
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR('5') //45
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR('4') //44
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR('7') //47
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR('8') //48
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) //PORT_CODE(KEYCODE_PLUS_PAD) //4b
	PORT_START("MD02")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Field Exit") PORT_CODE(KEYCODE_RALT) //68 78
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RShift") PORT_CODE(KEYCODE_RSHIFT) //0b 7b
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Carriage Return") PORT_CODE(KEYCODE_RCONTROL) //56 76
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CHAR('{') PORT_CHAR('}') PORT_CODE(KEYCODE_OPENBRACE) // 59 79
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(UCHAR_MAMEKEY(ENTER)) //2d
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR(UCHAR_MAMEKEY(SLASH)) //2c
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(UCHAR_MAMEKEY(BACKSPACE)) //3d
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) // No scan code
	PORT_START("MD03")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) // No scan code
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(".") PORT_CODE(KEYCODE_STOP) //09
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(";") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':') //1a
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("'") PORT_CODE(KEYCODE_QUOTE) //1b
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) //PORT_NAME("Cent") PORT_CODE(KEYCODE_) //2b
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P') //2a
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_') //3b
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+') //3c
	PORT_START("MD04")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') //08
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M') //07
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K') //18
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L') //19
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O') //29
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I') //28
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(') //39
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')') //3a
	PORT_START("MD05")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(UCHAR_MAMEKEY(SPACE)) //0f
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N') //06
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) // No scan code
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J') //17
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U') //27
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) // No scan code
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) // No scan code
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*') //38
	PORT_START("MD06")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V') //04
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B') //05
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H') //16
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G') //15
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T') //25
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y') //26
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&') //37
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^') //36
	PORT_START("MD07")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X') //02
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C') //03
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F') //14
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D') //13
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E') //23
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R') //24
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%') //35
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$') //34
	PORT_START("MD08")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z') //01
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CHAR('<') PORT_CHAR('>') //0e
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') //12
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A') //11
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q') //21
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W') //22
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#') //33
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@') //32
	PORT_START("MD09")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LAlt") PORT_CODE(KEYCODE_LALT)//7e 5e
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) //73
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) //70
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LShift") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)//57 77
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))//54 74
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t') //20
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!') //31
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) //3e TODO
	PORT_START("MD10")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) //72
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Up") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP)) //71
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Test") PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6)) //7d
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Restore / Error") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5)) //6e
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Inq") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3)) //6c
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Hold") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4)) //6d
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Erase / Attention") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1)) //7c
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Command") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2)) //6f 7f
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor system23_kbd_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( system23_kbd );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  system23_kbd_device - constructor
//-------------------------------------------------

system23_kbd_device::system23_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SYSTEM23_KEYBOARD, tag, owner, clock),
//      device_pc_kbd_interface(mconfig, *this),
		m_maincpu(*this, I8048_TAG),
		m_md(*this, "MD%02u", 0),
		m_data_strobe(*this),
		m_bus(0xff),
		m_p1(0xff),
		m_p2(0xff),
		m_sense(0),
		m_q(1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void system23_kbd_device::device_start()
{
	// state saving
	save_item(NAME(m_bus));
	save_item(NAME(m_p1));
	save_item(NAME(m_p2));
	save_item(NAME(m_sense));
	save_item(NAME(m_q));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void system23_kbd_device::device_reset()
{
}



//-------------------------------------------------
//  bus_r -
//-------------------------------------------------

uint8_t system23_kbd_device::bus_r()
{
	// HACK this should be handled in mcs48.cpp
	return m_bus;
}


//-------------------------------------------------
//  bus_w -
//-------------------------------------------------

void system23_kbd_device::bus_w(uint8_t data)
{
	/*

	    bit     description

	    0..6    scancode in bit 0..6 but bit reversed
	    7       sense chip enable

	*/

	if (!BIT(m_bus, 7) && BIT(data, 7))
	{
		uint8_t data = 0xff;

		if (BIT(m_p1, 0)) data &= m_md[0]->read();
		if (BIT(m_p1, 1)) data &= m_md[1]->read();
		if (BIT(m_p1, 2)) data &= m_md[2]->read();
		if (BIT(m_p1, 3)) data &= m_md[3]->read();
		if (BIT(m_p1, 4)) data &= m_md[4]->read();
		if (BIT(m_p1, 5)) data &= m_md[5]->read();
		if (BIT(m_p1, 6)) data &= m_md[6]->read();
		if (BIT(m_p1, 7)) data &= m_md[7]->read();
		if (BIT(m_p2, 4)) data &= m_md[8]->read();
		if (BIT(m_p2, 5)) data &= m_md[9]->read();
		if (BIT(m_p2, 6)) data &= m_md[10]->read();

		m_q = BIT(data, m_sense);
	}

	m_bus = data;
}


uint8_t system23_kbd_device::read_keyboard()
{
	LOG("Read Keyboard %02x\n", m_bus);
	printf("Scan Code: %02x\n", (m_bus ^ 0xff) & 0x7f);
	return m_bus & 0x7f;
}


//-------------------------------------------------
//  p1_w -
//-------------------------------------------------

void system23_kbd_device::p1_w(uint8_t data)
{
	/*
	    bit     description
	    0..7       Matrix Drive MD00..MD07
	*/

	m_p1 = data;
}


//-------------------------------------------------
//  p2_w -
//-------------------------------------------------

void system23_kbd_device::p2_w(uint8_t data)
{
	/*
	    bit     description
	    0..2   SELECT 0..2
	    3       sense test (also connected to t1)
	    4..6    MD08..MD10
	    7       data strobe
	*/

	m_data_strobe(BIT(data, 7));

	if (!BIT(m_p2, 3) && BIT(data, 3))   // clock in the m_sense selection
	{
		m_sense = data & 0x07;
	}

	m_p2 = data;
}

u8 system23_kbd_device::p2_r()
{
	return (m_p2 & ~(1<<3)) | (m_q << 3);
}

//-------------------------------------------------
//  t0_r -
//-------------------------------------------------

int system23_kbd_device::t0_r()
{
	return m_t0;  // delay strobe
}

//-------------------------------------------------
//  t1_r -
//-------------------------------------------------

int system23_kbd_device::t1_r()
{
//  return BIT(m_bus, 7) && m_q;
	return m_q;
}
