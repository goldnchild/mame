// license:BSD-3-Clause
// copyright-holders:Devin Acker

#ifndef MAME_BUS_CENTRONICS_PC6022_H
#define MAME_BUS_CENTRONICS_PC6022_H

#pragma once

#include "ctronics.h"
#include "cpu/upd7810/upd7810.h"
#include "machine/input_merger.h"
#include "machine/bitmap_printer.h"

class pc6022_device : public device_t,
					public device_centronics_peripheral_interface
{

public:
	static constexpr feature_type unemulated_features() { return feature::PRINTER; }

	// construction/destruction
	pc6022_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void ack_w(int state);
	void pen_ctrl_w(int state);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_reset_after_children() override ATTR_COLD
	{
		m_bitmap_printer->cr_stepper()->set_absolute_position(400);
		m_bitmap_printer->pf_stepper()->set_absolute_position(100);
	}

	virtual void input_strobe(int state) override;
	virtual void input_data0(int state) override { if (state) m_data |= 0x01; else m_data &= ~0x01; }
	virtual void input_data1(int state) override { if (state) m_data |= 0x02; else m_data &= ~0x02; }
	virtual void input_data2(int state) override { if (state) m_data |= 0x04; else m_data &= ~0x04; }
	virtual void input_data3(int state) override { if (state) m_data |= 0x08; else m_data &= ~0x08; }
	virtual void input_data4(int state) override { if (state) m_data |= 0x10; else m_data &= ~0x10; }
	virtual void input_data5(int state) override { if (state) m_data |= 0x20; else m_data &= ~0x20; }
	virtual void input_data6(int state) override { if (state) m_data |= 0x40; else m_data &= ~0x40; }
	virtual void input_data7(int state) override { if (state) m_data |= 0x80; else m_data &= ~0x80; }

private:
	required_device<upd7801_device> m_cpu;
	required_device<input_merger_any_high_device> m_busy;
	required_device<bitmap_printer_device> m_bitmap_printer;

	void io_map(address_map &map) ATTR_COLD;

	u8 data_r();

	u8 m_data;
	u8 m_pen_ctrl;
	u8 m_pen_down;
	u8 m_ack;
	u8 m_strobe;
	void pa_w(u8 data);
	u8 pa_r();
	u8 m_pa = 0;

	static constexpr int PAPER_WIDTH = 600 ;     // 72 dpi * 4 and 1/8 inches
	static constexpr int PAPER_HEIGHT = (11*72); //  72 dpi * 11 inches

	//u8 pen_sensor() { return (m_penposition == 0) && (m_bitmap_printer->m_xpos < 140); }

	u8 m_penchangethreshold = 100;

	u8 m_pencolor = 0;
	u8 m_penposition = 0;  // pencolor = penposition / 3
	//                      black          b           g           r
	u32 m_colors[4] = {   0x00000000 , 0x000000ff, 0x0000ff00, 0x00ff0000 };
	//int m_leftmostposition=2048;



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

	public:
	void drawline(int x0, int y0, int x1, int y1, u32 pixelval) { drawline( m_bitmap_printer->page_bitmap(), x0, y0, x1, y1, pixelval); }


};

// device type definition
DECLARE_DEVICE_TYPE(PC6022, pc6022_device)

#endif // MAME_BUS_CENTRONICS_PC6022_H
