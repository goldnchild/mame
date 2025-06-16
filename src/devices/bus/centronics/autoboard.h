// license:BSD-3-Clause
// copyright-holders:
/**********************************************************************

   Saitek Kasparov PC Auto Chessboard Emulation

   8x8 Reed Switch Matrix

   rows connected to 74HCT157 Quad 2-1 data selector
   columns connected to 74HCT137 decoder
   single column selected at a time
   active low when piece is above reed switch
   4 rows read at a time via parallel port inputs

   8x8 LED Matrix

   rows connected to 74HCT259 8 bit addressable latch
   columns connected to 74HCT137 decoder

**********************************************************************/

#ifndef MAME_BUS_CENTRONICS_AUTOBOARD_H
#define MAME_BUS_CENTRONICS_AUTOBOARD_H

#pragma once

#include "bus/centronics/ctronics.h"
#include "machine/74157.h"
#include "machine/74259.h"
#include "machine/sensorboard.h"
#include "video/pwm.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pc_autoboard_device

class pc_autoboard_device :  public device_t,
	public device_centronics_peripheral_interface
{
public:
	// construction/destruction
	pc_autoboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void update() // updates inputs and outputs
	{
		if (machine().ioport().safe_to_read())
		{
			// PC reads the magnetic board 4 rows at at time from the current file
			u8 r = ~(m_board->read_file(m_col, false) >> (m_high_nibble ? 4 : 0));  // switch engaged is active low

			// update status bits
			output_select(BIT(r,0));  // pin 13  D0
			output_perror(BIT(r,1));  // pin 12  D1
			output_ack(BIT(r,2));     // pin 10  D2
			output_busy(BIT(r,3));    // pin 11  D3

			// update led outputs
			if (m_power)
			{
				m_display->write_mx(1 << (7 - m_col));
				m_display->write_my(m_259->output_state());
			}
		}
	}

	virtual void input_strobe(int state) override {  m_high_nibble = state; update(); }  // pin 1 m_157_reed->select_w(state);
	virtual void input_data0(int state) override { if (state) m_col |= 0x01; else m_col &= ~0x01; update(); }  // pin 2
	virtual void input_data1(int state) override { if (state) m_col |= 0x02; else m_col &= ~0x02; update(); }  // pin 3
	virtual void input_data2(int state) override { if (state) m_col |= 0x04; else m_col &= ~0x04; update(); }  // pin 4
	virtual void input_data3(int state) override { } // column enable to 74137 reed switch enable  // pin 5
	virtual void input_data4(int state) override
		{ if (state) m_row |= 0x01; else m_row &= ~0x01; m_259->write_abcd(m_row, 1); update(); }  // pin 6
	virtual void input_data5(int state) override
		{ if (state) m_row |= 0x02; else m_row &= ~0x02; m_259->write_abcd(m_row, 1); update(); }  // pin 7
	virtual void input_data6(int state) override
		{ if (state) m_row |= 0x04; else m_row &= ~0x04; m_259->write_abcd(m_row, 1); update(); }  // pin 8
	virtual void input_data7(int state) override { m_259->clear_w(state); } // pin 9 clear is active LOW
	virtual void input_init(int state)  override { } // pin 16  reed switch 137 latch enable
	virtual void input_autofd(int state) override { m_power = !state; output_fault(m_power); }
	// pin 14 = power on (inverted)
	// pin 15 = power status gets read on pin 15 to check if power is on
	virtual void input_select_in(int state) override { m_259->enable_w(state); m_259->write_abcd(m_row, 1); update(); }
	// pin 17 *led enable, both 137 and 259

private:
	required_device<hct259_device> m_259;
//  required_device<hct137_device> m_137;  // there are two 74137 decoders used for the columns in the led and switch matrixes
//  required_device<hct157_device> m_157_reed;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;

	u8 m_row = 0;
	u8 m_col = 0;
	u8 m_power = 0;
	u8 m_high_nibble = 0;
};

DECLARE_DEVICE_TYPE(CENTRONICS_AUTOCHESSBOARD, pc_autoboard_device)

#endif // MAME_BUS_CENTRONICS_AUTOBOARD_H
