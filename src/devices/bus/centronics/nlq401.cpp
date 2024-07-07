// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Schneider NLQ 401 Matrix Printer (skeleton)

    This appears to be an OEM version of a Centronics Printer Corp. GLP 3101
    product. The hardware (but not the firmware) is also said to be
    identical with the Brother M1009.

    DIPSWITCH 1 is not present on the NLQ401 board.

    https://zrk.dk/glp/ has a list of printers based on same design.

    make sure DIP28+DIP27 is set on, because SELECT isn't hooked up.

    SELF TEST can be performed by LF key held down on reset/power up.

    HEX DUMP MODE can be engaged by holding LF and ONLINE on reset/powerup.
      (hex dump mode strangely doesn't seem to do line feeds)

**********************************************************************/

#include "emu.h"
#include "nlq401.h"

#include "cpu/upd7810/upd7810.h"


#define PAPER_WIDTH  1024    // 120 dpi * 8.5333 inches
#define PAPER_HEIGHT (11*72) // 72 dpi * 11 inches


//**************************************************************************
//  DEVICE DEFINITION
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(NLQ401, nlq401_device, "nlq401", "Schneider NLQ 401 Matrix Printer")


//-------------------------------------------------
//  nlq401_device - constructor
//-------------------------------------------------

nlq401_device::nlq401_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NLQ401, tag, owner, clock)
	, device_centronics_peripheral_interface(mconfig, *this)
	, m_maincpu(*this, "maincpu")
	, m_bitmap_printer(*this, "bitmap_printer")
	, m_inpexp(*this, "inpexp")
	, m_outexp(*this, "outexp")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nlq401_device::device_start()
{
	output_fault(1);
	output_select(1);
	output_perror(0);
}


//-------------------------------------------------
//  input_strobe - DATA STROBE line handler
//-------------------------------------------------

/*
void nlq401_device::input_strobe(int state)
{
    // TODO
}
*/

//-------------------------------------------------
//  input_init - INIT line handler
//-------------------------------------------------

void nlq401_device::input_init(int state)
{
	// TODO
}




u8 nlq401_device::porta_r()
{
	return ~m_centronics_data;  // centronics input passes through inverter
}

void nlq401_device::portb_w(u8 data)
{
	m_outexp->write_h(BIT(data, 0, 4));
	m_outexp->write_s(BIT(data, 4, 3));
	m_inpexp->write_s(BIT(data, 4, 3));
	m_outexp->write_std(BIT(data, 7));
	m_portselect = BIT(data, 4,3);  // keep track of portselect for debugging messages (not used otherwise)
}

u8 nlq401_device::portc_r()
{
	return 0x87 | (m_inpexp->read_h() << 3);
}

void nlq401_device::portc_w(u8 data)
{
	m_pcbusy = BIT(data,2);
	output_busy(m_pcbusy | m_ls74_input);
}


void nlq401_device::portf_w(u8 data)
{
	// pf6 is head pin 9
	// pf7 is CRPLS (carriage power)
	m_printhead = (m_printhead & ~(0x01)) | ((BIT(data,6)) << 0);
}

//-------------------------------------------------
//  mem_map - address map for microcontroller
//-------------------------------------------------

void nlq401_device::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("prom", 0);
}

//-------------------------------------------------
//  device_add_mconfig - device-specific config
//-------------------------------------------------

void nlq401_device::device_add_mconfig(machine_config &config)
{
	UPD7810(config, m_maincpu, 11_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &nlq401_device::mem_map);

	m_maincpu->pa_in_cb().set(FUNC(nlq401_device::porta_r));
	m_maincpu->pb_out_cb().set(FUNC(nlq401_device::portb_w));
	m_maincpu->pc_in_cb().set(FUNC(nlq401_device::portc_r));
	m_maincpu->pc_out_cb().set(FUNC(nlq401_device::portc_w));
	m_maincpu->pf_out_cb().set(FUNC(nlq401_device::portf_w));

	m_maincpu->an5_func().set(FUNC(nlq401_device::an5_r));
	m_maincpu->an6_func().set(FUNC(nlq401_device::an6_r));
	m_maincpu->an7_func().set(FUNC(nlq401_device::an7_r));
	m_maincpu->co1_func().set(FUNC(nlq401_device::co1_w));

	TMS1025(config, m_inpexp); // B8 (labeled M50780 on schematic)
	m_inpexp->set_ms(0);
	m_inpexp->read_port1_callback().set_ioport("P1");
	m_inpexp->read_port2_callback().set_ioport("P2");
	m_inpexp->read_port3_callback().set_ioport("P3");
	m_inpexp->read_port4_callback().set_ioport("P4");
	m_inpexp->read_port5_callback().set_ioport("P5");
	m_inpexp->read_port6_callback().set_ioport("P6");

	TMS1025(config, m_outexp); // B2 (labeled M50780 on schematic)
	m_outexp->set_ms(1); // tied to _RESET

	m_outexp->write_port1_callback().set([this] (u8 data)
	{
		m_bitmap_printer->update_cr_stepper(bitswap<4>(data, 0, 2, 1, 3));
	});

	m_outexp->write_port2_callback().set([this] (u8 data)
	{
		m_bitmap_printer->update_pf_stepper(bitswap<4>(data, 3, 1, 2, 0));  // reverse bits for reverse direction
	});

	m_outexp->write_port3_callback().set([this] (u8 data)
	{
		output_ack(BIT(data,2));
		if (BIT(data,3) == 0) m_ls74 = 0;
	});

	m_outexp->write_port4_callback().set([this] (u8 data)
	{
		m_printhead = (m_printhead & ~(0x1E)) | (BIT(data,0,4)) << 1;
	});

	m_outexp->write_port5_callback().set([this] (u8 data)
	{
		m_printhead = (m_printhead & ~(0x1E0)) | (BIT(data,0,4)) << 5;
	});

	m_outexp->write_port7_callback().set([this] (u8 data)
	{
		m_bitmap_printer->set_led_state(bitmap_printer_device::LED_ONLINE, !BIT(data, 0));
		m_bitmap_printer->set_led_state(bitmap_printer_device::LED_READY,  !BIT(data, 0));
		m_bitmap_printer->set_led_state(bitmap_printer_device::LED_ERROR,  !BIT(data, 1));

		if (!BIT(data,3))
		{
			m_ls74_input = 0;
			output_busy(m_pcbusy | m_ls74_input);
			m_maincpu->set_input_line(UPD7810_INTF1, !m_ls74_input ? CLEAR_LINE : ASSERT_LINE);
		}
	});

	BITMAP_PRINTER(config, m_bitmap_printer, PAPER_WIDTH, PAPER_HEIGHT, 120, 72);  // do 72 dpi
	m_bitmap_printer->set_pf_stepper_ratio(1,6);  // pf stepper moves at 216 dpi so at 72dpi half steps
	m_bitmap_printer->set_cr_stepper_ratio(1,1);
}


CUSTOM_INPUT_MEMBER( nlq401_device::homepos_r )
{
	return (m_bitmap_printer->m_xpos < 0);
}

CUSTOM_INPUT_MEMBER( nlq401_device::ls74_r )
{
	return m_ls74;
}

INPUT_CHANGED_MEMBER(nlq401_device::online_sw)
{
	if ((oldval == 0 ) && (newval == 1))
		m_ls74 = 1;  // set flip flop
}

INPUT_CHANGED_MEMBER(nlq401_device::reset_printer)
{
	if (newval)
	{
		m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);  // reset cpu (wasn't working disabled keyboard)
//      m_maincpu->reset();  // this works also
	}
}

//**************************************************************************
//  INPUT PORTS
//**************************************************************************

static INPUT_PORTS_START(nlq401)
	PORT_START("SW1")
	PORT_DIPNAME(0x01, 0x01, "DIP11") PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(0x01, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x02, "DIP12") PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(0x02, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x04, "DIP13") PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(0x04, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x08, 0x08, "DIP14") PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(0x08, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x10, 0x10, "DIP15") PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(0x10, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))

	PORT_START("P1")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("HP SW")  PORT_CUSTOM_MEMBER(nlq401_device, homepos_r)
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_OTHER)
	PORT_BIT(4, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PE SW")  PORT_CODE(KEYCODE_7_PAD) PORT_TOGGLE
	PORT_BIT(8, IP_ACTIVE_HIGH, IPT_UNKNOWN) PORT_NAME("ONLINE SW") PORT_CUSTOM_MEMBER(nlq401_device, ls74_r)

	PORT_START("P2")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("LF SW")  PORT_CODE(KEYCODE_9_PAD)
	PORT_DIPNAME(2, 2, "DIP28") PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(2, DEF_STR(Off))
	PORT_DIPSETTING(0, DEF_STR(On))
	PORT_DIPNAME(4, 4, "DIP27") PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(4, DEF_STR(Off))
	PORT_DIPSETTING(0, DEF_STR(On))
	PORT_DIPNAME(8, 8, "DIP16") PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(8, DEF_STR(Off))
	PORT_DIPSETTING(0, DEF_STR(On))

	PORT_START("P3")
	PORT_DIPNAME(1, 1, "DIP17") PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(1, DEF_STR(Off))
	PORT_DIPSETTING(0, DEF_STR(On))
	PORT_DIPNAME(2, 2, "DIP18") PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(2, DEF_STR(Off))
	PORT_DIPSETTING(0, DEF_STR(On))
	PORT_DIPNAME(4, 4, "DIP26") PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(4, DEF_STR(Off))
	PORT_DIPSETTING(0, DEF_STR(On))
	PORT_DIPNAME(8, 8, "DIP25") PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(8, DEF_STR(Off))
	PORT_DIPSETTING(0, DEF_STR(On))

	PORT_START("P4")
	PORT_DIPNAME(1, 1, "DIP24") PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(1, DEF_STR(Off))
	PORT_DIPSETTING(0, DEF_STR(On))
	PORT_DIPNAME(2, 2, "DIP23") PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(2, DEF_STR(Off))
	PORT_DIPSETTING(0, DEF_STR(On))
	PORT_DIPNAME(4, 4, "DIP22") PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(4, DEF_STR(Off))
	PORT_DIPSETTING(0, DEF_STR(On))
	PORT_DIPNAME(8, 8, "DIP21") PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(8, DEF_STR(Off))
	PORT_DIPSETTING(0, DEF_STR(On))

	PORT_START("P5")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("DSR")
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("CTS")
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(8, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("P6")
	PORT_BIT(1, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("P60 ONLINESW") PORT_CHANGED_MEMBER(DEVICE_SELF, nlq401_device, online_sw, 0) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(8, IP_ACTIVE_LOW, IPT_UNKNOWN)

	// if it's set to IPT_OTHER it always works, even when keyboard is disabled
	// IPT_KEYBOARD keyboard input can be disabled from the keyboard menu

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Reset Printer") PORT_CODE(KEYCODE_8_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, nlq401_device, reset_printer, 0)
INPUT_PORTS_END

//-------------------------------------------------
//  device_input_ports - device-specific ports
//-------------------------------------------------

ioport_constructor nlq401_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(nlq401);
}


//**************************************************************************
//  ROM DEFINITION
//**************************************************************************

ROM_START(nlq401)
	ROM_REGION(0x4000, "prom", 0)
	ROM_LOAD("schneider_nlq401_rev004.bin", 0x0000, 0x4000, CRC(5c331aed) SHA1(b6374abaebb8e484e573caa21b1cc87f1554c8d6))
ROM_END

//-------------------------------------------------
//  device_rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *nlq401_device::device_rom_region() const
{
	return ROM_NAME(nlq401);
}

uint8_t nlq401_device::an5_r()
{
	// not TPCS  (thermal protection sensor?)
	return 0x0;
}

uint8_t nlq401_device::an6_r()
{
	return 0xff;
}

uint8_t nlq401_device::an7_r()
{
	return 0xff;
}

void nlq401_device::co1_w(int state)
{
	/* Printhead is being fired on !state. */
	if (!state)
	{
		for (int i = 0; i < 9; i++)
		{
			if ((m_printhead & (1<<(8-i))) == 0)
				m_bitmap_printer->pix(m_bitmap_printer->m_ypos + i * 1, // * 1 for no interleave at 72 vdpi
				m_bitmap_printer->m_xpos )  = 0x000000;
		}
	}
}
