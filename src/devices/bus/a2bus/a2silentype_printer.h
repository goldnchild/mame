// license:BSD-3-Clause
// copyright-holders: Golden Child
/*
 *  silentype printer
 *
 */
#include "machine/bitmap_printer.h"
#include "machine/steppers.h"

#ifndef MAME_A2BUS_A2SILENTYPE_PRINTER_H
#define MAME_A2BUS_A2SILENTYPE_PRINTER_H

#pragma once

class silentype_printer_device : public device_t
{
public:
	silentype_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
protected:
	silentype_printer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

public:
//	DECLARE_READ_LINE_MEMBER( read_margin_switch ) { return (m_xpos <= 0); }
//	DECLARE_READ_LINE_MEMBER( read_data ) { return 0; } // should return shift register on read (unimplemented)
	
	u8 read_margin_switch () { return (m_bitmap_printer->m_xpos <= 0); }
	u8 read_data () { return 0; } // should return shift register on read (unimplemented)


	void write_data(u8 data);
	void write_shiftclock(u8 data);
	void write_storeclock(u8 data);

	//void update_printhead(uint8_t data);
	//void update_pf_stepper(uint8_t data);
	//void update_cr_stepper(uint8_t data);

protected:
	// device-level overrides

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_reset_after_children() override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;

private:

	const int dpi = 60;
	const int PAPER_WIDTH = 8.5 * dpi;  // 8.5 inches wide at 60 dpi
	const int PAPER_HEIGHT = 11 * dpi;   // 11  inches high at 60 dpi

	//int m_xpos = PAPER_WIDTH / 2 * 2;  // middle of paper (position in half steps)
	//int m_ypos = 0;

	required_device<bitmap_printer_device> m_bitmap_printer;
	
	int right_offset = 0;
	int left_offset = 3;

	double headtemp[7] = {0.0}; // initialize to zero - avoid nan bugs
	int heattime = 4000;   // time in usec to hit max temp  (smaller numbers mean faster)
	int decaytime = 2000;  // time in usec to cool off

	u8 lastheadbits = 0;
	double last_update_time = 0.0;  // strange behavior if we don't initialize

 private:

	u8 m_last_stepper_x = 0;
	double m_last_stepper_x_time = 0.0;
	
	u16	m_shift_reg = 0;
	u16 m_parallel_reg = 0;
	u8 m_datalast = 0;  // serial data bit
	u8 m_shiftclocklast = 0;
	u8 m_storeclocklast = 0;

	void adjust_headtemp(u8 pin_status, double time_elapsed,  double& temp);
	void darken_pixel(double headtemp, unsigned int& pixel);
	void update_printhead(u8 data);
	//int update_stepper_delta(stepper_device * stepper, uint8_t stepper_pattern);
	//s32 ypos_coord(s32 ypos) { return ypos * 7 / 4 / 2; }  // y position given in half steps, full step is 7/4 pixels
};

DECLARE_DEVICE_TYPE(SILENTYPE_PRINTER, silentype_printer_device)

#endif // MAME_A2BUS_A2SILENTYPE_PRINTER_H
