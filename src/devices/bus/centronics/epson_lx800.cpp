// license:BSD-3-Clause
// copyright-holders:Dirk Best
/**********************************************************************

   Epson LX-800 dot matrix printer

    Skeleton driver

    - CPU type uPD7810HG
    - CPU PORTD and PORTF are connected to the Gate Array
    - processing gets stuck in a loop, and never gets to scan the
      input buttons and switches.
    - CPU disassembly doesn't seem to indicate conditional JR or RET.

**********************************************************************/

#include "emu.h"
#include "epson_lx800.h"
#include "speaker.h"
#include "lx800.lh"
#include "sound/dac.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(EPSON_LX800, epson_lx800_device, "lx800", "Epson LX-800")


//-------------------------------------------------
//  ROM( lx800 )
//-------------------------------------------------

ROM_START( lx800 )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("lx800.ic3c", 0x0000, 0x8000, CRC(da06c45b) SHA1(9618c940dd10d5b43cd1edd5763b90e6447de667) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *epson_lx800_device::device_rom_region() const
{
	return ROM_NAME( lx800 );
}


//-------------------------------------------------
//  ADDRESS_MAP( lx800_mem )
//-------------------------------------------------

void epson_lx800_device::lx800_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom(); /* 32k firmware */
	map(0x8000, 0x9fff).ram(); /* 8k external RAM */
	map(0xa000, 0xbfff).noprw(); /* not used */
	map(0xc000, 0xc007).mirror(0x1ff8).rw("e05a03", FUNC(e05a03_device::read), FUNC(e05a03_device::write));
	map(0xe000, 0xfeff).noprw(); /* not used */
//  map(0xff00, 0xffff).ram(); /* 256 bytes of internal CPU ram */
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void epson_lx800_device::device_add_mconfig(machine_config &config)
{
	/* basic machine hardware */
	upd7810_device &upd(UPD7810(config, m_maincpu, 14.7456_MHz_XTAL));
	upd.set_addrmap(AS_PROGRAM, &epson_lx800_device::lx800_mem);
	upd.pa_in_cb().set(FUNC(epson_lx800_device::porta_r));
	upd.pa_out_cb().set(FUNC(epson_lx800_device::porta_w));
	upd.pb_in_cb().set_ioport("DIPSW1");
	upd.pc_in_cb().set(FUNC(epson_lx800_device::portc_r));
	upd.pc_out_cb().set(FUNC(epson_lx800_device::portc_w));
	upd.an0_func().set(FUNC(epson_lx800_device::an0_r));
	upd.an1_func().set(FUNC(epson_lx800_device::an1_r));
	upd.an2_func().set(FUNC(epson_lx800_device::an2_r));
	upd.an3_func().set(FUNC(epson_lx800_device::an3_r));
	upd.an4_func().set(FUNC(epson_lx800_device::an4_r));
	upd.an5_func().set(FUNC(epson_lx800_device::an5_r));
	upd.an6_func().set(FUNC(epson_lx800_device::an6_r));
	upd.an7_func().set(FUNC(epson_lx800_device::an7_r));
	upd.co0_func().set(FUNC(epson_lx800_device::co0_w));        // pc6 / co0
	upd.co1_func().set("dac", FUNC(dac_bit_interface::write));  // pc7 / co1

//  config.set_default_layout(layout_lx800);

	/* audio hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.25);

	/* gate array */
	e05a03_device &e05a03(E05A03(config, m_e05a03, 0));

	e05a03.pe_lp_wr_callback().set_output("paperout_led");
	e05a03.reso_wr_callback().set(FUNC(epson_lx800_device::reset_w)); //reset out
//  e05a03.pe_wr_callback().set(FUNC(epson_lx800_device::centronics_pe_w));
//  e05a03.data_rd_callback().set(FUNC(epson_lx800_device::centronics_data_r));
	e05a03.printhead().set(FUNC(epson_lx800_device::printhead));
	e05a03.pf_stepper().set(FUNC(epson_lx800_device::pf_stepper));
	e05a03.cr_stepper().set(FUNC(epson_lx800_device::cr_stepper));
//  e05a03.ready().set(FUNC(epson_lx800_device::e05a03_ready));
	e05a03.hp_sensor().set(FUNC(epson_lx800_device::hp_sensor_r));
	e05a03.centronics_ack().set(FUNC(epson_lx800_device::e05a03_centronics_ack));
	e05a03.centronics_busy().set(FUNC(epson_lx800_device::e05a03_centronics_busy));
	e05a03.centronics_perror().set(FUNC(epson_lx800_device::e05a03_centronics_perror));
	e05a03.centronics_fault().set(FUNC(epson_lx800_device::e05a03_centronics_fault));
	e05a03.centronics_select().set(FUNC(epson_lx800_device::e05a03_centronics_select));

	BITMAP_PRINTER(config, m_bitmap_printer, PAPER_WIDTH, PAPER_HEIGHT, 120, 72);  // do 72 dpi
	m_bitmap_printer->set_pf_stepper_ratio(1,6);  // pf stepper moves at 216 dpi so at 72dpi half steps
	m_bitmap_printer->set_cr_stepper_ratio(1,1);
}


//-------------------------------------------------
//  INPUT_PORTS( epson_lx800 )
//-------------------------------------------------

INPUT_PORTS_START( epson_lx800 )
	PORT_START("ONLINE")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("On Line") PORT_CODE(KEYCODE_0_PAD)
	PORT_CHANGED_MEMBER(DEVICE_SELF, epson_lx800_device, online_sw, 0)

	PORT_START("FORMFEED")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Form Feed") PORT_CODE(KEYCODE_7_PAD)

	PORT_START("LINEFEED")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Line Feed") PORT_CODE(KEYCODE_9_PAD)

	PORT_START("DIPSW1")
	PORT_DIPNAME(0x01, 0x00, "Typeface")                      PORT_DIPLOCATION("SW 1:!1")
	PORT_DIPSETTING(   0x00, "Normal")
	PORT_DIPSETTING(   0x01, "Condensed")
	PORT_DIPNAME(0x02, 0x00, "Shape of zero")                 PORT_DIPLOCATION("SW 1:!2")
	PORT_DIPSETTING(   0x00, "Not slashed")
	PORT_DIPSETTING(   0x02, "Slashed")
	PORT_DIPNAME(0x04, 0x00, "Character table")               PORT_DIPLOCATION("SW 1:!3")
	PORT_DIPSETTING(   0x00, "Italics")
	PORT_DIPSETTING(   0x04, "Graphics")
	PORT_DIPNAME(0x08, 0x00, "Paper-out detection")           PORT_DIPLOCATION("SW 1:!4")
	PORT_DIPSETTING(   0x00, "Invalid")
	PORT_DIPSETTING(   0x08, "Valid")
	PORT_DIPNAME(0x10, 0x00, "Printing quality")              PORT_DIPLOCATION("SW 1:!5")
	PORT_DIPSETTING(   0x00, "Draft")
	PORT_DIPSETTING(   0x10, "NLQ")
	PORT_DIPNAME(0xe0, 0xe0, "International character set")   PORT_DIPLOCATION("SW 1:!6,!7,!8")
	PORT_DIPSETTING(   0xe0, "USA")
	PORT_DIPSETTING(   0x60, "France")
	PORT_DIPSETTING(   0xa0, "Germany")
	PORT_DIPSETTING(   0x20, "UK")
	PORT_DIPSETTING(   0xc0, "Denmark")
	PORT_DIPSETTING(   0x40, "Sweden")
	PORT_DIPSETTING(   0x80, "Italy")
	PORT_DIPSETTING(   0x00, "Spain")

	PORT_START("DIPSW2")
	PORT_DIPNAME(0x01, 0x00, "Page length")                   PORT_DIPLOCATION("SW 2:!1")
	PORT_DIPSETTING(   0x00, "11\"")
	PORT_DIPSETTING(   0x01, "12\"")
	PORT_DIPNAME(0x02, 0x00, "Cut-sheet feeder mode")         PORT_DIPLOCATION("SW 2:!2")
	PORT_DIPSETTING(   0x00, DEF_STR(Off))
	PORT_DIPSETTING(   0x02, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x00, "Skip over perforation")         PORT_DIPLOCATION("SW 2:!3")
	PORT_DIPSETTING(   0x00, DEF_STR(Off))
	PORT_DIPSETTING(   0x04, "1\"")
	PORT_DIPNAME(0x08, 0x00, "Auto line feed")                PORT_DIPLOCATION("SW 2:!4")
	PORT_DIPSETTING(   0x00, DEF_STR(Off)) // controlled by /AUTO FEED XT signal (Centronics pin 14)
	PORT_DIPSETTING(   0x08, DEF_STR(On))
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor epson_lx800_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( epson_lx800 );
}

INPUT_CHANGED_MEMBER(epson_lx800_device::online_sw)
{
	m_maincpu->set_input_line(UPD7810_INTF2, newval ? CLEAR_LINE : ASSERT_LINE);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  epson_lx800_device - constructor
//-------------------------------------------------

epson_lx800_device::epson_lx800_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	epson_lx800_device(mconfig, EPSON_LX800, tag, owner, clock)
{
}

epson_lx800_device::epson_lx800_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_centronics_peripheral_interface(mconfig, *this),
	m_maincpu(*this, "maincpu"),
	m_e05a03(*this, "e05a03"),
	m_bitmap_printer(*this, "bitmap_printer"),
	m_online_led(*this, "online_led")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void epson_lx800_device::device_start()
{
	m_cr_timer = timer_alloc(TIMER_CR);
	m_online_led.resolve();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void epson_lx800_device::device_reset()
{
}


/***************************************************************************
    I/O PORTS
***************************************************************************/

/* PA0   W  CRCOM  carriage motor, 0 = holding voltage, 1 = drive voltage
 * PA1             not used
 * PA2   W  PFCOM  paper feed motor, 0 = holding voltage, 1 = drive voltage
 * PA3  R   LF SW  line feed switch
 * PA4  R   FF SW  form feed switch
 * PA5  R   PE SW  paper end sensor, 0 = no paper, 1 = paper
 * PA6             not used
 * PA7  R   P/S    P/S signal from the optional interface
 */
uint8_t epson_lx800_device::porta_r(offs_t offset)
{
	uint8_t result = 0;

	logerror("%s: lx800_porta_r(%02x)\n", machine().describe_context(), offset);

	result |= ioport("LINEFEED")->read() << 3;
	result |= ioport("FORMFEED")->read() << 4;
	result |= 1 << 5;  // paper end

	result |= 1 << 7;  // optional interface

	return result;
}

void epson_lx800_device::porta_w(offs_t offset, uint8_t data)
{
	logerror("%s: lx800_porta_w(%02x): %02x\n", machine().describe_context(), offset, data);
	logerror("--> carriage: %d, paper feed: %d\n", BIT(data, 0), BIT(data, 2));
}

/* PC0   W  TXD        serial i/o txd
 * PC1  R   RXD        serial i/o rxd
 * PC2   W  ONLINE LP  online led
 * PC3  R   ONLINE SW  online switch
 * PC4   W  ERR        centronics error
 * PC5   W  ACK        centronics acknowledge
 * PC6   W  FIRE       drive pulse width signal  co0
 * PC7   W  BUZZER     buzzer signal             co1
 */
uint8_t epson_lx800_device::portc_r(offs_t offset)
{
	uint8_t result = 0;

	logerror("%s: lx800_portc_r(%02x)\n", machine().describe_context(), offset);

	result |= ioport("ONLINE")->read() << 3; // online switch needs to trigger intf2

	return result;
}

void epson_lx800_device::portc_w(offs_t offset, uint8_t data)
{
	logerror("%s: lx800_portc_w(%02x): %02x\n", machine().describe_context(), offset, data);
	logerror("--> err: %d, ack: %d, fire: %d, buzzer: %d\n", BIT(data, 4), BIT(data, 5), BIT(data, 6), BIT(data, 7));

	m_online_led = !BIT(data, 2);
	m_bitmap_printer->set_led_state(bitmap_printer_device::LED_ONLINE, m_online_led);

	int err = !BIT(data, 4);
	m_bitmap_printer->set_led_state(bitmap_printer_device::LED_ERROR, err);

	// pc5 is not acknowledge
	output_ack(!BIT(data, 5));
}

uint8_t epson_lx800_device::an0_r()
{
	return BIT(ioport("DIPSW2")->read(), 0) ? 0 : 255;  // code checks for 0x80 bit so anything >= 0x80
}

uint8_t epson_lx800_device::an1_r()
{
	return BIT(ioport("DIPSW2")->read(), 1) ? 0 : 255;
}

uint8_t epson_lx800_device::an2_r()
{
	return BIT(ioport("DIPSW2")->read(), 2) ? 0 : 255;
}

uint8_t epson_lx800_device::an3_r()
{
	return BIT(ioport("DIPSW2")->read(), 3) ? 0 : 255; // can also read an external line AUTO_FEED_XT
}

uint8_t epson_lx800_device::an4_r()
{
	return 0; // Printer select line (0=always selected)
}

uint8_t epson_lx800_device::an5_r()
{
	return 0xdc; // Monitors 24v line, should return 4.08 volts
				// (1905) MOV A,CR1
				// (190d) LTI A,$DB
				// (1917) LTI A,$FB guessing expects between $DB and $FB
}

uint8_t epson_lx800_device::an6_r()
{
	return 0x1;
}

uint8_t epson_lx800_device::an7_r()
{
	return 0x1;
}

READ_LINE_MEMBER( epson_lx800_device::hp_sensor_r )
{
	return (m_bitmap_printer->m_xpos <= 0) ? 0 : 1;
}

/***************************************************************************
    GATE ARRAY
***************************************************************************/


void epson_lx800_device::printhead(uint16_t data)
{
	m_printhead = data;
}

void epson_lx800_device::pf_stepper(uint8_t data)
{
	m_bitmap_printer->update_pf_stepper(bitswap<4>(data, 0, 1, 2, 3));
}

void epson_lx800_device::cr_stepper(uint8_t data)
{

	[[maybe_unused]] static constexpr u8 cr_pattern[] = { 0xd, 9, 0xb, 0xa, 0xe, 6, 7, 5 };

	m_bitmap_printer->update_cr_stepper(data ^ 0xf);

	m_in_between_offset = 0;

	m_real_cr_steps = 1;
	m_cr_timer->adjust(attotime::from_usec(222), m_bitmap_printer->m_cr_direction);
}

/*
uint8_t epson_lx800_device::centronics_data_r()
{
    logerror("centronics: data read\n");
    return 0x55;
}

WRITE_LINE_MEMBER( epson_lx800_device::centronics_pe_w )
{
    logerror("centronics: pe = %d\n", state);
}
*/

WRITE_LINE_MEMBER( epson_lx800_device::reset_w )
{
	logerror("cpu reset");
	m_maincpu->reset();
}


WRITE_LINE_MEMBER( epson_lx800_device::co0_w )
{
	/* Printhead is being fired on !state. */
	if (!state)
	{
		[[maybe_unused]] static constexpr int pin_pattern[] = { 7, 6, 5, 4, 3, 2, 1, 0, 8 };

		if (m_bitmap_printer->m_xpos < m_bitmap_printer->m_page_bitmap.width())
		{
			for (int i = 0; i < 9; i++)
			{
				if (m_printhead & (1 << (wrap(7 - i, 9))))  // pin pattern: 7 6 5 4 3 2 1 0 8
//              if (m_printhead & (1 << pin_pattern[i]))  // pin pattern: 7 6 5 4 3 2 1 0 8
					m_bitmap_printer->pix(
						m_bitmap_printer->m_ypos + i * 1, // * 1 for no interleave at 72 vdpi
						m_bitmap_printer->m_xpos + CR_OFFSET + m_in_between_offset +
							(m_bitmap_printer->m_cr_direction > 0 ? m_rightward_offset : 0)) = 0x000000;
			}
		}
	}
}


//-------------------------------------------------
//  device_timer - device-specific timer
//-------------------------------------------------

void epson_lx800_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id) {
	case TIMER_CR:
		m_in_between_offset += param;

		if (m_real_cr_steps)
		{
			m_real_cr_steps--;
			m_cr_timer->adjust(attotime::from_usec(222), m_bitmap_printer->m_cr_direction);
		}
		break;
	}
}


