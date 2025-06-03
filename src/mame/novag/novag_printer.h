// license:BSD-3-Clause
// copyright-holders: Golden Child
#ifndef MAME_NOVAG_NOVAG_PRINTER_H
#define MAME_NOVAG_NOVAG_PRINTER_H

#pragma once

#include "machine/bitmap_printer.h"


class novag_printer_device;

DECLARE_DEVICE_TYPE(NOVAG_PRINTER, novag_printer_device)


class novag_printer_device : public device_t
{
public:
//  novag_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	novag_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0)
	: device_t(mconfig, NOVAG_PRINTER, tag, owner, clock)
	, m_bitmap_printer(*this, "bitmap_printer")
	{
	}

	void write(u8 data, u8 headnum) // headnum numbered from right side
	{
		m_motor = BIT(data, 5);
		drawpix(data, headnum);
	}

	u8 read() // returns m_timing wheel rotary encoder data
	{
		return m_timing[m_timingpos];
	}

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void device_add_mconfig(machine_config &config) override ATTR_COLD
	{
		constexpr int PAPER_WIDTH = 175;
		constexpr int PAPER_HEIGHT = (11*72);
		BITMAP_PRINTER(config, m_bitmap_printer, PAPER_WIDTH, PAPER_HEIGHT, 72, 72);
		m_bitmap_printer->set_pf_stepper_ratio(1,1);
		m_bitmap_printer->set_cr_stepper_ratio(1,6);
//      m_bitmap_printer->set_printhead_size(1,1,1);
//      m_bitmap_printer->set_printhead_color(0xffbbbb, 0xdfdd88);
	}

private:
	required_device<bitmap_printer_device> m_bitmap_printer;

	u8 m_headpos = 0;
	emu_timer *m_cr_timer;
	u8 m_motor = 0;
	u8 col;
	u8 m_timingpos = 0;
	u8 m_pf_stepper_index = 0;

	constexpr static u8 m_timingsize = 3 + 14 * 4 + 4 * 4; // 1(S) * 3 + 14(P) * 4 + 4(N) * 4
	constexpr static u8 m_timing[m_timingsize] =
	{
		0,4,0,
		0,0,0,0,
		2,0,1,0, 2,0,1,0, 2,0,1,0, 2,0,1,0, 2,0,1,0, 2,0,1,0, 2,0,1,0,
		0,0,0,0, 0,0,0,0,
		2,0,1,0, 2,0,1,0, 2,0,1,0, 2,0,1,0, 2,0,1,0, 2,0,1,0, 2,0,1,0,
		0,0,0,0
	};

	constexpr static u16 TICKTIMEUS = 3000;  // 6000, 12000 also works, makes printing slower

	constexpr static u8 m_headpos_to_offset_size = 29;
	constexpr static int m_headpos_to_offset[m_headpos_to_offset_size]=
	{13,13,12,11,10,9,8,7,6,5, 4, 3, 2, 1, 0,    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13};
	//0  1  2  3  4 5 6 7 8 9 10 11 12 13 14    15 16 17 18 19 20 21 22 23 24 25 26 27 28
	// 14 pulses moving right to left (offset +13 and decreasing to 0)
	// 14 pulses moving left to right (offset 0 and increasing to +13)
	// 28 total pulses
	// 0 is a dummy position, when start pulse fires, m_headpos gets set to 0
	// m_headpos gets advanced to 1 upon the first pulse from the m_timing wheel


	//Standard drive table is 2,6,4,5,1,9,8,a

	constexpr static u8 m_stepper_table_size = 8;
	constexpr static u8 m_stepper_table[m_stepper_table_size] = {2, 6, 4, 5, 1, 9, 8, 0xa};

	void drawpix(u8 data, u8 headnum)
	{
		for (u8 i = 0; i < 5; i++) // 5 bits of printhead data 0..4
		{
			if (BIT(data,i))
				m_bitmap_printer->pix(
								m_bitmap_printer->m_ypos + ((m_headpos <= m_headpos_to_offset_size / 2) ? 0 : 1),
								((5-i) * 14)+ m_headpos_to_offset[m_headpos] + (5 * headnum * 14)) = 0x000000;
		}
	}

	TIMER_CALLBACK_MEMBER(cr_tick)
	{
		if (m_motor) // motor on
		{
			m_timingpos++;
			m_timingpos %= m_timingsize;
			u8 cur = m_timing[m_timingpos];  // current m_timing value
			if (cur == 4)
			{
				// every cycle of the timing wheel shifts the paper up by two dot lines
				// so when the cycle resets, we will shift down by two dots
				// using the stepper motors is equivalent to:
				// m_bitmap_printer->m_ypos += 2;
				for (int i = 0; i < 2; i++)
				{
					m_bitmap_printer->update_pf_stepper(m_stepper_table[m_pf_stepper_index]);
					m_pf_stepper_index++;
					m_pf_stepper_index %= m_stepper_table_size;
					m_bitmap_printer->update_cr_stepper(1);  // wiggle the cr stepper to make the bitmap printer save the page
					m_bitmap_printer->update_cr_stepper(5);  // at the bottom
					m_bitmap_printer->update_cr_stepper(1);
					// set the headpos to avoid visible jumping
					m_bitmap_printer->setheadpos(m_headpos_to_offset[m_headpos], m_bitmap_printer->m_ypos);
				}
				m_headpos = 0;
			}
			else if (cur == 2 || cur == 1)
			{
				m_headpos++;
				m_headpos %= m_headpos_to_offset_size;
				m_bitmap_printer->setheadpos(m_headpos_to_offset[m_headpos], m_bitmap_printer->m_ypos);
			}
		}
		m_cr_timer->adjust(attotime::from_usec(TICKTIMEUS));
	}
};

#endif // MAME_NOVAG_NOVAG_PRINTER_H

