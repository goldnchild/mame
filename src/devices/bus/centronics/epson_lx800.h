// license:BSD-3-Clause
// copyright-holders:Dirk Best
/**********************************************************************

    Epson LX-800 dot matrix printer emulation

**********************************************************************/

#ifndef MAME_BUS_CENTRONICS_EPSON_LX800_H
#define MAME_BUS_CENTRONICS_EPSON_LX800_H

#pragma once

#include "ctronics.h"
#include "cpu/upd7810/upd7810.h"
#include "machine/e05a03.h"
#include "machine/bitmap_printer.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> epson_lx800_device

class epson_lx800_device :  public device_t, public device_centronics_peripheral_interface
{
public:
	// construction/destruction
	epson_lx800_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	epson_lx800_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	/* Centronics stuff */
	virtual DECLARE_WRITE_LINE_MEMBER( input_strobe ) override { m_e05a03->centronics_input_strobe(state); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data0 ) override { m_e05a03->centronics_input_data0(state); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data1 ) override { m_e05a03->centronics_input_data1(state); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data2 ) override { m_e05a03->centronics_input_data2(state); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data3 ) override { m_e05a03->centronics_input_data3(state); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data4 ) override { m_e05a03->centronics_input_data4(state); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data5 ) override { m_e05a03->centronics_input_data5(state); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data6 ) override { m_e05a03->centronics_input_data6(state); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data7 ) override { m_e05a03->centronics_input_data7(state); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_init ) override { m_e05a03->centronics_input_init(state); }

	/* Panel buttons */
	DECLARE_INPUT_CHANGED_MEMBER(online_sw);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

	virtual bool supports_pin35_5v() override { return true; }

private:
	uint8_t porta_r(offs_t offset);
	void porta_w(offs_t offset, uint8_t data);
	uint8_t portc_r(offs_t offset);
	void portc_w(offs_t offset, uint8_t data);

	DECLARE_WRITE_LINE_MEMBER(e05a03_centronics_ack) { output_ack(state); }
	DECLARE_WRITE_LINE_MEMBER(e05a03_centronics_busy) { output_busy(state); }
	DECLARE_WRITE_LINE_MEMBER(e05a03_centronics_perror) { output_perror(state); }
	DECLARE_WRITE_LINE_MEMBER(e05a03_centronics_fault) { output_fault(state); }
	DECLARE_WRITE_LINE_MEMBER(e05a03_centronics_select) { output_select(state); }

	uint8_t an0_r();
	uint8_t an1_r();
	uint8_t an2_r();
	uint8_t an3_r();
	uint8_t an4_r();
	uint8_t an5_r();
	uint8_t an6_r();
	uint8_t an7_r();

	/* GATE ARRAY */
	void printhead(uint16_t data);
	void pf_stepper(uint8_t data);
	void cr_stepper(uint8_t data);
	DECLARE_READ_LINE_MEMBER(hp_sensor_r);


//  uint8_t centronics_data_r();
//  DECLARE_WRITE_LINE_MEMBER(centronics_pe_w);
	DECLARE_WRITE_LINE_MEMBER(reset_w);


	DECLARE_WRITE_LINE_MEMBER(co0_w);

	void lx800_mem(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<e05a03_device> m_e05a03;
	required_device<bitmap_printer_device> m_bitmap_printer;

	output_finder<> m_online_led;

	int16_t m_printhead;
	int m_real_cr_steps;
	int m_in_between_offset; // in between cr_stepper phases
	int m_rightward_offset;  // offset pixels when stepper moving rightward

	enum {
		TIMER_CR
	};

	emu_timer *m_cr_timer;

	int wrap(int x, int y) { x = x % y; if (x < 0) x = x + y; return x; }

	static constexpr int CR_OFFSET = (-14);
	static constexpr int PAPER_WIDTH = 1024;     // 120 dpi * 8.5333 inches
	static constexpr int PAPER_HEIGHT = (11*72); //  72 dpi * 11 inches
};


// device type definition
DECLARE_DEVICE_TYPE(EPSON_LX800, epson_lx800_device)

#endif // MAME_BUS_CENTRONICS_EPSON_LX800_H
