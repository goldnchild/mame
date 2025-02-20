// license:BSD-3-Clause
// copyright-holders:Curt Coder, Golden Child
/**********************************************************************

    System 23 Datamaster Keyboard

      (very similar to IBM Model F PC/XT 83-key keyboard emulation)

*********************************************************************/

#ifndef MAME_IBM_SYSTEM23_KBD_H
#define MAME_IBM_SYSTEM23_KBD_H

#pragma once

#include "cpu/mcs48/mcs48.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> system23_kbd_device

class system23_kbd_device : public device_t
{
public:
	// construction/destruction
	system23_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto data_strobe() { return m_data_strobe.bind(); };
	u8 read_keyboard();
	void delay_strobe(int state){ m_t0 = state; };  // read on t0
	void reset_w(int state){ m_maincpu->set_input_line(INPUT_LINE_RESET, state);};

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	uint8_t bus_r();
	void bus_w(uint8_t data);
	void p1_w(uint8_t data);
	void p2_w(uint8_t data);
	u8 p2_r();
	int t0_r();
	int t1_r();

	required_device<i8048_device> m_maincpu;
	required_ioport_array<11> m_md;
	devcb_write_line m_data_strobe;

	uint8_t m_bus;
	uint8_t m_p1;
	uint8_t m_p2;
	int m_sense;
	int m_q;
	u8 m_t0;
};

// device type definition
DECLARE_DEVICE_TYPE(SYSTEM23_KEYBOARD, system23_kbd_device)

#endif // MAME_IBM_SYSTEM23_KBD_H
