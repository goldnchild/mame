// license:BSD-3-Clause
// copyright-holders: Golden Child
/*********************************************************************

    silentype_printer.cpp

    Implementation of the Apple Silentype Printer

    (Implements physical printhead and stepper motors)

**********************************************************************/

#include "emu.h"
#include "a2silentype_printer.h"
//#include "emuopts.h"
//#include "fileio.h"
//#include "png.h"
//#include <bitset>

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"



/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(SILENTYPE_PRINTER, silentype_printer_device, "silentype", "Apple Silentype Printer")


//**************************************************************************
//  INPUT PORTS
//**************************************************************************

#define PORT_ADJUSTER_16MASK(_default, _name)                   \
	configurer.field_alloc(IPT_ADJUSTER, (_default), 0xffff, (_name)); \
	configurer.field_set_min_max(0, 100);



INPUT_PORTS_START(silentype_printer)
	PORT_START("CNF")
	PORT_CONFNAME(0x1, 0x01, "Print Darkness")
	PORT_CONFSETTING(0x0, "Normal (grey)")
	PORT_CONFSETTING(0x1, "Dark   (b/w)")

	PORT_START("HEATTIME")
	PORT_ADJUSTER_16MASK(1, "Heat Time")
	PORT_MINMAX(1,200)

	PORT_START("HEATTIME2")
	PORT_ADJUSTER_16MASK(40, "Heat Time2")
	PORT_MINMAX(1,200)

	PORT_START("COOLTIME")
	PORT_ADJUSTER_16MASK(1, "Cool Time")
	PORT_MINMAX(1,200)


INPUT_PORTS_END


ioport_constructor silentype_printer_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(silentype_printer);
}

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void silentype_printer_device::device_add_mconfig(machine_config &config)
{
	[[maybe_unused]] bitmap_printer_device &printer(BITMAP_PRINTER(config, m_bitmap_printer, PAPER_WIDTH, PAPER_HEIGHT, dpi, dpi));
	m_bitmap_printer->set_cr_stepper_ratio(1,2);
	m_bitmap_printer->set_pf_stepper_ratio(7,8);

	//STEPPER(config, m_pf_stepper, (uint8_t) 0xa);
	//STEPPER(config, m_cr_stepper, (uint8_t) 0xa);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

silentype_printer_device::silentype_printer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
		device_t(mconfig, type, tag, owner, clock),
		m_bitmap_printer(*this, "bitmap_printer")
//		m_pf_stepper(*this, "pf_stepper"),
//		m_cr_stepper(*this, "cr_stepper")
{
}

silentype_printer_device::silentype_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		silentype_printer_device(mconfig, SILENTYPE_PRINTER, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void silentype_printer_device::device_start()
{
//	save_item(NAME(m_xpos));
//	save_item(NAME(m_ypos));
	save_item(NAME(right_offset));
	save_item(NAME(left_offset));
	save_item(NAME(heattime));
	save_item(NAME(decaytime));
	save_item(NAME(lastheadbits));
	save_item(NAME(headtemp));
//	save_item(NAME(hstepperlast));
//	save_item(NAME(vstepperlast));
//	save_item(NAME(xdirection));
//	save_item(NAME(newpageflag));
//	save_item(NAME(page_count));
	save_item(NAME(last_update_time));
}

void silentype_printer_device::device_reset_after_children()
{
//	m_ypos=10;
}

void silentype_printer_device::device_reset()
{
//	update_pf_stepper(0);
//	update_cr_stepper(0);
//	update_printhead(0);
}

// this doesn't really take the time into account
// temp and length of time its exposed to heat

// flat out wrong results because it is only looking at the current value and extrapolating
// instead of looking at the last value and noting the change
// treat each bit as independent, track when bits change


void silentype_printer_device::darken_pixel(double headtemp, unsigned int& pixel)
{
	if (headtemp > 0.0)
	{
		u8 intensity = (
						ioport("CNF")->read() & 0x1 ?
							std::min(headtemp * 4, 100.0) :
							headtemp
						) * 0.150;
		u32 pixelval = pixel;
		u32 darkenval = intensity * 0x111111;

		pixelval &= 0xffffff;

		u32 rp = BIT(pixelval, 16, 8);
		u32 gp = BIT(pixelval, 8, 8);
		u32 bp = BIT(pixelval, 0, 8);

		u32 rd = BIT(darkenval, 16, 8);
		u32 gd = BIT(darkenval, 8, 8);
		u32 bd = BIT(darkenval, 0, 8);

		u32 r = (rp >= rd) ? rp - rd : 0;    // subtract the amount to darken
		u32 g = (gp >= gd) ? gp - gd : 0;
		u32 b = (bp >= bd) ? bp - bd : 0;

		pixelval = (r << 16) | (g << 8) | (b << 0);

		pixel = pixelval;
	}
}

//-------------------------------------------------
//    Adjust Printhead Temperature
//-------------------------------------------------

void silentype_printer_device::adjust_headtemp(u8 pin_status, double time_elapsed, double& temp)
{
/*
	temp += ( (pin_status) ?
				(time_elapsed / ((double) heattime  / 1.0E6)) :
			  - (time_elapsed / ((double) decaytime / 1.0E6)) );
*/
	double scaledheat = ((double) ioport("HEATTIME")->read() * 100.0 + ioport("HEATTIME2")->read() * 1.0) / 1.0e6;
	double tempfrac = time_elapsed / scaledheat;
	temp += ( (pin_status) ?
				(time_elapsed / (((double) ioport("HEATTIME")->read() * 100.0 + ioport("HEATTIME2")->read() * 1.0) / 1.0E6)) :
			  - (time_elapsed / (((double) ioport("COOLTIME")->read() * 100.0) / 1.0E6)) );		  
	if (temp < 0.0) temp = 0;
	if (temp > 100.0) temp = 100.0;
	if (temp > 0.0) printf("TEMP = %f  time_elapsed=%f   time=%f    scaled=%f  tempfrac=%f\n",temp, time_elapsed, machine().time().as_double(), scaledheat, tempfrac);
}

//-------------------------------------------------
//    Update Printhead
//-------------------------------------------------

void silentype_printer_device::update_printhead(u8 headbits)
{

	double current_time = machine().time().as_double();
	double time_elapsed = current_time - last_update_time;
	last_update_time = current_time;
/*
	LOG("PRINTHEAD %x\n",headbits);
	LOG("PRINTHEAD TIME ELAPSED = %f   %f usec     bitpattern=%s\n",
	  time_elapsed, time_elapsed*1e6, std::bitset<8>(headbits).to_string().c_str());
*/
	for (int i=0;i<7;i++)
	{
		adjust_headtemp( BIT(lastheadbits,i), time_elapsed,  headtemp[i] );

//		int xpixel = (m_xpos/2) + ((xdirection == 1) ? right_offset : left_offset);
//		int ypixel = ypos_coord(m_ypos) + (6 - i);

		int x = m_bitmap_printer->m_xpos + ((m_bitmap_printer->m_cr_direction == 1) ? right_offset : left_offset);
		int y = m_bitmap_printer->m_ypos + (6-i);

		if ((x >= 0) && (x <= (PAPER_WIDTH - 1)) && (y >= 0) && (y <= (PAPER_HEIGHT - 1)))
		{
			darken_pixel( headtemp[i], m_bitmap_printer->pix(y, x) );
		}
	}
	lastheadbits = headbits;
}

void silentype_printer_device::write_data(u8 data)
{
	//printf("write data = %x   %s\n",data,machine().describe_context().c_str());
	m_datalast = data;
}

void silentype_printer_device::write_shiftclock(u8 newstate)
{
	//printf("write  shiftclock = %x   %s\n",newstate,machine().describe_context().c_str());
	
	if (newstate && !m_shiftclocklast) // shift clock rising edge
	{
		if (m_storeclocklast) 
		{
			printf("CLEAR PARALLEL SC\n");
			m_parallel_reg = 0;
		}
		m_shift_reg = (m_shift_reg << 1) | m_datalast;
	}
	m_shiftclocklast = newstate;
}



// rw depends on the bit 8 coming out of the parallel register
// so the last shift in affects the function table

// ok, xco by itself stores
// xco + shift = clear


void silentype_printer_device::write_storeclock(u8 newstate)
{
	//printf("write   storeclock = %x   %s\n",newstate,machine().describe_context().c_str());
	
	if (newstate && !m_storeclocklast) // store clock rising edge
	{
		{
			m_parallel_reg = m_shift_reg;

			for (int i=15;i>=0;i--) 
			{ printf( "%s%s", BIT(m_parallel_reg,i) & 1 ? "1" : "0", i==8 ? " " : "" );}
			printf("   %f %s  %f\n", machine().time().as_double(), machine().describe_context().c_str(),  machine().time().as_double() - m_last_stepper_x_time);
			
			if (m_last_stepper_x != BIT(m_parallel_reg, 0, 4)) m_last_stepper_x_time = machine().time().as_double();
			m_last_stepper_x = BIT(m_parallel_reg, 0, 4);

update_printhead(BIT(m_parallel_reg, 9, 7));
	
		}
	}

	if (m_shiftclocklast & newstate) // we clear the parallel register
	{
		printf("CLEAR PARALLEL\n");
		m_parallel_reg = 0;
		update_printhead(BIT(m_parallel_reg, 9, 7));
	
	}

	m_bitmap_printer->update_cr_stepper(bitswap<4>(BIT(m_parallel_reg, 0, 4),3,1,2,0));
	m_bitmap_printer->update_pf_stepper(bitswap<4>(BIT(m_parallel_reg, 4, 4),3,1,2,0));
	
	m_storeclocklast = newstate;
}
