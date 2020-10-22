// license:BSD-3-Clause
// copyright-holders:R. Belmont, Golden Child
/*********************************************************************

    grapplerplus.h

    Orange Micro Grappler Plus Apple II Parallel Interface Card

*********************************************************************/

#ifndef MAME_BUS_A2BUS_GRAPPLERPLUS_H
#define MAME_BUS_A2BUS_GRAPPLERPLUS_H

#pragma once

#include "a2bus.h"
#include "bus/centronics/ctronics.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_grapplerplus_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_grapplerplus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_grapplerplus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;
	virtual uint8_t read_c800(uint16_t offset) override;

	void start_strobe();
	void clear_strobe();

	required_ioport m_dsw1;

	required_device<centronics_device> m_ctx;
	required_device<input_buffer_device> m_ctx_data_in;
	required_device<output_latch_device> m_ctx_data_out;
	required_region_ptr<u8> m_rom;

private:
	DECLARE_WRITE_LINE_MEMBER( ack_w );
	DECLARE_WRITE_LINE_MEMBER( busy_w );
	DECLARE_WRITE_LINE_MEMBER( perror_w );
	DECLARE_WRITE_LINE_MEMBER( select_w );

	bool m_started;
	uint8_t m_ack;
	uint8_t m_busy;
	uint8_t m_perror;
	uint8_t m_select;
	uint8_t m_irqbit;
	bool m_irqenable;
	emu_timer *m_timer;
	uint8_t m_rombank;
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_GRAPPLERPLUS, a2bus_grapplerplus_device)

#endif  // MAME_BUS_A2BUS_GRAPPLERPLUS_H
