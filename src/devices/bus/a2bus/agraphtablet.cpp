// license:BSD-3-Clause
// copyright-holders: Golden Child
/*********************************************************************

    agraphtablet.cpp

    Implementation of the Apple Graphics Tablet

*********************************************************************/

#include "emu.h"
#include "agraphtablet.h"
#include "ioport.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2BUS_AGRAPHTABLET, a2bus_agraphtablet_device, "a2agraphtablet", "Apple Graphics Tablet")

#define AGRAPHTABLET_ROM_REGION  "rom"

ROM_START( AGRAPHTABLET )
	ROM_REGION(0x800, AGRAPHTABLET_ROM_REGION, 0)
	ROM_LOAD( "341-0026-00.bin", 0x000000, 0x000800, CRC(3dda85de) SHA1(b025f03bb5a9316d4a50bca5503ad4825389518f))
ROM_END


#define MOUSE_BUTTON_TAG    "a2mse_button"
#define MOUSE_XAXIS_TAG     "a2mse_x"
#define MOUSE_YAXIS_TAG     "a2mse_y"



#define JOYSTICK_DELTA          80
#define JOYSTICK_SENSITIVITY    50
#define JOYSTICK_AUTOCENTER     80

static INPUT_PORTS_START( apple2_joystick3 )
		PORT_START("joystick_3_x")      /* Joystick 3 X Axis */
		PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X) PORT_NAME("P3 Joystick X")
		PORT_SENSITIVITY(JOYSTICK_SENSITIVITY)
		PORT_KEYDELTA(JOYSTICK_DELTA)
		PORT_CENTERDELTA(JOYSTICK_AUTOCENTER)
		PORT_MINMAX(0,0xff) PORT_PLAYER(3)
		PORT_CODE(MOUSECODE_X)
		PORT_CODE_DEC(KEYCODE_4_PAD)    PORT_CODE_INC(KEYCODE_6_PAD)
		PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH)    PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH)

		PORT_START("joystick_3_y")      /* Joystick 3 Y Axis */
		PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y) PORT_NAME("P3 Joystick Y")
		PORT_SENSITIVITY(JOYSTICK_SENSITIVITY)
		PORT_KEYDELTA(JOYSTICK_DELTA)
		PORT_CENTERDELTA(JOYSTICK_AUTOCENTER)
		PORT_MINMAX(0,0xff) PORT_PLAYER(3)
		PORT_CODE(MOUSECODE_Y)
		PORT_CODE_DEC(KEYCODE_8_PAD)    PORT_CODE_INC(KEYCODE_2_PAD)
		PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH)      PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH)

		PORT_START("joystick_3_buttons")
		PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2)  PORT_PLAYER(3)            PORT_CODE(KEYCODE_0_PAD)    PORT_CODE(JOYCODE_BUTTON1)
		PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1)  PORT_PLAYER(3)            PORT_CODE(KEYCODE_ENTER_PAD)PORT_CODE(JOYCODE_BUTTON2)
INPUT_PORTS_END

/*
static INPUT_PORTS_START( mouse )
        PORT_START(MOUSE_BUTTON_TAG) // Mouse - button
        PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Mouse Button") PORT_CODE(MOUSECODE_BUTTON1)

        PORT_START(MOUSE_XAXIS_TAG) // Mouse - X AXIS
        PORT_BIT( 0xffff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(40) PORT_KEYDELTA(0) PORT_PLAYER(1)
        PORT_CODE(MOUSECODE_X)

        PORT_START(MOUSE_YAXIS_TAG) // Mouse - Y AXIS
        PORT_BIT( 0xffff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(40) PORT_KEYDELTA(0) PORT_PLAYER(1)
        PORT_CODE(MOUSECODE_Y)
INPUT_PORTS_END
*/

static INPUT_PORTS_START(gfxall)
//    PORT_INCLUDE(mouse)
	PORT_INCLUDE(apple2_joystick3)
INPUT_PORTS_END

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

ioport_constructor a2bus_agraphtablet_device::device_input_ports() const
{
	return INPUT_PORTS_NAME (gfxall);
	// return INPUT_PORTS_NAME( apple2_joystick3 );
	// return INPUT_PORTS_NAME(mouse);

}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_agraphtablet_device::device_add_mconfig(machine_config &config)
{
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *a2bus_agraphtablet_device::device_rom_region() const
{
	return ROM_NAME( AGRAPHTABLET );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_agraphtablet_device::a2bus_agraphtablet_device(const machine_config &mconfig, device_type type, const char * tag, device_t * owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_rom(*this, AGRAPHTABLET_ROM_REGION),
	m_joy3x(*this, "joystick_3_x"),
	m_joy3y(*this, "joystick_3_y"),
	m_joy3buttons(*this, "joystick_3_buttons"),
	m_xpos(*this, "tablet_xpos"),
	m_ypos(*this, "tablet_ypos")
//  m_mouseb(*this, MOUSE_BUTTON_TAG),
//  m_mousexy(*this, { MOUSE_XAXIS_TAG, MOUSE_YAXIS_TAG })
{
}

a2bus_agraphtablet_device::a2bus_agraphtablet_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		a2bus_agraphtablet_device(mconfig, A2BUS_AGRAPHTABLET, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_agraphtablet_device::device_start()
{
	save_item(NAME(m_tabletmin));
	save_item(NAME(m_tabletmax));
	save_item(NAME(m_tabletoffscale));
	save_item(NAME(m_timervalue));
	m_xpos.resolve();
	m_ypos.resolve();
	
}

void a2bus_agraphtablet_device::device_reset()
{
}


/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

uint8_t a2bus_agraphtablet_device::read_c0nx(uint8_t offset)
{

	// printf(" Joystick 3  %02x %02x %02X\n",m_joy3x->read(),m_joy3y->read(),m_joy3buttons->read());
	// printf(" MOUSE  %02x %02x %02X\n",m_mousexy[0]->read(),m_mousexy[1]->read(),m_mouseb->read());

	int value = 0;
	if (offset == 0)       // DEV1 C080,X
	{
		// select x axis  start timer
		m_timervalue = (m_tabletmax - m_tabletmin) / 255.0 * m_joy3x->read() + m_tabletmin;
		if (m_joy3buttons->read() & 0x10) { m_timervalue = m_tabletoffscale; }
		m_xpos = m_timervalue;
		return value;
	}
	else if (offset == 1)  // DEV0 C081,X
	{
		// select y axis  start timer
		m_timervalue = (m_tabletmax - m_tabletmin) / 255.0 * m_joy3y->read() + m_tabletmin;
		if (m_joy3buttons->read() & 0x10) { m_timervalue = m_tabletoffscale; }
		m_ypos = m_timervalue;
		return value;
	}
	else if (offset == 2)  // DEV3 C082,X  = low 4 bits of counter
	{
		// Return a value if the pen is removed from the surface (simulated here by pressing a joystick button)
		// reason we XOR is because the outputs from the 74S161 are inverted by gating through an LS368
		//u8 xorvalue = 0x0f;  // clearly creates unstable position values
		u8 xorvalue = 0x07;
		value = ((m_timervalue & 0x0f)) ^ xorvalue;  // invert only the 3 lowest bits 
		// the msb of the lower four doesn't get inverted, because of the way it is used to clock both of the 85L54 chips
		// clock of the 85L54 works on positive transition
		// 0 1111   
		// 1 0000  <- increments on negative transition   (don't care about the first countup being missedss
		return value;
	}
	else if (offset == 3)  // DEV2 C083,X = high 8 bits of counter
	{
		// return high 8 bits of counter
		value = (m_timervalue & 0xff0) >> 4;

		// reading DEV2 will load the pen status into bit 0 and clear the low 4 bits
		m_timervalue = (m_timervalue & 0xff0) | ((m_joy3buttons->read()&0x20) ? 0 : 1);
		return value;
	}
	else return 0x00;
}

/*-------------------------------------------------
  write_c0nx - called for writes to this card's c0nx space
  -------------------------------------------------*/

void a2bus_agraphtablet_device::write_c0nx(uint8_t offset, uint8_t data)
{
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

uint8_t a2bus_agraphtablet_device::read_cnxx(uint8_t offset)
{
	return m_rom[slotno() * 0x100 + offset];  // entry code is repeated in the rom at offset 0xN00 for slot N
}

/*-------------------------------------------------
    write_cnxx - called for writes to this card's cnxx space
-------------------------------------------------*/
void a2bus_agraphtablet_device::write_cnxx(uint8_t offset, uint8_t data)
{
}

/*-------------------------------------------------
    read_c800 - called for reads from this card's c800 space
-------------------------------------------------*/

uint8_t a2bus_agraphtablet_device::read_c800(uint16_t offset)
{
	return m_rom[offset];
}

/*-------------------------------------------------
    write_c800 - called for writes to this card's c800 space
-------------------------------------------------*/
void a2bus_agraphtablet_device::write_c800(uint16_t offset, uint8_t data)
{
}

