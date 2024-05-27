// license:BSD-3-Clause
// copyright-holders:Golden Child

/**************************************************************************

    Apple ImageWriter 2 Printer



It uses a gate array to control the printhead and the steppers.
It's unclear how that works exactly so everything here is guesses and is probably wrong, especially anything
that has to do with the cr stepper.

After launching, it gets hung up at 2d9 so executing a pc=2da will get past that spot.

Then pressing and holding keypad 0 and the 1 key and then pressing the space bar, then releasing the keypad 0
will make it do a bunch of activity then it will flash the error led, and you can change
the print quality led.

Also if you set the FF key upon reset, then perform the above steps, it will go into a self
test mode, which can be seen by opening up the B800 memory range in the debugger.

The layout is setup to work with the apple2e driver due to the inputtags being relative, e.g. inputtag=":sl1:ssc:rs232:imagewriter2:RESET"

./mame apple2e -sl1 ssc -sl1:ssc:rs232 imagewriter2 -window -debug


Some notes:


ESC jump table:

     2C1B   2D15   -.  !    start bold
     2C1D   2D18   -.  "    stop bold
     2C1F   368B   6.  #    xxx
     2C21   313E   1>   $    switch to normal font
     2C23   368B   6.   %   xxx
     2C25   32E5   2�   &   mousetext to low ascii
     2C27   3163   1c   '   switch to custom character font
     2C29   2E8D   ..   (   set tabs
     2C2B   2E7A   .z   )   clear tabs
     2C2D   3166   1f   *   swtich to custom char font high ascii
     2C2F   317E   1~   +   max width of custom 16 dots
     2C31   368B   6.   ,   xxx
     2C33   317B   1{   -   max width of custom 8 dots
     2C35   368B   6.   .   xxx
     2C37   368B   6.   /   xxx
     2C39   2E9B   ..   0    clear all tabs
     2C3B   2D45   -E   1   insert 1 dot space
     2C3D   2D47   -G   2   insert 2 dot space
     2C3F   2D49   -I   3    3
     2C41   2D4B   -K   4    4
     2C43   2D4D   -M   5    5
     2C45   2D4F   -O   6    6 insert 6 dot space
     2C47   368B   6.   7   XXX
     2C49   368B   6.   8   XXX
     2C4B   368B   6.   9   XXX
     2C4D   368B   6.   :   XXX
     2C4F   368B   6.   ;   XXX
     2C51   3241   2A   <   BIDIRECTIONAL
     2C53   368B   6.   =   XXX
     2C55   324B   2K   >   UNIDIRECTIONAL
     2C57   3688   6.   ?   SEND ID STRING   IW10CF
     2C5B   32FF   2�   A   6 lines per inch   mvi a,$18   24 * 6 = 144
     2C5D   3306   3.   B   8 lines per inch   mvi a,$12   18 * 8 = 144
     2C5F   368B   6.   C   xxx
     2C61   302A   0*   D   set DIP switches  ON
     2C63   2CE2   ,�   E   2ce2  Elite ESC E    mvi b,$08
     2C65   3283   2.   F   F nnnn dot column from left margin
     2C67   333A   3:   G   G IS GRAPHICS nnnn
     2C69   31F6   1�   H   set page length to nnnn/144
     2C6B   318F   1.   I   start loading new characters
     2C6D   368B   6.   J   XXX
     2C6F   2D27   -'   K   COLOR PRINT
     2C71   30B6   0�   L   set left margin at col nnn
     2C73   30F5   0�   M   NLQ font (same as ESC a 2)
     2C75   2CDE   ,�   N   ESC N 10 cpi   mvi b,$00
     2C77   30E1   0�   O   paper out sensor OFF
     2C79   2CED   ,�   P   ESC P 160 dpi mvi b,$58
     2C7B   2CE6   ,�   Q   ESC Q ultracondensed mvi b,$10
     2C7D   31CD   1�   R   nnn repeat char nnn times
     2C7F   333A   3:   S   S IS GRAPHICS nnnn
     2C81   3309   3    T   distance between lines nn/144 nn = 01 to 99
     2C83   368B   6.   U   xxx
     2C85   331A   3.   V   repeat pattern nnnn c
     2C87   2FFA   /�   W   ESC W stop half height
     2C89   312C   1,   X   start underline
     2C8B   312F   1/   Y   stop underling
     2C8D   3065   0e   Z   set DIP switches OFF
     2C8F   368B   6.   [   xxx
     2C91   368B   6.   \   xxx
     2C93   368B   6.   ]   xxx
     2C95   368B   6.   ^   xxx
     2C97   368B   6.   _   xxx
     2C99   368B   6.   `   xxx
     2C9B   310D   1.  a    print quality ESC a 0 = correspondence 3103 ESC a 1 = draft 3119 ESC a 2 = NLQ 30f5
     2C9D   368B   6.  b    xxx
     2C9F   31E3   1�  c    reset defaults
     2CA1   368B   6.  d   xxx
     2CA3   2CF4   ,�  e   semicondensed ESC e 13.4 mvi b,$28
     2CA5   32F4   2�  f   forward line feed
     2CA7   332B   3+  g   print line of graphics g nnn
     2CA9   368B   6.  h   xxx
     2CAB   368B   6.  i   xxx
     2CAD   368B   6.  j   xxx
     2CAF   368B   6.  k   xxx
     2CB1   3251   2Q  l   linefeed 0 or 1
     2CB3   3103   1.  m   select correspondence font
     2CB5   2CF1   ,�  n   ESC n = 9cpi   mvi b,$20
     2CB7   30F0   0�  o   paper out sensor o
     2CB9   2CFD   ,�  p   ESC p = 144dpi  mvi b,$78
     2CBB   2CFA   ,�  q   ESC q = 15 cpi  mvi b,$30
     2CBD   32FC   2�  r   reverse line feed
     2CBF   2E3D   .=  s   s n set dot spacing for proportional
     2CC1   368B   6.  t   xxx
     2CC3   2E5A   .Z  u   ESC u nnn add one tab stop  ?
     2CC5   3263   2c  v   set TOF to current position  ?
     2CC7   2FF1   /�  w   start half height text  ESC W is stop
     2CC9   3003   0.  x   start superscript
     2CCB   300E   0.  y   start subscript
     2CCD   3019   0.  z   stop super or subscript

CONTROL CODES TABLE

     28B0  1B 07 2C   ..,    2c07   ESC    (encounter an ESC code, jump to 2c07)
     28B3  09 18 37    .7    3718   ctrl+i (moves printer to next tab)
     28B6  0A 99 37   ..7    3799   ctrl+j  (feeds paper one line)
     28B9  0B A4 37   .�7    37a4   ctrl+k  (vertical tab???)
     28BC  0C B1 37   .�7    37b1   ctrl+l  (form feed)
     28BF  0D F5 36   .�6    36f5   ctrl+m  (execute carriage return)
     28C2  0E 0D 37   ..7    370d   ctrl+n
     28C5  0F 06 37   ..7    3706   ctrl+o
     28C8  13 CB 37   .�7    37cb   ctrl+s (deselect printer)
     28CB  18 C8 37   .�7    37c8   ctrl+x (erase current line from buffer)
     28CE  1D D7 37   .�7    37d7   CTRL+] GS  (???)
     28D1  1F 48 37   .H7    3748   feed 1 to 15 lines of blank paper
     28D4  FF                       end of table




pa0-7   1-8

pa0-3   lf abcd phase

pa4=pe lp           paper error LP  PA,4
pa5=sel lp
pa6=norm lp
pa7=draft lp

pb0-7   9-16

pb0-3 ribbon motor
pb4-5 sfmotor
1516 nc

pc0 = 17 = txdb
pc1 = 18 = rxd
pc2 = 19 = sck (according to pg13 should be 16x the baud rate 9600 = 153.6khz)
pc3 = 20 = CK64 = TI/INT2
pc4 = 21 = NC  (TIMER OUTPUT PIN), is it really NC? how does it control the baud rate?

pc5 = 22 = rxrdy     pc5= counter input

pc6 = 23 = dtrb
pc7 = 24 = asyn/apbus interface switching

pt0-7 34-41
01 pt0 sel sw
02 pt1 lf sw
04 pt2 tof sw
08 pt3 set sw
10 pt4 pe sw       paper end
20 pt5 cv open    reads at b6f (calt 0092)  closed = low open=high
40 pt6 col rbn
80 pt7 csf on

***************************************************************************/


#include "emu.h"
#include "imagewriter2_printer.h"
//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"
#include "imagewriter2_printer.lh"

DEFINE_DEVICE_TYPE(APPLE_IMAGEWRITER2_PRINTER, apple_imagewriter2_printer_device, "apple_imagewriter2", "Apple ImageWriter 2 Printer")


apple_imagewriter2_printer_device::apple_imagewriter2_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: apple_imagewriter2_printer_device(mconfig, APPLE_IMAGEWRITER2_PRINTER, tag, owner, clock)
{
}



apple_imagewriter2_printer_device::apple_imagewriter2_printer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock),
	device_rs232_port_interface(mconfig, *this),
	m_maincpu(*this, "maincpu"),
	m_bitmap_printer(*this, "bitmap_printer"),
	m_pf_stepper(*this, "pf_stepper"),
	m_cr_stepper(*this, "cr_stepper"),
	m_timer_clk64(*this, "gate_array_clk64"),
	m_pa_led(*this, "pa_led"),
	m_pb_led(*this, "pb_led"),
	m_pc_led(*this, "pc_led"),
	m_pt_led(*this, "pt_led")
{
}

void apple_imagewriter2_printer_device::device_add_mconfig(machine_config &config)
{
	// basic machine hardware
	upd7807_device &cpu(UPD7807(config, m_maincpu, baseCLK )); // 12Mhz xtal
	cpu.set_addrmap(AS_PROGRAM, &apple_imagewriter2_printer_device::mem_map);

	m_maincpu->pa_out_cb().set(FUNC(apple_imagewriter2_printer_device::porta_w));
	m_maincpu->pb_out_cb().set(FUNC(apple_imagewriter2_printer_device::portb_w));
	m_maincpu->pt_in_cb().set(FUNC(apple_imagewriter2_printer_device::portt_r));

	BITMAP_PRINTER(config, m_bitmap_printer, PAPER_WIDTH, PAPER_HEIGHT, dpi, dpi);
	m_bitmap_printer->set_screen_update(FUNC(apple_imagewriter2_printer_device::screen_update_bitmap));

	STEPPER(config, m_pf_stepper, (uint8_t) 0xa);
	STEPPER(config, m_cr_stepper, (uint8_t) 0xa);

	TIMER(config, m_timer_clk64, 0);
	m_timer_clk64->configure_periodic(FUNC(apple_imagewriter2_printer_device::pulse_clk64_clock), attotime::from_hz( 8.6E6 / 64 ));

	config.set_default_layout(layout_imagewriter2_printer);
}

//-------------------------------------------------
//  Memory Map
//-------------------------------------------------

void apple_imagewriter2_printer_device::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0).nopw();  // main rom

	map(0xa000, 0xbfff).ram();  // ram on the imagewriter ii  NEC d4168c-15

	map(0xfe00, 0xfeff).rw(FUNC(apple_imagewriter2_printer_device::gatearray_r), FUNC(apple_imagewriter2_printer_device::gatearray_w));
}


//-------------------------------------------------
//  ROM definition
//-------------------------------------------------

ROM_START(apple_imagewriter2_printer)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("imagewriter2new.bin", 0x0000, 0x8000, CRC(8f5d3363) SHA1(e7dee1661cd0fa72350b32a60abd4625a4f3ccff))
ROM_END

//-------------------------------------------------
//  Tiny rom entry
//-------------------------------------------------

const tiny_rom_entry *apple_imagewriter2_printer_device::device_rom_region() const
{
	return ROM_NAME(apple_imagewriter2_printer);
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

#define PORT_ADJUSTER_16MASK(_default, _name)                   \
		configurer.field_alloc(IPT_ADJUSTER, (_default), 0xffff, (_name)); \
		configurer.field_set_min_max(0, 100);


static INPUT_PORTS_START( apple_imagewriter2_printer )

	PORT_START("TOPMARGIN")
	PORT_ADJUSTER_16MASK(18, "Top Margin")
	PORT_MINMAX(0,500)

	PORT_START("BOTTOMMARGIN")
	PORT_ADJUSTER_16MASK(18, "Bottom Margin")
	PORT_MINMAX(0,500)

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Reset Printer") PORT_CODE(KEYCODE_8_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, apple_imagewriter2_printer_device, reset_sw, 0)


	// Buttons on printer
	PORT_START("SELECT")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Select Printer") PORT_CODE(KEYCODE_9_PAD)
	PORT_START("FORMFEED")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Form Feed") PORT_CODE(KEYCODE_2_PAD) PORT_TOGGLE
	PORT_START("LINEFEED")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Line Feed") PORT_CODE(KEYCODE_3_PAD) PORT_TOGGLE
	PORT_START("QUALITY")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Print Quality") PORT_CODE(KEYCODE_6_PAD)
	PORT_START("PAPEREND")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Paper End Sensor") PORT_CODE(KEYCODE_7_PAD) PORT_TOGGLE
	PORT_START("COVER")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Carrier Cover") PORT_CODE(KEYCODE_4_PAD) PORT_TOGGLE // Active high when cover open?

	PORT_START("RIBBON")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ribbon Switch") PORT_CODE(KEYCODE_5_PAD) PORT_TOGGLE // Active high when cover open?

	PORT_START("SHEETFEED")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SheetFeed Switch") PORT_CODE(KEYCODE_1_PAD)


	PORT_START("INTERRUPTS")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("INTERRUPTS") PORT_CODE(KEYCODE_0_PAD)


	PORT_START("F1")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("INTERRUPTSF1") PORT_CODE(KEYCODE_1)

	PORT_START("F2")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("INTERRUPTSF2") PORT_CODE(KEYCODE_2)

	PORT_START("READ0")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("READ") PORT_CODE(KEYCODE_SPACE)

	PORT_START("RETVAL0")
	PORT_ADJUSTER_16MASK(0x10, "ReturnVal")
	PORT_MINMAX(0,255)



	// DIPSW1
	PORT_START("DIPSW1")
	PORT_DIPNAME(0x07, 0x07, "International characters and PC selection")
	PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(0x07, "American") // default
	PORT_DIPSETTING(0x04, "British")
	PORT_DIPSETTING(0x03, "German")
	PORT_DIPSETTING(0x01, "French")
	PORT_DIPSETTING(0x02, "Swedish")
	PORT_DIPSETTING(0x06, "Italian")
	PORT_DIPSETTING(0x00, "Spanish")
	PORT_DIPSETTING(0x05, "American")  // duplicate american setting

	PORT_DIPNAME(0x08, 0x08, "Page length")
	PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(0x08, "66 lines (11 inches)") // default
	PORT_DIPSETTING(0x00, "72 lines (12 inches)")

	PORT_DIPNAME(0x10, 0x00, "Perforation Skip")
	PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(0x10, "Perforation Skip Off")
	PORT_DIPSETTING(0x00, "Perforation Skip On") // default

	PORT_DIPNAME(0x60, 0x60, "Character Set")
	PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(0x60, "Pica (10 cpi)") // default
	PORT_DIPSETTING(0x40, "Elite (12 cpi)")
	PORT_DIPSETTING(0x20, "Ultracondensed (17 cpi)")
	PORT_DIPSETTING(0x00, "Elite Proportional")

	PORT_DIPNAME(0x80, 0x00, "Line Feed")
	PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(0x00, "Auto Add LF after CR")
	PORT_DIPSETTING(0x80, "No LF after CR") // default

	// DIPSW2
	PORT_START("DIPSW2")
	PORT_DIPNAME(0x03, 0x00, "Baud Rate")
	PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(0x00, "9600") // default
	PORT_DIPSETTING(0x01, "2400")
	PORT_DIPSETTING(0x02, "1200")
	PORT_DIPSETTING(0x03, "300")

/*

FE04 reads DIPSW2

021d: 70 69 01 ff  MOV     A,($FF01)
0221: 07 03        ANI     A,$03
0223: 34 6b 03     LXI     HL,$036B
0226: ac           LDAX    (HL+A)
0227: 4d da        MOV     TM0,A

036b: 40 10 08 02

02 = 9600
08 = 2400
10 = 1200
40 = 300

how does this change the SCK?  TI  TO?

023a: 34 60 fe     LXI     HL,$FE60          DIPSW1
023d: 2b           LDAX    (HL)
023e: 16 ff        XRI     A,$FF
0240: 70 79 00 ff  MOV     ($FF00),A
0244: 34 04 fe     LXI     HL,$FE04          DIPSW2
0247: 2b           LDAX    (HL)
0248: 16 ff        XRI     A,$FF
024a: 07 0f        ANI     A,$0F
024c: 70 6a 01 ff  MOV     B,($FF01)
0250: 74 0a 80     ANI     B,$80
0253: 60 9a        ORA     A,B
0255: 70 79 01 ff  MOV     ($FF01),A
0259: b8           RET



*/

	PORT_DIPNAME(0x04, 0x04, "Flow Control")
	PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(0x04, "Data Terminal Ready") // default
	PORT_DIPSETTING(0x00, "XON/XOFF")

	PORT_DIPNAME(0x08, 0x08, "Option Card")
	PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(0x08, "No Option Card")
	PORT_DIPSETTING(0x00, "Option Card Enabled") // default


	PORT_DIPNAME(0x30, 0x00, "Hammer Fire Timing Bidirectional")
	PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(0x00, "Fire 0")
	PORT_DIPSETTING(0x10, "Fire 1") // default
	PORT_DIPSETTING(0x20, "Fire 2")
	PORT_DIPSETTING(0x30, "Fire 3") // default



INPUT_PORTS_END

//-------------------------------------------------
//    io port constructor
//-------------------------------------------------

ioport_constructor apple_imagewriter2_printer_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( apple_imagewriter2_printer );
}



//-------------------------------------------------
//    Input Changed Member Select Switch
//-------------------------------------------------




uint8_t apple_imagewriter2_printer_device::gatearray_r(offs_t offset)
{
	u8 data = 0x0;
	switch (offset)
	{
		case 0x60:  // FE60   (guess that high FE numbers go to the gate array)
			data = ioport("DIPSW1")->read();
			break;
		case 0x61: // FE61
		{
			static u8 count = 0;
			data = ioport("READ0")->read() ? count++  : ioport("RETVAL0")->read();
			break;
		}
		case 0x04:  // FE04   (guess that low FE0... goes to the other chip)
			data = ioport("DIPSW2")->read();
			break;

		default:
			break;
	}
	LOG("%s: gatearray_r(%02x): data=%d\n", machine().describe_context(), offset, data);
	printf("%s: gatearray_r(%02x): data=%x\n", machine().describe_context().c_str(), offset, data);
	return data;
}


void apple_imagewriter2_printer_device::gatearray_w(offs_t offset, uint8_t data)
{
	LOG("%s: gatearray_w(%02x): data=%d\n", machine().describe_context(), offset, data);
	switch (offset)
	{
	case 0x73:
		{
			u8 orig = BIT(data,0,4);
//          u8 newval = bitswap<4>(BIT(data,0,4) ^ 0xff, 1, 2, 3, 0);
			u8 newval = bitswap<4>(BIT(data,0,4) ^ 0xff, 0, 2, 1, 3);
			printf("orig,new : %x = %x \n", orig, newval);
//          update_cr_stepper(bitswap<4>(BIT(data,0,4) ^ 0xff, 1, 2, 3, 0));
			update_cr_stepper(bitswap<4>(BIT(data,0,4) ^ 0xff, 0, 2, 1, 3));
		}
		break;
	default:
		break;
	}
	printf("%s: gatearray_w(%02x): data=%x\n", machine().describe_context().c_str(), offset, data);

}



//-------------------------------------------------
//    7807
//-------------------------------------------------

uint8_t apple_imagewriter2_printer_device::portt_r()
{
	// unimplemented
	// connects to fault* on serial interface  pin 14  (secondary cts)
	// Apple II super serial card has a secondary cts
	//  printf("MAINCPU OUT SOD FUNCTION VALUE = %x   TIME = %f  %s\n",data, machine().time().as_double(), machine().describe_context().c_str());


	// PT0 (34) = SEL SW  (select)
	// PT1 (35) = LF SW   (line feed)
	// PT2 (36) = TOF SW  (form feed)
	// PT3 (37) = SET SW  (print quality)
	// PT4 (38) = PE      (paper error)
	// PT5 (39) = CV OPEN (cover open)
	// PT6 (40) = COL RBN (color ribbon)
	// PT7 (41) = CSF ON  (sheet feeder)

	u8 data =
			(ioport("SELECT")->read()                 << 0) | //
			(ioport("LINEFEED")->read()               << 1) | //
			(ioport("FORMFEED")->read()               << 2) | //
			(ioport("QUALITY")->read()                << 3) | //
			(ioport("PAPEREND")->read()               << 4) | //
			(ioport("COVER")->read()                  << 5) | //
			(ioport("RIBBON")->read()                 << 6) | //
			(ioport("SHEETFEED")->read()              << 7);  //
	m_pt_led = data;
	return data;
}

void apple_imagewriter2_printer_device::porta_w(uint8_t data)
{
	m_porta = data;
	m_pa_led = data;
}
void apple_imagewriter2_printer_device::portb_w(uint8_t data)
{
	m_portb = data;
	m_pa_led = data;
}




INPUT_CHANGED_MEMBER(apple_imagewriter2_printer_device::reset_sw)
{
	if (newval == 0) m_maincpu->reset();
}


//-------------------------------------------------
//    Update Stepper and return delta
//-------------------------------------------------

int apple_imagewriter2_printer_device::update_stepper_delta(stepper_device * stepper, uint8_t pattern, const char * name, int direction)
{
	int lastpos = stepper->get_absolute_position();
	stepper->update(bitswap<4>(pattern, 0, 2, 1, 3));  // drive pattern is the "standard" reel pattern when bits 1,2 swapped
	int delta = stepper->get_absolute_position() - lastpos;
	return delta * direction;
}

//-------------------------------------------------
//    Update Head Position
//-------------------------------------------------

void apple_imagewriter2_printer_device::update_head_pos()
{
	m_bitmap_printer->setheadpos( std::max(0, x_pixel_coord(m_xpos)), // keep printhead visible on left of screen
									y_pixel_coord(m_ypos));
}

//-------------------------------------------------
//    Update Paper Feed Stepper
//-------------------------------------------------

void apple_imagewriter2_printer_device::update_pf_stepper(uint8_t vstepper)
{
	int delta = update_stepper_delta(m_pf_stepper, vstepper, "PF", -1);

	if (delta > 0)
	{
		m_ypos += delta; // move down

		if (newpageflag == 1)
		{
			m_ypos = ioport("TOPMARGIN")->read();  // lock to the top of page until we seek horizontally

		}
		if (y_pixel_coord(m_ypos) > PAPER_HEIGHT - 1 - ioport("BOTTOMMARGIN")->read())  // i see why it's failing
			// if we are within 50 pixels of the bottom of the page we will
			// write the page to a file, then erase the top part of the page
			// so we can still see the last page printed.
		{
			// clear paper to bottom from current position
			m_bitmap_printer->bitmap_clear_band(y_pixel_coord(m_ypos) + 7, PAPER_HEIGHT - 1, rgb_t::white());

			// save a snapshot with the slot and page as part of the filename
			m_bitmap_printer->write_snapshot_to_file();

			newpageflag = 1;
			// clear page down to visible area, starting from the top of page
			m_bitmap_printer->bitmap_clear_band(0, PAPER_HEIGHT - 1 - PAPER_SCREEN_HEIGHT, rgb_t::white());

			m_ypos = ioport("TOPMARGIN")->read();  // lock to the top of page until we seek horizontally
		}
		// clear page down to visible area
		m_bitmap_printer->bitmap_clear_band(y_pixel_coord(m_ypos) + distfrombottom, std::min(y_pixel_coord(m_ypos) + distfrombottom+30, PAPER_HEIGHT - 1), rgb_t::white());

	}
	else if (delta < 0) // we are moving up the page
	{
		m_ypos += delta;
		if (m_ypos < 0) m_ypos = 0;  // don't go backwards past top of page
	}
	update_head_pos();
}

//-------------------------------------------------
//    Update Carriage Stepper
//-------------------------------------------------

void apple_imagewriter2_printer_device::update_cr_stepper(uint8_t hstepper)
{
	int delta = update_stepper_delta(m_cr_stepper, hstepper, "CR", 1);

	if (delta != 0)
	{
		newpageflag = 0;

		if (delta > 0)
		{
			m_xpos += delta; xdirection = 1;
		}
		else if (delta < 0)
		{
			m_xpos += delta; xdirection = -1;
		}
	}
	update_head_pos();
}

//-------------------------------------------------
//    Device Start
//-------------------------------------------------

void apple_imagewriter2_printer_device::device_start()
{
	m_pa_led.resolve();
	m_pb_led.resolve();
	m_pc_led.resolve();
	m_pt_led.resolve();

	save_item(NAME(left_offset));
	save_item(NAME(right_offset));
	save_item(NAME(m_left_edge));
	save_item(NAME(m_right_edge));
	save_item(NAME(xposratio0));
	save_item(NAME(xposratio1));
	save_item(NAME(yposratio0));
	save_item(NAME(yposratio1));
	save_item(NAME(PAPER_WIDTH_INCHES));
	save_item(NAME(PAPER_WIDTH));
}

//-------------------------------------------------
//    Device Reset
//-------------------------------------------------

void apple_imagewriter2_printer_device::device_reset()
{
}

//-------------------------------------------------
//    IO port safe read
//-------------------------------------------------

int apple_imagewriter2_printer_device::ioportsaferead(const char * name)
{
	// Safe read of ioport (mame does not allow ioport read at init time)
	// Avoids the following error:
	//   Ignoring MAME exception: Input ports cannot be read at init time!
	//   Fatal error: Input ports cannot be read at init time!
	if (ioport(name)->manager().safe_to_read()) return ioport(name)->read();
	else return 0;
}


TIMER_DEVICE_CALLBACK_MEMBER (apple_imagewriter2_printer_device::pulse_clk64_clock)
{
	if (!ioport("INTERRUPTS")->read()) return;

	m_int2 = !m_int2;


	if (ioport("F1")->read())
	m_maincpu->set_input_line(UPD7810_INTF1, m_int2 ? ASSERT_LINE : CLEAR_LINE );

	if (ioport("F2")->read())
	m_maincpu->set_input_line(UPD7810_INTF2, m_int2 ? ASSERT_LINE : CLEAR_LINE );

}



uint32_t apple_imagewriter2_printer_device::screen_update_bitmap(screen_device &screen,
							 bitmap_rgb32 &bitmap, const rectangle &cliprect)
{

	static constexpr int size=40;
	for (int b=0;b<8;b++) bitmap.plot_box(1*(size*12)+(size/2)*(b>3)+b*size,size*2,size - 2,size - 2, m_porta & (1<<(7-b)) ? (((7-b)==4) ? 0xff0000 : 0x00ff00 ) : 0x0 );
	for (int b=0;b<8;b++) bitmap.plot_box(1*(size*12)+(size/2)*(b>3)+b*size,size*4,size - 2,size - 2, m_portb & (1<<(7-b)) ? 0x00ff00 : 0x0 );
	u8 portt = portt_r();
	for (int b=0;b<8;b++) bitmap.plot_box(1*(size*12)+(size/2)*(b>3)+b*size,size*6,size - 2,size - 2, portt & (1<<(7-b)) ? 0x00ff00 : 0x0 );


	return 0;
}
