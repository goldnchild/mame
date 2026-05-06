// license:BSD-3-Clause
// copyright-holders:Golden Child
/******************************************************************************

    Apple 410 Color Plotter


memory locations:
c000-c001  16 bit y position  (range from 0-2394  0x0-0x095a initial=0x2bc (700))
c002-c003  16 bit x position  (range from 0-1759  0x0-0x06df initial=0x0)
c004 y stepper phase 0-7
c005 y direction (either ff or 01)
c006 x stepper phase 0-7
c007 x direction (either ff or 01)
c00c current stepper pattern
c00e current writing to leds/head position
c071  pen up/down
c072  pen color

441d stepper drive table
c1cf-c2d7   command buffer
c2d8 command buffer pointer begin
c2da command buffer pointer end

0000-5fff rom
c000-c7ff 2k ram
e000 : write steppers, read panel switches
e800 : write panel leds and pen up/down, read home position and dip switches
f000-f001 : i8251 read/write

Baud clock is handled by dip switches that feed into an TC4512BP 8-way selector
that takes the outputs from a TC4024BP 7 stage binary counter and selects the appropriate
divisor.   There is an 74LS293 counter that it may take an input from.

standard pen colors:
position 1 : black
position 2 : red
position 3 : green
position 4 : blue

home sensor does double duty to discover the pen position:
When penposition is the rotation before the final home rotation,
it sticks out just a small amount more and activates the switch early.
It must be more than 10 half steps, comparison occurs at 409b.

./mame apple2e -sl2 ssc  -sl2:ssc:rs232 a410  -override_artwork apple2e_apple410

emu.keypost([[100 PR#2
110 PRINT"MA 0,0"
120 PRINT"DA 1500,0,1500,1500,0,1500,0,0"
150 FOR J = 1 TO 1000
200 X=INT(RND(1)*1500) : Y=INT(RND(1)*1500)
300 PRINT "DA "X","Y
350 NEXT
400 PR#0
]])

focus 1
bp 411c,1,{printf "411c = move right %x units",bc;g}
bp 4131,1,{printf "4131 = move left %x units",bc;g}
bp 40d4,1,{printf "40d4 = move left until sensor count from %x",bc;g}
bp 408f,1,{printf "408f = bc = %x return sensor count from %x",bc,bc;g}
bp 4063,1,{printf "4063 = reset phase and check home position";g}
bp 40a4,1,{printf "%x 40a4 = exit the process of finding the homepos R C8 L 4b",pc;g}
bp 409e,1,{printf "next d = %x",d;g}
bp 4096,1,{printf "hl = %x",hl;g}
bp 4099,1,{printf "bc = %x  now do subtract bc from hl",bc;g}

Notes:
    You have to have the Remote button engaged to have it respond to commands from the computer.
    I have made it a toggle button on Keypad 9.

    Don't forget to enable the keyboard in the menus.

    This driver is based on guesswork as there is no schematic available.
    Most of the information comes from phooky at nycresistor.  (Thanks phooky!)

Specifications:
    Model Number 410
    Plotting Method X- axis paper motion combined with
    Y-axis pen motion
    Driving Method Stepper motors
    Pen Type Liquid ink, porous plastic tip
    Pen Selection 4 in head, software selectable
    Relative Position Accuracy+ 1% + 0.3 mm
    Position Repeatability Same pen: + 0.2 mm
    Different pen: + 0.4 mm
    Position Resolution 0.1 mm
    Plotting Speedup to 10 cm/sec in either axis
    Maximum Plotting Area X axis: 391.8 mm Y axis: 257.0 mm
    Paper Width (Y axis) Minimum: 120 mm Maximum : 300 mm
    Paper Thickness Minimum: 0.05 mm (0.002") Maximum: 0.8 mm (0.031 ")

******************************************************************************/

#include "emu.h"
#include "rs232.h"
#include "apple_410.h"
#include "machine/i8251.h"
#include "machine/bitmap_printer.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/z80/z80.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

namespace {

class apple_410_device :  public device_t, public device_rs232_port_interface
{
public:
	static constexpr feature_type unemulated_features() { return feature::PRINTER; }

	// construction/destruction
	apple_410_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void init_serial()
	{
		output_dcd(0);  // seems to be needed by apple2 ssc
	}

	void rxready(u8 state)
	{
		//printf("RXREADY=%x\n",state);
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, !state ? CLEAR_LINE : ASSERT_LINE);
	}

	virtual void input_rts(int state) override
	{
		m_i8251->write_cts(state);
		printf("input_rts = %x %s   TIME=%f\n",state, machine().describe_context().c_str(),machine().time().as_double());
	}

	virtual void input_txd(int state) override
	{
		printf("INPUT TXD = %x\n",state);
		m_i8251->write_rxd(state);
	}

	void mem_map(address_map &map);

	u8 m_lastdiff = 0;

	void advancepen()
	{
		m_penposition = (m_penposition + (4 * m_pensteps) + m_penincrement) %
			(4 * m_pensteps);
		m_pencolor = m_penposition / 2;
		m_bitmap_printer->set_printhead_color(m_colors[m_pencolor],0xff8800);
		//printf("PENPOSITION = %x\n", m_penposition);
		m_output_pen_color = m_pencolor;
		m_output_pen_pos = m_penposition + 8 * m_pen_down;
	}

	void detectdirchange(int newx, int oldx)
	{
		int diff = newx - oldx;
		if (diff != 0)
			if (diff != m_lastdir)
			{
				if (diff > 0 && newx < m_penchangethreshold)
					advancepen();
				//printf("DIR change  m_xpos = %x  diff = %x\n", oldx, diff);
				m_lastdir = diff;
			}
		if (diff == 0 && m_lastdiff != 0)
		  //printf("DIRSTOP m_xpos = %x\n",newx);
		m_lastdiff = diff;
	}

	void writestepper(u8 data)
	{
		[[maybe_unused]] int oldx = m_bitmap_printer->m_xpos;
		[[maybe_unused]] int oldy = m_bitmap_printer->m_ypos;

		m_bitmap_printer->update_pf_stepper(bitswap<4>(BIT(data,0,4),0,1,2,3));
		m_bitmap_printer->update_cr_stepper(BIT(data,4,4));

		[[maybe_unused]] int newx = m_bitmap_printer->m_xpos;
		[[maybe_unused]] int newy = m_bitmap_printer->m_ypos;

		if (m_pen_down)
		{
			m_bitmap_printer->drawline(oldx, oldy, newx, newy, m_colors[m_pencolor]);
		}

		detectdirchange(newx, oldx);

		m_output_xpos = newx;
		m_output_ypos = newy;
	}

	void writepanel(u8 data)
	{
		m_pen_down = BIT(data,0);  // ha!  it works!!! it's the pen down bit
		m_output_pen_pos = m_penposition + 8 * m_pen_down;
		m_output_led_remote  = BIT(data, 2);
		m_output_led_free = BIT(data, 3);
		m_output_led_error = BIT(data, 5);
		m_output_panel = data;
		//printf("WRITEPANEL = %x   %s   %f\n",data, machine().describe_context().c_str(), machine().time().as_double());
	}

	INPUT_CHANGED_MEMBER(savepage_switch)
	{
		if (newval)
		{
			m_bitmap_printer->write_snapshot_to_file();
		}
	}

	INPUT_CHANGED_MEMBER(clearpage_switch)
	{
		if (newval)
		{
			m_bitmap_printer->clear_page();
		}
	}

	constexpr static double baseclock = 4.9152e6;
	constexpr static double bauddelay[8] =  // all values here will be multiplied internally by 16 by the i8251
	{
		1.0 / (baseclock / 32   * 2),  // half cycle time, so multiply by 2
		1.0 / (baseclock / 64   * 2),
		1.0 / (baseclock / 128  * 2),
		1.0 / (baseclock / 256  * 2),
		1.0 / (baseclock / 512  * 2),
		1.0 / (baseclock / 1024 * 2),
		1.0 / (baseclock / 2048 * 2),
		1.0 / (baseclock / 4096 * 2)
	};

protected:
	// constructor to pass along a device type
	apple_410_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_reset_after_children() override ATTR_COLD
	{
		m_bitmap_printer->pf_stepper()->set_absolute_position(800);  // initial m_xpos is 400
		m_bitmap_printer->cr_stepper()->set_absolute_position(500);  // initial m_ypos is 250
	}

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_device<i8251_device> m_i8251;
	required_device<bitmap_printer_device> m_bitmap_printer;

	output_finder<> m_output_led_error;
	output_finder<> m_output_led_free;
	output_finder<> m_output_led_remote;
	output_finder<> m_output_pen_color;
	output_finder<> m_output_pen_pos;
	output_finder<> m_output_panel;
	output_finder<> m_output_xpos;
	output_finder<> m_output_ypos;

	static constexpr int PAPER_WIDTH = 120 * 8.5; // 72 dpi * 4 and 1/8 inches
	static constexpr int PAPER_HEIGHT = (11*120); //  72 dpi * 11 inches
public:
	u8 home_sensor();

protected:
	u8 m_txd = 0;

	int m_lastdir = -1;

	u8 m_penchangethreshold = 50 + 20 +2;

	s8 m_penincrement = -1;  // must be either 1 or -1  (specifies direction of rotation)

	u8 m_pencolor = 0;
	s8 m_penposition = 0;  // pencolor = penposition / 3
	u8 m_pensteps = 2;
	//                      black          b           g           r
	u32 m_colors[4] = {   0x00000000 , 0x000000ff, 0x0000ff00, 0x00ff0000 };

	u8 m_pen_down = 0;

	emu_timer *m_baud_timer;
	u8 m_timer_output = 0;

	TIMER_CALLBACK_MEMBER(baud_timer)
	{
		m_timer_output = !m_timer_output;
		m_i8251->write_rxc(m_timer_output);
		m_i8251->write_txc(m_timer_output);
		m_baud_timer->adjust(attotime::from_double(bauddelay[ioport("BAUD")->read() & 0x7]));
	}
};

//-------------------------------------------------
//  ROM( apple_410 )
//-------------------------------------------------

ROM_START( apple_410 )
	ROM_REGION(0x6000, "maincpu", 0)
	ROM_LOAD("b9801yl.bin", 0x0000, 0x2000, CRC(b8996c26) SHA1(37cc6ab365be99c9776679d05fb7de1a711f1e50))
	ROM_LOAD("b9801ym.bin", 0x2000, 0x2000, CRC(228adde0) SHA1(bfd3bcace101ffa50331696cea3ae8df4c4eece6))
	ROM_LOAD("b9801yn.bin", 0x4000, 0x2000, CRC(07505ce4) SHA1(4b6c1c608e922f150c1674e8e4670fb701f1a907))
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *apple_410_device::device_rom_region() const
{
	return ROM_NAME( apple_410 );
}

void apple_410_device::mem_map(address_map &map)
{
	 map(0x0000, 0x5fff).rom().region("maincpu", 0);
	 map(0xc000, 0xc7ff).ram(); // 2k ram mb8128
	 map(0xe000, 0xe000).w(FUNC(apple_410_device::writestepper));
	 map(0xe000, 0xe000).portr("PANEL");
	 map(0xe800, 0xe800).portr("E800");
	 map(0xe800, 0xe800).w(FUNC(apple_410_device::writepanel));
	 map(0xf000, 0xf001).rw(m_i8251, FUNC(i8251_device::read), FUNC(i8251_device::write));
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void apple_410_device::device_add_mconfig(machine_config &config)
{
	z80_device &main(Z80(config, m_maincpu, 4.9152_MHz_XTAL / 2));
	main.set_addrmap(AS_PROGRAM, &apple_410_device::mem_map);

	I8251(config, m_i8251, DERIVED_CLOCK(1, 1));
	m_i8251->dtr_handler().set(FUNC(apple_410_device::output_dsr));  // there is no cts handler
	m_i8251->rts_handler().set(FUNC(apple_410_device::output_cts));
	m_i8251->rxrdy_handler().set(FUNC(apple_410_device::rxready));

	BITMAP_PRINTER(config, m_bitmap_printer, PAPER_WIDTH, PAPER_HEIGHT, 120, 72);  // do 72 dpi
	m_bitmap_printer->set_pf_stepper_ratio(1,2);
	m_bitmap_printer->set_cr_stepper_ratio(1,2);
}

//-------------------------------------------------
//  INPUT_PORTS( apple_410 )
//-------------------------------------------------

INPUT_PORTS_START( apple_410 )

	PORT_START("PANEL")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Free") PORT_CODE(KEYCODE_7_PAD) // Free
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Pen Select") PORT_CODE(KEYCODE_3_PAD) // Pen Select
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Pen Up/Down") PORT_CODE(KEYCODE_1_PAD) // Pen Up/Down
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_4_PAD) // Left
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_2_PAD) // Down
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_6_PAD) // Right
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_8_PAD) // Up
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Local") PORT_CODE(KEYCODE_9_PAD) PORT_TOGGLE // Local  is this a toggle switch?

	PORT_START("BAUD")
	PORT_DIPNAME(0x07, 0x00, "Baud Rate") PORT_DIPLOCATION("DIPSW1:3,2,1")
	PORT_DIPSETTING(0x00, "9600")
	PORT_DIPSETTING(0x01, "4800")
	PORT_DIPSETTING(0x02, "2400")
	PORT_DIPSETTING(0x03, "1200")
	PORT_DIPSETTING(0x04, "600")
	PORT_DIPSETTING(0x05, "300")
	PORT_DIPSETTING(0x06, "150")
	PORT_DIPSETTING(0x07, "75")

	PORT_START("E800")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH,  IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(apple_410_device::home_sensor))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )  // does this make it high?

	PORT_DIPNAME(0x04, 0x04, "Data Bits") PORT_DIPLOCATION("DIPSW1:8")
	PORT_DIPSETTING(0x00, "7")
	PORT_DIPSETTING(0x04, "8")

	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )  // does this make it high? yes

	PORT_DIPNAME(0x10, 0x00, "Parity") PORT_DIPLOCATION("DIPSW1:7")
	PORT_DIPSETTING(0x00, "None")
	PORT_DIPSETTING(0x10, "Parity Enable")

	PORT_DIPNAME(0x20, 0x00, "Odd/Even Parity") PORT_DIPLOCATION("DIPSW1:6")
	PORT_DIPSETTING(0x00, "Odd")
	PORT_DIPSETTING(0x20, "Eveh")

	PORT_DIPNAME(0xC0, 0x40, "Stop") PORT_DIPLOCATION("DIPSW1:5,4")
	PORT_DIPSETTING(0x00, "Invalid")
	PORT_DIPSETTING(0x40, "1 Stop Bit")
	PORT_DIPSETTING(0x80, "1.5 Stop Bits")
	PORT_DIPSETTING(0xC0, "2 Stop Bits")

	PORT_START("SAVEPAGE")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Save page") PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(apple_410_device::savepage_switch), 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Clear page") PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(apple_410_device::clearpage_switch), 0)

INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor apple_410_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( apple_410 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  apple_410_device - constructor
//-------------------------------------------------

apple_410_device::apple_410_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	apple_410_device(mconfig, APPLE_410, tag, owner, clock)
{
}

// constructor that passes device type
apple_410_device::apple_410_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_rs232_port_interface(mconfig, *this),
	m_maincpu(*this, "maincpu"),
	m_i8251(*this, "i8251"),
	m_bitmap_printer(*this, "bitmap_printer"),
	m_output_led_error(*this, "led_error"),
	m_output_led_free(*this, "led_free"),
	m_output_led_remote(*this, "led_remote"),
	m_output_pen_color(*this, "pen_color"),
	m_output_pen_pos(*this, "pen_pos"),
	m_output_panel(*this, "panel"),
	m_output_xpos(*this, "xpos"),
	m_output_ypos(*this, "ypos")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void apple_410_device::device_start()
{
	m_baud_timer = timer_alloc(FUNC(apple_410_device::baud_timer), this);
	m_baud_timer->adjust(attotime::zero);

	m_output_led_error.resolve();
	m_output_led_free.resolve();
	m_output_led_remote.resolve();
	m_output_pen_color.resolve();
	m_output_pen_pos.resolve();
	m_output_panel.resolve();
	m_output_xpos.resolve();
	m_output_ypos.resolve();

	init_serial();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void apple_410_device::device_reset()
{
}

u8 apple_410_device::home_sensor()
{
	return (!(m_bitmap_printer->m_xpos <= (50 + ( m_penposition == 1 ? 7 : 0))));  // active low
	// checks for more than 10 half steps difference for pen home sensing (5 full steps), 7 seems to work
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(APPLE_410, device_rs232_port_interface, apple_410_device, "apple_410", "Apple 410 Plotter")

