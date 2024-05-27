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

	XTAL baseCLK = 12_MHz_XTAL;

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



	void drawline(bitmap_rgb32 &bitmap, int x0, int y0, int x1, int y1, u32 pixelval)
//routine copied from void hp9845ct_base_state::draw_line(unsigned x0 , unsigned y0 , unsigned x1 , unsigned y1)
{
	int dx, dy, sx, sy, x, y, err, e2;

	// draw line, vector generator uses Bresenham's algorithm
	x = x0;
	y = y0;
	dx = abs((int) (x1 - x));
	sx = x < x1 ? 1 : -1;
	dy = abs((int) (y1 - y));
	sy = y < y1 ? 1 : -1;
	err = (dx > dy ? dx : -dy) / 2;

	for(;;)
	{
		if (!((x<0) || (x >= bitmap.width()) || (y<0) || (y >= bitmap.height())))
				bitmap.pix(y,x) = pixelval;

		if (x == x1 && y == y1) break;

		e2 = err;
		if (e2 > -dx)
		{
				err -= dy;
				x += sx;
		}
		if (e2 < dy)
		{
				err += dx;
				y += sy;
		}
	}
}

void draw_vector_char(bitmap_rgb32 &bitmap, u8 c, int x0, int y0, int xsize, int ysize, u32 pixelval)
{

static constexpr u8 vector_font[96][20] =
{
	{128}, // char 32 " "
	{34,37,53,51,35,38,47,63,55,167}, // char 33 "!"
	{30,29,62,189}, // char 34 """
	{20,29,60,53,70,7,10,203}, // char 35 "#"
	{20,53,71,57,25,11,29,61,46,163}, // char 36 "$"
	{4,77,28,13,11,27,29,54,71,69,53,183}, // char 37 "%"
	{66,11,13,31,45,43,7,5,19,35,199}, // char 38 "&"
	{42,175}, // char 39 "'"
	{50,35,21,29,47,191}, // char 40 "("
	{34,51,69,77,63,175}, // char 41 ")"
	{4,77,12,69,44,165}, // char 42 "*"
	{36,45,8,201}, // char 43 "+"
	{32,51,53,37,35,179}, // char 44 ","
	{8,201}, // char 45 "-"
	{34,37,53,51,163}, // char 46 "."
	{2,221}, // char 47 "/"  221 = 0xdd = (5,6)
	{4,77,63,31,13,5,19,51,69,205}, // char 48 "0"
	{28,47,35,18,179}, // char 49 "1"
	{12,31,63,77,75,3,195}, // char 50 "2"
	{12,31,63,77,75,57,41,56,71,69,51,19,133}, // char 51 "3"
	{50,63,9,7,199}, // char 52 "4"
	{4,19,51,69,73,59,11,15,207}, // char 53 "5"
	{8,27,59,73,69,51,19,5,13,31,63,205}, // char 54 "6"
	{2,75,79,143}, // char 55 "7"
	{24,11,13,31,63,77,75,57,71,69,51,19,5,7,25,185}, // char 56 "8"
	{4,19,51,69,77,63,31,13,11,25,57,203}, // char 57 "9"
	{20,23,39,37,21,26,29,45,43,155}, // char 58 ":"
	{18,37,39,23,21,37,42,45,29,27,171}, // char 59 ";"
	{66,25,207}, // char 60 "<"
	{22,71,26,203}, // char 61 "="
	{18,73,159}, // char 62 ">"
	{12,31,63,77,75,57,41,39,36,163}, // char 63 "?"
	{54,59,27,21,53,71,75,61,29,11,5,19,195}, // char 64 "@"
	{2,11,47,75,67,8,201}, // char 65 "A"
	{2,15,63,77,75,57,8,57,71,69,51,131}, // char 66 "B"
	{68,51,19,5,13,31,63,205}, // char 67 "C"
	{2,15,63,77,69,51,131}, // char 68 "D"
	{66,3,15,79,56,137}, // char 69 "E"
	{2,15,79,8,185}, // char 70 "F"
	{76,63,31,13,5,19,67,73,169}, // char 71 "G"
	{2,15,78,67,8,201}, // char 72 "H"
	{18,51,34,47,30,191}, // char 73 "I"
	{4,19,35,53,191}, // char 74 "J"
	{2,15,78,7,24,195}, // char 75 "K"
	{14,3,195}, // char 76 "L"
	{2,15,43,41,43,79,195}, // char 77 "M"
	{2,15,12,69,78,195}, // char 78 "N"
	{4,13,31,63,77,69,51,19,133}, // char 79 "O"
	{2,15,63,77,75,57,137}, // char 80 "P"
	{38,67,50,19,5,13,31,63,77,69,179}, // char 81 "Q"
	{2,15,63,77,75,57,9,24,195}, // char 82 "R"
	{4,19,51,69,71,57,25,11,13,31,63,205}, // char 83 "S"
	{34,47,14,207}, // char 84 "T"
	{14,5,19,51,69,207}, // char 85 "U"
	{14,7,35,71,207}, // char 86 "V"
	{14,3,39,40,39,67,207}, // char 87 "W"
	{2,5,77,79,14,13,69,195}, // char 88 "X"
	{34,41,77,79,14,13,169}, // char 89 "Y"
	{14,79,77,5,3,195}, // char 90 "Z"
	{34,3,15,175}, // char 91 "["
	{0x0c,0xd3}, // char 92 "\"
//    {92,77,59,53,35,19,5,23,51,83,40,201}, // char 92 "\"
	{18,51,63,159}, // char 93 "]"
	{0xa,0x2f,0xcb}, // char 94 "^"
//    {34,45,8,45,201}, // char 94 "^"
	{0x02,0xc3}, // char 95 "_"
//    {36,9,45,8,217}, // char 95 "_"
	{0x1e,0xbb}, // char 96 "`"
//    {8,249}, // char 96 "`"
	{52,35,19,5,7,25,41,55,10,43,57,53,195}, // char 97 "a"
	{14,3,35,53,57,43,139}, // char 98 "b"
	{52,35,19,5,9,27,43,185}, // char 99 "c"
	{58,27,9,5,19,51,191}, // char 100 "d"
	{6,55,57,43,27,9,5,19,179}, // char 101 "e"
	{0x22,0x2d,0x3f,0x4f,0x08,0xc9}, // char 102 "f"
//    {34,47,63,24,185}, // char 102 "f"
	{2,17,33,51,57,43,27,9,7,21,37,183}, // char 103 "g"
	{2,15,10,43,57,179}, // char 104 "h"
	{34,41,42,173}, // char 105 "i"
	{2,17,33,51,57,58,189}, // char 106 "j"
	{50,23,14,3,4,187}, // char 107 "k"
	{30,19,163}, // char 108 "l"
	{2,11,8,27,41,35,40,59,73,195}, // char 109 "m"
	{2,11,9,27,43,57,179}, // char 110 "n"
	{4,9,27,43,57,53,35,19,133}, // char 111 "o"
	{4,37,55,57,43,11,129}, // char 112 "p"
	{64,49,59,27,9,7,21,181}, // char 113 "q"
	{10,25,19,24,43,187}, // char 114 "r"
	{4,19,35,53,39,23,9,27,43,185}, // char 115 "s"
	{10,43,30,19,163}, // char 116 "t"
	{10,0x05,0x13,51,187}, // char 117 "u"
//    {10,3,51,187}, // char 117 "u"
	{10,7,35,71,203}, // char 118 "v"
	{10,5,19,37,39,37,51,69,203}, // char 119 "w"
	{2,75,10,195}, // char 120 "x"
	{10,9,37,74,73,129}, // char 121 "y"
	{10,75,3,195}, // char 122 "z"
	{0x3e,0x2d,0x2b,0x19,0x27,0x25,0xb3}, // char 123 "{"
//    {48,191}, // char 123 "{"
	{0x22,0xaf}, // char 124 "|"
//    {112,129}, // char 124 "|"  0x70, 0x81
	{0x1e,0x2d,0x2b,0x39,0x27,0x25,0x93}, // char 125 "}"
//    {2,57,99,131}, // char 125 "}"
	{0x0a,0x1d,0x2d,0x2b,0x3b,0xcd}, // char 126 "~"
//    {6,25,57,51,18,25,57,203}, // char 126 "~"
	{2,15,95,83,131} // char 127 ""
};

	if (c >= 32 && c <= 127)
	{
		u8 i = 0;
		u8 val = 0;
		u8 xold = 0;
		u8 yold = 0;
		c = c - 32;
		while (val < 128 && i <= 20)
		{
			val = vector_font[c][i];
			u8 xc = (val & 0x70) >> 4;
			u8 yc = (val & 0x0e) >> 1;
			u8 lineto = (val & 0x1);
			if (lineto) drawline(bitmap, xc * xsize + x0, (7-yc) * ysize + y0, xold * xsize + x0, (7-yold)*ysize + y0, pixelval);
			xold = xc;
			yold = yc;
			i ++;
		}
	}
} // draw_vector_char

void draw_string(bitmap_rgb32 &bitmap, const char* b, int x0, int y0, int xsize, int ysize, u32 pixelval)
{
	for (int i=0; i<strlen(b); i++)
	{
		draw_vector_char(bitmap, b[i], x0 + i*xsize*8,     y0, xsize, ysize, 0);
		draw_vector_char(bitmap, b[i], x0 + i*xsize*8 + 1, y0, xsize, ysize, 0);
	}
}
};


DECLARE_DEVICE_TYPE(APPLE_IMAGEWRITER2_PRINTER, apple_imagewriter2_printer_device)


#endif // MAME_BUS_IMAGEWRITER2_PRINTER_H
