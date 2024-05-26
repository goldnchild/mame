// license:BSD-3-Clause
// copyright-holders:Golden Child
#ifndef MAME_BUS_IMAGEWRITER2_PRINTER_H
#define MAME_BUS_IMAGEWRITER2_PRINTER_H

#pragma once

#include "rs232.h"
#include "cpu/upd7810/upd7810.h"
#include "machine/bitmap_printer.h"
#include "machine/steppers.h"
#include "machine/timer.h"

class apple_imagewriter2_printer_device : public device_t,
	public device_rs232_port_interface
{
public:
	apple_imagewriter2_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_INPUT_CHANGED_MEMBER(reset_sw);
protected:
	apple_imagewriter2_printer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual void device_start() override;
	virtual void device_reset() override;

	void mem_map(address_map &map);
	


private:

	required_device<upd7807_device> m_maincpu;

	required_device<bitmap_printer_device> m_bitmap_printer;
	required_device<stepper_device> m_pf_stepper;
	required_device<stepper_device> m_cr_stepper;

	required_device<timer_device> m_timer_clk64;

	output_finder<> m_pa_led;
	output_finder<> m_pb_led;
	output_finder<> m_pc_led;
	output_finder<> m_pt_led;

	void porta_w(uint8_t data);
	void portb_w(uint8_t data);
	uint8_t portt_r();


	int ioportsaferead(const char * name);

	TIMER_DEVICE_CALLBACK_MEMBER (pulse_clk64_clock);

	uint8_t gatearray_r(offs_t offset);
	void gatearray_w(offs_t offset, uint8_t data);

	int xdirection = 0;
	int newpageflag = 0;
	int page_count = 0;

	XTAL baseCLK = 9.8304_MHz_XTAL;  // base clock to 8085 cpu = 9.8304 Mhz
	XTAL CLK2 = baseCLK / 2;         // CLK2 name from Sams schematic = 4.9152 Mhz
	XTAL CLK1 = CLK2 / 2;            // CLK1 name from Sams schematic = 2.4576 Mhz
protected:
	int dpi = 144;
	double xscale = 9.0 / 8.0; // 1.125  (stepper moves at 162 dpi, not 144 dpi)
	double PAPER_WIDTH_INCHES = 8.5;
	double PAPER_HEIGHT_INCHES = 11.0;
	double MARGIN_INCHES = .25;
	int PAPER_WIDTH  = PAPER_WIDTH_INCHES * dpi * xscale;  // 8.5 inches wide
	int PAPER_HEIGHT = PAPER_HEIGHT_INCHES * dpi;          // 11  inches high
	int PAPER_SCREEN_HEIGHT = 384; // match the height of the apple II driver
	int distfrombottom = 50;

	int xposratio0 = 144;
	int xposratio1 = 144;
	int yposratio0 = 18;
	int yposratio1 = 18;

	int m_xpos = PAPER_WIDTH / 2;  // set initial position at middle of paper (paper width in pixels)
	int m_ypos = 30;
	s32 x_pixel_coord(s32 xpos) { return xpos * xposratio0 / xposratio1; }  // x position
	s32 y_pixel_coord(s32 ypos) { return ypos * yposratio0 / yposratio1; }  // y position given in half steps

	int update_stepper_delta(stepper_device * stepper, uint8_t stepper_pattern, const char * name, int direction);
	void update_printhead();
	void update_pf_stepper(uint8_t data);
	void update_cr_stepper(uint8_t data);

	u8 m_uart_clock = 0;
	int m_baud_clock_divisor = 1;
	int m_baud_clock_divisor_delay = 0;


	int m_left_edge_adjust = -6;  // to get perfect centering with macpaint

	int right_offset = 0;
	int left_offset  = 0;
	int m_left_edge  = (MARGIN_INCHES) * dpi * xscale + m_left_edge_adjust;
	int m_right_edge = (PAPER_WIDTH_INCHES + MARGIN_INCHES) * dpi * xscale - 1;

	// m_left_edge controls the location of the print head position sensor on the left side of the
	// printer.
	//
	// m_right edge controls the location of the print head position switch on the right side of the
	// printer.  When the carriage hits the right edge, it will return to the left edge and
	// the printer will go deselected.
	// If this is set improperly, the self test will not print more than a single line since it will
	// deselect the printer at the right edge.

	void update_head_pos();

	uint32_t screen_update_bitmap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	u8 m_porta = 0;
	u8 m_portb = 0;
	u8 m_int2 = 0;
};


DECLARE_DEVICE_TYPE(APPLE_IMAGEWRITER2_PRINTER, apple_imagewriter2_printer_device)


#endif // MAME_BUS_IMAGEWRITER2_PRINTER_H
