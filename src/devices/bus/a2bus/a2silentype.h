// license:BSD-3-Clause
// copyright-holders: Golden Child
/*********************************************************************

    silentype.h

    Implementation of the Apple II Silentype Printer Interface Card

*********************************************************************/

#ifndef MAME_BUS_A2BUS_A2SILENTYPE_H
#define MAME_BUS_A2BUS_A2SILENTYPE_H

#pragma once

#include "a2bus.h"
#include "a2silentype_printer.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_silentype_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	a2bus_silentype_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_silentype_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_reset_after_children() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;
	virtual void write_cnxx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_c800(uint16_t offset) override;
	virtual void write_c800(uint16_t offset, uint8_t data) override;
	virtual bool take_c800() const override { return true; }
	
	uint8_t m_ram[256];
	uint16_t m_shift_reg = 0;
	uint16_t m_parallel_reg = 0;

	int m_romenable = 0;
	u8 last_write_c0nx = 0;

	required_device<silentype_printer_device> m_silentype_printer;
	required_region_ptr<u8> m_rom;

 private:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_SILENTYPE, a2bus_silentype_device)

#endif // MAME_BUS_A2BUS_A2SILENTYPE_H
