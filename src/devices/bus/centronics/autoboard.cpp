// license:BSD-3-Clause
// copyright-holders:
/**********************************************************************

    Saitek Kasparov PC Auto Chessboard

**********************************************************************/

#include "emu.h"
#include "autoboard.h"
#include "autoboard.lh"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CENTRONICS_AUTOCHESSBOARD, pc_autoboard_device, "pc_autoboard", "Saitek PC Auto Chessboard Device")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void pc_autoboard_device::device_add_mconfig(machine_config &config)
{
	HCT259(config, m_259);
//  HCT157(config, m_157_reed);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::MAGNETS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(200));
	m_board->set_nvram_enable(true);

	PWM_DISPLAY(config, m_display).set_size(8, 8);
	config.set_default_layout(layout_autoboard);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pc_autoboard_device - constructor
//-------------------------------------------------

pc_autoboard_device::pc_autoboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CENTRONICS_AUTOCHESSBOARD, tag, owner, clock),
		device_centronics_peripheral_interface(mconfig, *this),
		m_259(*this, "259"),
//      m_157_reed(*this, "157_reed"),
		m_board(*this, "board"),
		m_display(*this, "display")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pc_autoboard_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pc_autoboard_device::device_reset()
{
}

