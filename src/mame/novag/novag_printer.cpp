// license:BSD-3-Clause
// copyright-holders:Golden Child
#include "emu.h"
#include "novag_printer.h"

DEFINE_DEVICE_TYPE(NOVAG_PRINTER, novag_printer_device, "novag_printer", "Novag Chess Printer")

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void novag_printer_device::device_start()
{
	m_cr_timer = timer_alloc(FUNC(novag_printer_device::cr_tick), this);
	m_cr_timer->adjust(attotime::from_usec(TICKTIMEUS));

//  save_item(NAME());
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void novag_printer_device::device_reset()
{

}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void novag_printer_device::device_stop()
{

}


