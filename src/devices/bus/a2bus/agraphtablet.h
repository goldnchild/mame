// license:BSD-3-Clause
// copyright-holders:Golden Child
/*********************************************************************

    agraphtablet.h

    Implementation of the Apple II Graphics Tablet

	The Apple Graphics Tablet is based on the same technology as the Summagraphics Bitpad One.

	The Apple Graphics Tablet uses a proprietary interface along with a proprietary interface card 
	to connect to the Apple II instead of the Bitpad One's RS232/IEEE488/Parallel port, so they
	are not directly compatible.  The Bitpad One has an embedded microprocessor to handle the
	interface and tablet scanning, where the Apple II version uses the main 6502 cpu along with 2k
	of rom code.

	from https://interfacemuseum.com/exhibits/summagraphics-bit-pad/

	"The Bit Pad One housed a magnetostrictive wire grid beneath a rigid 12 × 12‑inch (305 × 305 mm)
	active area, surrounded by a sturdy metal or plastic enclosure measuring approximately 
	14.5 × 14.5 × 1.5 inches (368 × 368 × 38 mm) and weighing around 11 pounds (5 kg). 
	The grid produced a magnetic field; when the coil in the stylus or puck passed over it,
	the tablet electronics measured the time delay of the induced pulse to calculate the 
	absolute X and Y position. Resolution was 0.005 inches (0.127 mm), with a specified accuracy 
	of ±0.015 inches (0.38 mm)."

    With many thanks to Marc S. Ressl and his OpenEmulator for doing all of the
    heavy lifting in figuring out how it works.  The A2 Documentation Project
    was also extremely helpful in assembling all of the technical information and manuals.

    The Graphics Tablet has an array of wires for detecting the position of a
    pen tip.  It works by timing how long a pulse takes to
    travel back to the detector.  It fires a pulse and counts at
    49 mhz until it gets the return signal from the magnetostrictive wires.
    The counting is done by 3 4-bit binary counters, a 74S161, and two DM85L54 counters.

    Each axis has a set of wires that run the length of the tablet.
    The wires run horizontally for the x axis or vertically for the y axis.

    X = slot number * 10

    After the pulse is fired by accessing
    LDA C080,X for the x axis or
    LDA C081,X for the y axis,
    the firmware waits for a time long enough for the counting to be completed
    and then reads C082,X and C083,X for the 12 bits of the counters.

	The firmware is limits the maximum raw value to 0xa00 (2560 decimal) 
	which is less than the full 12 bit 4096 range.
	The minimum raw value is 0x60+1 (97 decimal).

	The tablet active area is approximately 12x12 inches at 200 dpi resolution.  
	The range from 97 to 2560 = 2463 which is approximately 2400 (12 inches * 200 dpi).

    LDA C082,X returns the lower 4 bits of the timer.  Bits 0-2 are inverted from a 74LS368
    hex bus driver.

    LDA C083,X returns the upper 8 bits of the timer and also loads the pen status into
    bit 0 of the timer value and clears bits 1-3 so that a subsequent LDA C082,X
    will return the pen status in bit 0 and bits 1,2,3 cleared.

    Use of the tablet (assuming slot #2)

    From applesoft:

    PR#2:PRINT"D,M1"

This will activate a tablet in slot 2, initialize it to defaults and set to
mixed mode hgr page 1.  It will also automatically switch back to PR#0
immediately after the command.

Just PR#2 by itself will give ***SYNTAX ERROR as an empty command string
is an error.

IN#2:INPUT"";X$Y$Z$:IN#0 :  REM This will read the tablet position.

This short program will read the tablet for every click of the pen.

emu.keypost(''..
'      90 PR#2:PRINT"D,M1" : REM DEFAULT, MIXED MODE PAGE 1\n'..
'     100 FOR I=1 TO 2\n'..
'     110 IN#2:INPUT "";X$,Y$,Z$:IN#0\n'..
'     115 VTAB 23\n'..
'     120 PRINT X$Y$Z$\n'..
'     125 POKE -16368,0\n'..
'     130 NEXT : REM NEED TO DO READ TWICE, ONCE FOR PEN CLICK DOWN AND ONCE FOR CLICK UP\n'..
'     140 GOTO 100\n'..
'     RUN\n')

CTRL+C will break this program back to the command prompt.

This short program will stream values to the screen.  Command P is "streaming mode".

emu.keypost(''..
'      90 PR#2:PRINT"D,M1" : REM DEFAULT, MIXED MODE PAGE 1\n'..
'     100 PR#2:PRINT"P,X291,Y291,S9"\n'..
'     110 IN#2:INPUT "";X$,Y$,Z$:IN#0\n'..
'     120 VTAB 22:PRINT X$Y$Z$\n'..
'     125 POKE -16368,0\n'..
'     140 GOTO 110\n'..
'     RUN\n')

Off scale values cause the tablet read routine to pause while it keeps looking
for a proper onscale value.  Hitting a keyboard key during this pause will
force the read routine to return the offscale values as zeroes: +0000+0000-10

The tens digit will show a 1 to indicate the offscale value, and there
will be a minus sign to indicate the keyboard was pressed.

A 3 in the ones digit isn't mentioned in the manual but it appears to be
pen switch up.  (p.60 of the Apple Graphics Tablet Reference Manual)

0 = pen down, still down
1 = just lifted up
2 = just pressed down
3 = pen up, still up


Loading DOS 3.3 is unnecessary to access the tablet, as the tablet's
firmware is in ROM on the interface card.

Once you boot up, hit CTRL+F12 to reset, and then you can do
PR#2 (or whatever slot you've put it into) and print it an initialization string.


emu.keypost('50 HGR\n100 PR#2\n105 PRINT "M1,P"\n110 PR#0\n200 IN#2\n210 INPUT A$,B$,C$\n220 IN#0\n250 VTAB 22\n260 PRINT A$;B$;C$\n270 X=PEEK(-16368)\n280 GOTO 200\nRUN\n')

for text mode, use T1, the cursor will be an underscore character

emu.keypost('50 HGR\n100 PR#2\n105 PRINT "T1,P"\n110 PR#0\n200 IN#2\n210 INPUT A$,B$,C$\n220 IN#0\n250 VTAB 22\n260 PRINT A$;B$;C$\n270 X=PEEK(-16368)\n280 GOTO 200\nRUN\n')


default offset 1536 = 0x600
default scale = 16 (divides by 16)

PRINT "D,T1,P,R" will give a range from -77 to 269

PRINT "D,T1,P,R,X0,Y0" will give a range from +18 to +365

PRINT "D,T1,P,R,X0,Y0,S1" will give ranges from +291 to +5853   (same as without R)
PRINT "D,T1,P,R,X0,Y0,S1" will give ranges from +145 to +2926



emu.keypost('NEW\n90 ONERR GOTO 150\n100 D$=CHR$(4)\n110 F$="TAB.INFORMATION":PRINT D$"OPEN "F$:PRINT D$"READ "F$\n120 INPUT A$\n130 PRINT A$\n140 GOTO 120\n150 PRINT D$"CLOSE "F$\nRUN\n')


*********************************************************************/

#ifndef MAME_BUS_A2BUS_AGRAPHTABLET_H
#define MAME_BUS_A2BUS_AGRAPHTABLET_H

#pragma once

#include "a2bus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_agraphtablet_device: public device_t,
	public device_a2bus_card_interface
{
	public:
	a2bus_agraphtablet_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	protected:
	a2bus_agraphtablet_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;
	virtual void write_cnxx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_c800(uint16_t offset) override;
	virtual void write_c800(uint16_t offset, uint8_t data) override;
	virtual bool take_c800() const override { return true; }

	required_region_ptr<u8> m_rom;

	required_ioport m_joy3x;
	required_ioport m_joy3y;
	required_ioport m_joy3buttons;
	//required_ioport m_mouseb;
	//required_ioport_array<2> m_mousexy;

	//  int m_tabletmax = 2560;  // maximum value of the timer counters is 0xa00  = 2560
	//  int m_tabletmin = 97;    // minimum value of the timer counters is 0x60+1 = 97
	//  values outside of this range is considered "off-scale"

	/*  values that work well for Applesoft
	int m_tabletmin = 0x000+97;   // minimum value is 0x60+1 = 97
	int m_tabletmax = 0x400-97;   // range 0-97,512,927-1024     // maximum value is 0xa00  = 2560
	int m_tabletoffscale = 0xa01;  // value to return when pen is off the surface
	*/

	//  values that work better for apple graphics tablet disk
	int m_tabletmin = 0x000+97;   // minimum value is 0x60+1 = 97
	int m_tabletmax = 0x800-97;   // range 0-97,512,927-1024     // maximum value is 0xa00  = 2560
	int m_tabletoffscale = 0xa01;  // value to return when pen is off the surface

	// tuning the tablet can be done from the mame debugger memory window,
	//  just search for the Apple Graphics Tablet/:sl2
	// tuning the tablet can be done from the mame lua console:
	// too high values or too low values go "off scale"
		// emu.item(manager:machine().devices[":sl2:agraphtablet"].items["0/m_tabletmax"]):write(0,2560)
	// emu.item(manager:machine().devices[":sl2:agraphtablet"].items["0/m_tabletmin"]):write(0,100)
	// emu.item(manager:machine().devices[":sl2:agraphtablet"].items["0/m_tabletoffvalue"]):write(0,0x60)
	//
	// some of the offsets used by the firmware can be tuned on the fly as well.
	// see page 64 of the Apple Graphics Tablet Reference manual
	// OFFXL = $05B8 + MSLOT
	// OFFXH = $0638 + MSLOT
	// MSLOT = $CX where X is the slot number
	// this will set the X offset to 0x600  or 1536
	// print(manager:machine().devices[":maincpu"].spaces["program"]:write_u8(0x5b8+0xc2,0))
	// print(manager:machine().devices[":maincpu"].spaces["program"]:write_u8(0x638+0xc2,6))

	int m_timervalue;  // timer return value
	private:
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_AGRAPHTABLET, a2bus_agraphtablet_device)
#endif // MAME_BUS_A2BUS_A2VIDEOTERM_H
