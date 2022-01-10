// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

    E05A03 Gate Array (used in the Epson LX-800)

***************************************************************************/

#include "emu.h"
#include "e05a03.h"


/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

DEFINE_DEVICE_TYPE(E05A03, e05a03_device, "e05a03", "Epson E05A03 Gate Array")

e05a03_device::e05a03_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, E05A03, tag, owner, clock),
	m_write_printhead(*this),
	m_write_pf_stepper(*this),
	m_write_cr_stepper(*this),
	m_read_hp_sensor(*this),
	m_write_nlq_lp(*this),
	m_write_cond_lp(*this),
	m_write_pe_lp(*this),
	m_write_reso(*this),
//  m_write_pe(*this),
//  m_read_data(*this),
	m_write_centronics_ack(*this),
	m_write_centronics_busy(*this),
	m_write_centronics_perror(*this),
	m_write_centronics_fault(*this),
	m_write_centronics_select(*this),
	m_shift(0),
	m_busy_leading(0),
	m_busy_software(0),
	m_nlqlp(0),
	m_cndlp(0),
	#if 0
	m_pe(0),
	m_pelp(0),
	#endif
	m_printhead(0),
	m_pf_motor(0),
	m_cr_motor(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void e05a03_device::device_start()
{
	/* resolve callbacks */
	m_write_nlq_lp.resolve_safe();
	m_write_cond_lp.resolve_safe();
	m_write_pe_lp.resolve_safe();
	m_write_reso.resolve_safe();
//  m_write_pe.resolve_safe();
//  m_read_data.resolve_safe(0);

	m_write_printhead.resolve_safe();
	m_write_pf_stepper.resolve_safe();
	m_write_cr_stepper.resolve_safe();
	m_read_hp_sensor.resolve_safe(0);  // reads have to have a parameter for resolve safe

	m_write_centronics_ack.resolve_safe();
	m_write_centronics_busy.resolve_safe();
	m_write_centronics_perror.resolve_safe();
	m_write_centronics_fault.resolve_safe();
	m_write_centronics_select.resolve_safe();

	/* register for state saving */
	save_item(NAME(m_shift));
	save_item(NAME(m_busy_leading));
	save_item(NAME(m_busy_software));
	save_item(NAME(m_nlqlp));
	save_item(NAME(m_cndlp));
	#if 0
	save_item(NAME(m_pe));
	save_item(NAME(m_pelp));
	#endif
	save_item(NAME(m_printhead));
	save_item(NAME(m_pf_motor));
	save_item(NAME(m_cr_motor));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void e05a03_device::device_reset()
{
	m_printhead = 0x00;
	m_pf_motor = 0x00;
	m_cr_motor = 0x0f;

//  m_write_pe(0);  // connect to m_write_centronics_perror
	m_write_pe_lp(1);

	m_busy_software = 1;
	m_nlqlp = 1;
	m_cndlp = 1;

	/* centronics init */
	m_centronics_nack = false;
	m_centronics_busy = false;
//  m_write_ready_led(get_ready_led());
	m_write_centronics_ack   (!m_centronics_nack);
	m_write_centronics_busy  ( m_centronics_busy);
	m_write_centronics_perror(false);
	m_write_centronics_fault (true);
	m_write_centronics_select(true);

//  m_write_ready(1);

}



/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

void e05a03_device::write(offs_t offset, uint8_t data)
{
	logerror("%s: e05a03_w(%02x): %02x\n", machine().describe_context(), offset, data);

	switch (offset)
	{
	/* shift register */
	case 0x00: m_shift = (m_shift & 0x00ffff) | (data << 16); break;
	case 0x01: m_shift = (m_shift & 0xff00ff) | (data << 8); break;
	case 0x02: m_shift = (m_shift & 0xffff00) | (data << 0); break;

	case 0x03:
		m_busy_leading = BIT(data, 7);
		m_busy_software = BIT(data, 6);
		m_nlqlp = BIT(data, 4);
		m_cndlp = BIT(data, 3);

//      m_write_pe(BIT(data, 2));
		m_write_centronics_perror(BIT(data, 2));
		m_write_pe_lp(!BIT(data, 2));

#if 0
		m_pe = BIT(data, 2);
		m_pelp = !BIT(data, 2);
#endif

		break;

	/* printhead */
	case 0x04: m_printhead = (m_printhead & 0xff00) | (data);
				m_write_printhead(m_printhead ^ 0xffff); break;
	case 0x05: m_printhead = (m_printhead & 0x0ff) | (BIT(data, 7) ? (1 << 8) : 0);
				m_write_printhead(m_printhead ^ 0xffff); break;

	/* paper feed and carriage motor phase data*/
	case 0x06: m_pf_motor = (data & 0xf0) >> 4; m_write_pf_stepper(m_pf_motor); break;
	case 0x07: m_cr_motor = (data & 0x0f) >> 0; m_write_cr_stepper(m_cr_motor); break;
	}
}

uint8_t e05a03_device::read(offs_t offset)
{
	uint8_t result = 0;

	logerror("%s: e05a03_r(%02x)\n", machine().describe_context(), offset);

	switch (offset)
	{
	case 0x00:  // read latched data and clear busy
		m_centronics_data_latched = false;
		m_centronics_busy = false;
		m_write_centronics_busy(m_centronics_busy);
		result = m_centronics_data_latch;
		break;

	case 0x01:
		result = m_read_hp_sensor() << 6 |
				m_centronics_data_latched << 7;
		break;

	case 0x02:  // immediate read of data input
		result = m_centronics_data;
		break;

	case 0x03:
		result |= BIT(m_shift, 23) << 7;
		m_shift <<= 1;
		break;
	}

	return result;
}

/* home position signal */
WRITE_LINE_MEMBER( e05a03_device::home_w )
{
}

/* printhead solenoids trigger */
WRITE_LINE_MEMBER( e05a03_device::fire_w )
{
}

WRITE_LINE_MEMBER( e05a03_device::strobe_w )
{
}

READ_LINE_MEMBER( e05a03_device::busy_r )
{
	return 1;
}

WRITE_LINE_MEMBER( e05a03_device::resi_w )
{
	if (!state)
	{
		device_reset();
		m_write_reso(1);
	}
}

WRITE_LINE_MEMBER( e05a03_device::init_w )
{
	resi_w(state);
}

/***************************************************************************
    Centronics
***************************************************************************/

WRITE_LINE_MEMBER( e05a03_device::centronics_input_strobe )
{
	if (m_centronics_strobe == true && state == false && !m_centronics_busy) {
		m_centronics_data_latch   = m_centronics_data;

		m_centronics_data_latched = true;
		m_centronics_busy         = true;
	//  m_write_ready_led(get_ready_led());
		m_write_centronics_busy(m_centronics_busy);
	}

	m_centronics_strobe = state;
}

WRITE_LINE_MEMBER( e05a03_device::centronics_input_init )
{
	if (m_centronics_init == 1 && state == 0) // when init goes low, do a reset cycle
	{
//      m_write_cpu_reset(0);
//      m_write_cpu_reset(1);
		device_reset(); // this will trigger an NMI after 0.9 seconds
	}
	m_centronics_init = state;
}

