// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_BUS_CENTRONICS_NLQ401_H
#define MAME_BUS_CENTRONICS_NLQ401_H

#pragma once

#include "ctronics.h"
#include "cpu/upd7810/upd7810.h"
#include "machine/tms1024.h"
#include "machine/bitmap_printer.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nlq401_device

class nlq401_device : public device_t, public device_centronics_peripheral_interface
{
public:
	// device type constructor
	nlq401_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::PRINTER; }

	DECLARE_CUSTOM_INPUT_MEMBER( homepos_r );
	DECLARE_CUSTOM_INPUT_MEMBER( ls74_r );

	INPUT_CHANGED_MEMBER( online_sw );
	INPUT_CHANGED_MEMBER( reset_printer );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	// device_centronics_peripheral_interface overrides

	virtual void input_strobe(int state) override
	{
		if ((state != 1) && (last_input_strobe != 0))
		{
			m_ls74_input = 1;
			output_busy(m_pcbusy | m_ls74_input);
			m_maincpu->set_input_line(UPD7810_INTF1, !m_ls74_input ? CLEAR_LINE : ASSERT_LINE);
		}
		last_input_strobe = state;
	}
	virtual void input_data0(int state) override { if (state) m_centronics_data |= 0x01; else m_centronics_data &= ~0x01; }
	virtual void input_data1(int state) override { if (state) m_centronics_data |= 0x02; else m_centronics_data &= ~0x02; }
	virtual void input_data2(int state) override { if (state) m_centronics_data |= 0x04; else m_centronics_data &= ~0x04; }
	virtual void input_data3(int state) override { if (state) m_centronics_data |= 0x08; else m_centronics_data &= ~0x08; }
	virtual void input_data4(int state) override { if (state) m_centronics_data |= 0x10; else m_centronics_data &= ~0x10; }
	virtual void input_data5(int state) override { if (state) m_centronics_data |= 0x20; else m_centronics_data &= ~0x20; }
	virtual void input_data6(int state) override { if (state) m_centronics_data |= 0x40; else m_centronics_data &= ~0x40; }
	virtual void input_data7(int state) override { if (state) m_centronics_data |= 0x80; else m_centronics_data &= ~0x80; }

	virtual void input_busy(int state) override { };
	virtual void input_init(int state) override;
	virtual bool supports_pin35_5v() override { return true; }

private:

	u8 porta_r();
	void portb_w(u8 data);
	u8 portc_r();
	void portc_w(u8 data);
	void portf_w(u8 data);

	u8 an5_r();
	u8 an6_r();
	u8 an7_r();
	void co1_w(int state);


	u8 m_portselect;
	u16 m_printhead;
	u8 m_ls74;        // flipflop for online switch
	u8 m_ls74_input;  // flipflop for centronics input
	u8 m_centronics_data;
	u8 last_input_strobe;
	u8 m_pcbusy;

	void mem_map(address_map &map);

	required_device<upd7810_device> m_maincpu;
	required_device<bitmap_printer_device> m_bitmap_printer;
	required_device<tms1025_device> m_inpexp;
	required_device<tms1025_device> m_outexp;
};

// device type declaration
DECLARE_DEVICE_TYPE(NLQ401, nlq401_device)

#endif // MAME_BUS_CENTRONICS_NLQ401_H
