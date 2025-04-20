// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Wilbert Pol, David Shah
/***************************************************************************

  Atari VCS 2600 driver

TODO:
- Move the 2 32-in-1 rom dumps into their own driver
- Add 128-in-1 driver


  Systema TV Boy

TODO:
- The TV Boy II and Super TV Boy units did not have joystick ports but a builtin D-pad
- Find, dump and add the other variants. There are many. Systema and Akor released PAL TV Boys.
  NICS released NTSC TV Boys in the US and Japan. The US NICS unit is also seen with the name Mega-Game.
  Mega Boy WS-11 is another rebranded model.
  There's also the TV Boy 3, Portable Walkiecom PTG-2600, Compact Master Boy, Game Link 2001, etc.


  Atari A2600 Point of Purchase Display Unit

TODO:
- Does the reset by pressing game select buttons only work on the infinite
  timer mode or also in the other modes?
- Asteroids is an 8KB software title with banking. Does the display unit
  use a special 4KB version or is there banking hardware installed?
- 2 joysticks and 2 paddles are hooked up to the unit; unknown how.


Selection buttons on the kiosk unit:
2 "Press to select game number" buttons
1 "Press to start" button


Switch A8 controls the gameplay duration. Factory default is one minute
gameplay duration.
1 - 2 minute gameplay
2 - 1 minute gameplay
3 - 30 second gameplay
4 - Infinite gameplay (gameplay allowed until end of game or until a "Press
     to select game number" button is pressed)

This seems to be completely hardware controlled; it is never queried or set up
by the game selection rom.


The POP unit comes with the following 42 games installed:
Location Game
D6 Prog ROM 1 ADVENTURE (CX2613)
E6 Prog ROM 2 AIR-SEA BATTLE (CX2602)
F6 Prog ROM 3 ASTEROIDS (CX2649)
J6 Prog ROM 4 BACKGAMMON (CX2617)
K6 Prog ROM 5 BASKETBALL (CX2624)
L6 Prog ROM 6 BOWLING (CX2628)
M6 Prog ROM 7 BREAKOUT (CX2622)
C5 Prog ROM 8 CANYON BOMBER (CX2607)
D5 Prog ROM 9 CASINO (CX2652)
E5 Prog ROM 10 CIRCUS ATARI (CX2630)
F5 Prog ROM 11 COMBAT (CX2601)
J5 Prog ROM 12 DODGE'EM (CX2637)
K5 Prog ROM 13 FOOTBALL (CX2625)
L5 Prog ROM 14 GOLF (CX2634)
M5 Prog ROM 15 HANGMAN (CX2662)
C4 Prog ROM 16 HOMERUN (CX2623)
D4 Prog ROM 17 HUMAN CANNONBALL (CX2627)
E4 Prog ROM 18 MAZE CRAZE (CX2635)
F4 Prog ROM 19 MISSILE COMMAND (CX2638)
J4 Prog ROM 20 NIGHT DRIVER (CX2633)
K4 Prog ROM 21 OTHELLO (CX2639)
L4 Prog ROM 22 OUTLAW (CX2605)
M4 Prog ROM 23 SOCCER (CX2616)
C3 Prog ROM 24 SKYDIVER (CX2629)
D3 Prog ROM 25 SLOT RACERS (CX2606)
E3 Prog ROM 26 SPACE INVADERS (CX2632)
F3 Prog ROM 27 STREET RACER (CX2612)
J3 Prog ROM 28 SUPERMAN (CX2631)
K3 Prog ROM 29 3D TIC-TAC-TOE (CX2618)
L3 Prog ROM 30 VIDEO CHECKERS (CX2636)
M3 Prog ROM 31 VIDEO CHESS (CX2645)
C2 Prog ROM 32 VIDEO OLYMPICS (CX2621)
D2 Prog ROM 33 VIDEO PINBALL (CX2648)
E2 Prog ROM 34 WARLORDS (CX2610)
F2 Prog ROM 35 BERZERK (CX2650)
J2 Prog ROM 36 HAUNTED HOUSE (CX2654)
K2 Prog ROM 37 MATH GRAND PRIX (CX2658)
L2 Prog ROM 38 DEFENDER (CX2609)
M2 Prog ROM 39 YAR'S REVENGE (CX2655)
C1 Prog ROM 40 PAC-MAN (CX2646)
D1 Prog ROM 41 SUPER BREAKOUT (CX2608)
E1 Prog ROM 42 DEMONS TO DIAMONDS (CX2615)

Sears Video Game Selection Center has the same 42 games
(games with Sears Tele-Games equivalent have Tele-Games naming)
D6 Prog ROM 1 ADVENTURE (CX2613)
E6 Prog ROM 2 AIR-SEA BATTLE (CX2602) = Target Fun
F6 Prog ROM 3 ASTEROIDS (CX2649)
J6 Prog ROM 4 BACKGAMMON (CX2617)
K6 Prog ROM 5 BASKETBALL (CX2624)
L6 Prog ROM 6 BOWLING (CX2628)
M6 Prog ROM 7 BREAKOUT (CX2622) = Breakaway
C5 Prog ROM 8 CANYON BOMBER (CX2607)
D5 Prog ROM 9 CASINO (CX2652) = Poker Plus
E5 Prog ROM 10 CIRCUS ATARI (CX2630) = Circus
F5 Prog ROM 11 COMBAT (CX2601) = Tank Plus
J5 Prog ROM 12 DODGE'EM (CX2637) = Dodger Cars
K5 Prog ROM 13 FOOTBALL (CX2625)
L5 Prog ROM 14 GOLF (CX2634)
M5 Prog ROM 15 HANGMAN (CX2662)
C4 Prog ROM 16 HOMERUN (CX2623) = Baseball
D4 Prog ROM 17 HUMAN CANNONBALL (CX2627) = Cannonman
E4 Prog ROM 18 MAZE CRAZE (CX2635) = Maze Mania
F4 Prog ROM 19 MISSILE COMMAND (CX2638)
J4 Prog ROM 20 NIGHT DRIVER (CX2633)
K4 Prog ROM 21 OTHELLO (CX2639)
L4 Prog ROM 22 OUTLAW (CX2605) = Gunslinger
M4 Prog ROM 23 SOCCER (CX2616)
C3 Prog ROM 24 SKYDIVER (CX2629) = Darediver
D3 Prog ROM 25 SLOT RACERS (CX2606) = Maze
E3 Prog ROM 26 SPACE INVADERS (CX2632)
F3 Prog ROM 27 STREET RACER (CX2612) = Speedway II
J3 Prog ROM 28 SUPERMAN (CX2631)
K3 Prog ROM 29 3D TIC-TAC-TOE (CX2618) = Tic-Tac-Toe
L3 Prog ROM 30 VIDEO CHECKERS (CX2636) = Checkers
M3 Prog ROM 31 VIDEO CHESS (CX2645) = Chess
C2 Prog ROM 32 VIDEO OLYMPICS (CX2621) = Pong Sports
D2 Prog ROM 33 VIDEO PINBALL (CX2648) = Pinball
E2 Prog ROM 34 WARLORDS (CX2610)
F2 Prog ROM 35 BERZERK (CX2650)
J2 Prog ROM 36 HAUNTED HOUSE (CX2654)
K2 Prog ROM 37 MATH GRAND PRIX (CX2658)
L2 Prog ROM 38 DEFENDER (CX2609)
M2 Prog ROM 39 YAR'S REVENGE (CX2655)
C1 Prog ROM 40 PAC-MAN (CX2646)
D1 Prog ROM 41 SUPER BREAKOUT (CX2608)
E1 Prog ROM 42 DEMONS TO DIAMONDS (CX2615)

and adds 3 more games that were Sears exclusives:

F1 Prog ROM 43 Submarine Commander (49-75142)
J1 Prog ROM 44 Stellar Track (49-75159)  (unnamed blank entry 44 in game list)
K1 Prog ROM 45 Steeple Chase (49-75126)
***************************************************************************/

#include "emu.h"


#include "bus/vcs/compumat.h"
#include "bus/vcs/dpc.h"
#include "bus/vcs/harmony_melody.h"
#include "bus/vcs/rom.h"
#include "bus/vcs/scharger.h"
#include "bus/vcs/vcs_slot.h"
#include "bus/vcs_ctrl/ctrl.h"
#include "cpu/m6502/m6507.h"
#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "sound/tiaintf.h"
#include "speaker.h"
#include "tia.h"

#include "machine/mos6530.h"

#include "formats/compumate_cas.h"
#include "imagedev/cassette.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"


namespace {

static const uint16_t supported_screen_heights[4] = { 262, 312, 328, 342 };


class a2600_base_state : public driver_device
{
protected:
	a2600_base_state(const machine_config &mconfig, device_type type, const char *tag, XTAL xtal) :
		driver_device(mconfig, type, tag),
		m_riot_ram(*this, "riot_ram"),
		m_tia(*this, "tia_video"),
		m_maincpu(*this, "maincpu"),
		m_riot(*this, "riot"),
		m_joy1(*this, "joyport1"),
		m_joy2(*this, "joyport2"),
		m_screen(*this, "screen"),
		m_xtal(xtal)
	{ }

	virtual void machine_start() override ATTR_COLD;

	void a2600_mem(address_map &map) ATTR_COLD;

	void a2600_base_ntsc(machine_config &config);
	void a2600_base_pal(machine_config &config);

	void switch_A_w(uint8_t data);
	uint8_t switch_A_r();
	void switch_B_w(uint8_t data);
	void irq_callback(int state);
	uint16_t a2600_read_input_port(offs_t offset);
	uint8_t a2600_get_databus_contents(offs_t offset);
	void a2600_tia_vsync_callback(uint16_t data);

	required_shared_ptr<uint8_t> m_riot_ram;
	required_device<tia_video_device> m_tia;
	required_device<m6507_device> m_maincpu;
	required_device<mos6532_device> m_riot;
	required_device<vcs_control_port_device> m_joy1;
	required_device<vcs_control_port_device> m_joy2;
	required_device<screen_device> m_screen;

private:
	uint16_t m_current_screen_height = 0U;
	XTAL m_xtal;
};


class a2600_cons_state : public a2600_base_state
{
public:
	a2600_cons_state(const machine_config &mconfig, device_type type, const char *tag, XTAL xtal) :
		a2600_base_state(mconfig, type, tag, xtal),
		m_cartslot(*this, "cartslot")
	{ }

protected:
	void a2600_cartslot(machine_config &config);

private:
	required_device<vcs_cart_slot_device> m_cartslot;
};


class a2600_state : public a2600_cons_state
{
public:
	a2600_state(const machine_config &mconfig, device_type type, const char *tag) :
		a2600_cons_state(mconfig, type, tag, 3.579575_MHz_XTAL)
	{ }

	void a2600(machine_config &config);
};


class a2600p_state : public a2600_cons_state
{
public:
	a2600p_state(const machine_config &mconfig, device_type type, const char *tag) :
		a2600_cons_state(mconfig, type, tag, 3.546894_MHz_XTAL)
	{ }

	void a2600p(machine_config &config);
};


class a2600_pop_state : public a2600_base_state
{
public:
	a2600_pop_state(const machine_config &mconfig, device_type type, const char *tag)
		: a2600_base_state(mconfig, type, tag, 3.579545_MHz_XTAL)
		, m_bank(*this, "bank")
		, m_a7(*this, "A7")
		, m_a8(*this, "A8")
		, m_swb(*this, "SWB")
	{ }

	void a2600_pop(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void memory_map(address_map &map) ATTR_COLD;

	uint8_t rom_switch_r(offs_t offset);
	void rom_switch_w(offs_t offset, uint8_t data);
	TIMER_CALLBACK_MEMBER(reset_timer_callback);
	TIMER_CALLBACK_MEMBER(game_select_button_timer_callback);

	required_memory_bank m_bank;
	required_ioport m_a7;
	required_ioport m_a8;
	required_ioport m_swb;
	emu_timer *m_reset_timer = nullptr;
	emu_timer *m_game_select_button_timer = nullptr;
};

class tvboy_state : public a2600_base_state
{
public:
	tvboy_state(const machine_config &mconfig, device_type type, const char *tag)
		: a2600_base_state(mconfig, type, tag, 3.546894_MHz_XTAL)
		, m_crom(*this, "crom")
		, m_rom(*this, "mainrom")
	{ }

	void tvboy(machine_config &config);
	void tvboyn(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void bank_write(offs_t offset, uint8_t data);

	void tvboy_mem(address_map &map) ATTR_COLD;

	required_memory_bank m_crom;
	required_region_ptr<uint8_t> m_rom;
};


void a2600_base_state::a2600_mem(address_map &map) // 6507 has 13-bit address space, 0x0000 - 0x1fff
{
	map(0x0000, 0x007f).mirror(0x0f00).rw(m_tia, FUNC(tia_video_device::read), FUNC(tia_video_device::write));
	map(0x0080, 0x00ff).mirror(0x0d00).ram().share("riot_ram");
	map(0x0280, 0x029f).mirror(0x0d00).m("riot", FUNC(mos6532_device::io_map));
}

void a2600_pop_state::memory_map(address_map &map) // 6507 has 13-bit address space, 0x0000 - 0x1fff
{
	a2600_base_state::a2600_mem(map);
	map(0x0800, 0x0800).rw(FUNC(a2600_pop_state::rom_switch_r), FUNC(a2600_pop_state::rom_switch_w));
	map(0x1000, 0x1fff).bankr(m_bank);
}

void tvboy_state::tvboy_mem(address_map &map)
{ // 6507 has 13-bit address space, 0x0000 - 0x1fff
	a2600_base_state::a2600_mem(map);
	map(0x1800, 0x1fff).w(FUNC(tvboy_state::bank_write));
	map(0x1000, 0x1fff).bankr(m_crom);
}


//  read returns number of empty game rom slots
uint8_t a2600_pop_state::rom_switch_r(offs_t offset)
{
	return m_a7->read();
//  return 5;   // Max 47 games, 5 empty slots => 42 games
}


//  Rom switch
void a2600_pop_state::rom_switch_w(offs_t offset, uint8_t data)
{
	m_bank->set_entry(data & 0x7f);
	if (data & 0x80)
	{
		// (re)start reset timer?
		// Unknown what happens when multiple dipswitches are set
		uint8_t a8 = m_a8->read();
		attotime reset_time = attotime::never;
		if (a8 & 0x08)
		{
			// infinite
			// enable reset by pressing game selection button when on infinite timer
			m_game_select_button_timer->adjust(attotime::from_hz(60), 0, attotime::from_hz(60));
		}
		else if (a8 & 0x04)
		{
			// 30 seconds
			reset_time = attotime::from_seconds(30);
		}
		else if (a8 & 0x02)
		{
			// 1 minute
			reset_time = attotime::from_seconds(60);
		}
		else if (a8 & 0x01)
		{
			// 2 minutes
			reset_time = attotime::from_seconds(120);
		}
		m_reset_timer->adjust(reset_time);
	}
}

void tvboy_state::bank_write(offs_t offset, uint8_t data)
{
	LOG("banking (?) write %04x, %02x\n", offset, data);
	m_crom->set_entry(data);
}


TIMER_CALLBACK_MEMBER(a2600_pop_state::reset_timer_callback)
{
	machine().schedule_soft_reset();
}


TIMER_CALLBACK_MEMBER(a2600_pop_state::game_select_button_timer_callback)
{
	if ((m_swb->read() & 0x14) != 0x14) {
		machine().schedule_soft_reset();
	}
}


void a2600_base_state::switch_A_w(uint8_t data)
{
	/* Left controller port */
	m_joy1->joy_w( data >> 4 );

	/* Right controller port */
	m_joy2->joy_w( data & 0x0f );

//  switch( ioport("CONTROLLERS")->read() % 16 )
//  {
//  case 0x0a:  /* KidVid voice module */
//      m_cassette->change_state(( data & 0x02 ) ? CASSETTE_MOTOR_DISABLED : (CASSETTE_MOTOR_ENABLED | CASSETTE_PLAY), CASSETTE_MOTOR_DISABLED );
//      break;
//  }
}

uint8_t a2600_base_state::switch_A_r()
{
	uint8_t val = 0;

	// Left controller port PINs 1-4 ( 4321 )
	val |= (m_joy1->read_joy() & 0x0f) << 4;

	// Right controller port PINs 1-4 ( 4321 )
	val |= m_joy2->read_joy() & 0x0f;

	return val;
}

void a2600_base_state::switch_B_w(uint8_t data)
{
}

void a2600_base_state::irq_callback(int state)
{
}

uint16_t a2600_base_state::a2600_read_input_port(offs_t offset)
{
	switch (offset)
	{
	case 0: // Left controller port PIN 5
		return m_joy1->read_pot_x();

	case 1: // Left controller port PIN 9
		return m_joy1->read_pot_y();

	case 2: // Right controller port PIN 5
		return m_joy2->read_pot_x();

	case 3: // Right controller port PIN 9
		return m_joy2->read_pot_y();

	case 4: // Left controller port PIN 6
		return (m_joy1->read_joy() & 0x20) ? 0xff : 0x7f;

	case 5: // Right controller port PIN 6
		return (m_joy2->read_joy() & 0x20) ? 0xff : 0x7f;
	}
	return 0xff;
}

/* There are a few games that do an LDA ($80-$FF),Y instruction.
   The contents off the databus then depend on whatever was read
   from the RAM. To do this really properly the 6502 core would
   need to keep track of the last databus contents so we can query
   that. For now this is a quick hack to determine that value anyway.
   Examples:
   Q-Bert's Qubes (NTSC,F6) at 0x1594
   Berzerk at 0xF093.
*/
uint8_t a2600_base_state::a2600_get_databus_contents(offs_t offset)
{
	uint16_t  last_address, prev_address;
	uint8_t   last_byte, prev_byte;
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);

	last_address = m_maincpu->pc() + 1;
	if ( ! ( last_address & 0x1080 ) )
	{
		return offset;
	}
	last_byte = prog_space.read_byte(last_address );
	if ( last_byte < 0x80 || last_byte == 0xFF )
	{
		return last_byte;
	}
	prev_address = last_address - 1;
	if ( ! ( prev_address & 0x1080 ) )
	{
		return last_byte;
	}
	prev_byte = prog_space.read_byte(prev_address );
	if ( prev_byte == 0xB1 )
	{   /* LDA (XX),Y */
		return prog_space.read_byte(last_byte + 1 );
	}
	return last_byte;
}

#if 0
static const rectangle visarea[4] = {
	{ 26, 26 + 160 + 16, 24, 24 + 192 + 31 },   /* 262 */
	{ 26, 26 + 160 + 16, 32, 32 + 228 + 31 },   /* 312 */
	{ 26, 26 + 160 + 16, 45, 45 + 240 + 31 },   /* 328 */
	{ 26, 26 + 160 + 16, 48, 48 + 240 + 31 }    /* 342 */
};
#endif

void a2600_base_state::a2600_tia_vsync_callback(uint16_t data)
{
	for (int i = 0; i < std::size(supported_screen_heights); i++)
	{
		if (data >= supported_screen_heights[i] - 3 && data <= supported_screen_heights[i] + 3)
		{
			if (supported_screen_heights[i] != m_current_screen_height)
			{
				m_current_screen_height = supported_screen_heights[i];
//              m_screen->configure(228, m_current_screen_height, visarea[i], HZ_TO_ATTOSECONDS(m_xtal) * 228 * m_current_screen_height);
			}
		}
	}
}

void a2600_base_state::machine_start()
{
	m_current_screen_height = m_screen->height();
	memset(m_riot_ram, 0x00, 0x80);

	save_item(NAME(m_current_screen_height));
}

void tvboy_state::machine_start()
{
	a2600_base_state::machine_start();
	m_crom->configure_entries(0, m_rom.bytes() / 0x1000, &m_rom[0], 0x1000);
}

void a2600_pop_state::machine_start()
{
	a2600_base_state::machine_start();
	m_bank->configure_entries(0, 48, memregion("maincpu")->base(), 0x1000);
	m_bank->set_entry(0);
	m_reset_timer = timer_alloc(FUNC(a2600_pop_state::reset_timer_callback), this);
	m_game_select_button_timer = timer_alloc(FUNC(a2600_pop_state::game_select_button_timer_callback), this);
}


void tvboy_state::machine_reset()
{
	m_crom->set_entry(0);
	a2600_base_state::machine_reset();
}

void a2600_pop_state::machine_reset()
{
	a2600_base_state::machine_reset();

	m_bank->set_entry(0);
	m_reset_timer->adjust(attotime::never);
	m_game_select_button_timer->adjust(attotime::never);
}


static INPUT_PORTS_START( a2600 )
	PORT_START("SWB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Reset Game") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Select Game") PORT_CODE(KEYCODE_1)
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_CONFNAME( 0x08, 0x08, "TV Type" ) PORT_CODE(KEYCODE_C) PORT_TOGGLE
	PORT_CONFSETTING(    0x08, "Color" )
	PORT_CONFSETTING(    0x00, "B&W" )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_CONFNAME( 0x40, 0x00, "Left Diff. Switch" ) PORT_CODE(KEYCODE_3) PORT_TOGGLE
	PORT_CONFSETTING(    0x40, "A" )
	PORT_CONFSETTING(    0x00, "B" )
	PORT_CONFNAME( 0x80, 0x00, "Right Diff. Switch" ) PORT_CODE(KEYCODE_4) PORT_TOGGLE
	PORT_CONFSETTING(    0x80, "A" )
	PORT_CONFSETTING(    0x00, "B" )

	PORT_START("DUM0")  // for use by basic programming layout
	PORT_START("DUM1")
	PORT_START("DUM2")
INPUT_PORTS_END

static INPUT_PORTS_START(a2600_pop)
	PORT_START("SWB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Reset Game") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Select Game") PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Tens") PORT_CODE(KEYCODE_3)  // Tens
	PORT_CONFNAME( 0x08, 0x08, "TV Type" ) PORT_CODE(KEYCODE_C) PORT_TOGGLE
	PORT_CONFSETTING(    0x08, "Color" )
	PORT_CONFSETTING(    0x00, "B&W" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Ones")  PORT_CODE(KEYCODE_4) // Ones
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Start") PORT_CODE(KEYCODE_5) // Start
	// Left difficulty switch not present
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	// Right difficulty switch not present
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("A8")
	PORT_DIPNAME(0x0f, 0x02, "Gameplay duration") PORT_DIPLOCATION("A8:!1,!2,!3,!4")
	PORT_DIPSETTING(0x01, "2 minutes")
	PORT_DIPSETTING(0x02, "1 minute")
	PORT_DIPSETTING(0x04, "30 seconds")
	PORT_DIPSETTING(0x08, "Infinite")

	PORT_START("A7")
	PORT_DIPNAME(0x0f, 0x05, "Upper Rom Slots Free (47 - Free = Used)") PORT_DIPLOCATION("A7:1,2,3,4")
	PORT_DIPSETTING(0x00, "0 Free 47 Used")
	PORT_DIPSETTING(0x01, "1 Free 46 Used")
	PORT_DIPSETTING(0x02, "2 Free 45 Used Sears VGSC")
	PORT_DIPSETTING(0x03, "3 Free 44 Used")
	PORT_DIPSETTING(0x04, "4 Free 43 Used")
	PORT_DIPSETTING(0x05, "5 Free 42 Used Atari POP")
	PORT_DIPSETTING(0x06, "6 Free 41 Used")
	PORT_DIPSETTING(0x07, "7 Free 40 Used")
	PORT_DIPSETTING(0x08, "8 Free 39 Used")
	PORT_DIPSETTING(0x09, "9 Free 38 Used")
	PORT_DIPSETTING(0x0a, "10 Free 37 Used")
	PORT_DIPSETTING(0x0b, "11 Free 36 Used")
	PORT_DIPSETTING(0x0c, "12 Free 35 Used")
	PORT_DIPSETTING(0x0d, "13 Free 34 Used")
	PORT_DIPSETTING(0x0e, "14 Free 33 Used")
	PORT_DIPSETTING(0x0f, "15 Free 32 Used")
INPUT_PORTS_END

static INPUT_PORTS_START( tvboy )
	PORT_INCLUDE( a2600 )

	PORT_MODIFY("SWB")
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_UNUSED ) // No TV Type switch, hard coded to Color
	PORT_BIT ( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED ) // No Left Diff. switch, hard coded to B
	PORT_BIT ( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // No Right Diff. switch, hard coded to B
INPUT_PORTS_END


static void a2600_cart(device_slot_interface &device)
{
	device.option_add("a26_2k_4k", A26_ROM_2K_4K);
	device.option_add("a26_f4",    A26_ROM_F4);
	device.option_add("a26_f6",    A26_ROM_F6);
	device.option_add("a26_f8",    A26_ROM_F8);
	device.option_add("a26_f8sw",  A26_ROM_F8_SW);
	device.option_add("a26_fa",    A26_ROM_FA);
	device.option_add("a26_fe",    A26_ROM_FE);
	device.option_add("a26_3e",    A26_ROM_3E);
	device.option_add("a26_3f",    A26_ROM_3F);
	device.option_add("a26_e0",    A26_ROM_E0);
	device.option_add("a26_e7",    A26_ROM_E7);
	device.option_add("a26_ua",    A26_ROM_UA);
	device.option_add("a26_cv",    A26_ROM_CV);
	device.option_add("a26_dc",    A26_ROM_DC);
	device.option_add("a26_fv",    A26_ROM_FV);
	device.option_add("a26_jvp",   A26_ROM_JVP);
	device.option_add("a26_cm",    A26_ROM_COMPUMATE);
	device.option_add("a26_ss",    A26_ROM_SUPERCHARGER);
	device.option_add("a26_dpc",   A26_ROM_DPC);
	device.option_add("a26_4in1",  A26_ROM_4IN1);
	device.option_add("a26_8in1",  A26_ROM_8IN1);
	device.option_add("a26_32in1", A26_ROM_32IN1);
	device.option_add("a26_x07",   A26_ROM_X07);
	device.option_add("a26_harmony",   A26_ROM_HARMONY);
}

void a2600_cons_state::a2600_cartslot(machine_config &config)
{
	VCS_CART_SLOT(config, m_cartslot, a2600_cart, nullptr).set_must_be_loaded(true);
	m_cartslot->set_address_space(m_maincpu, AS_PROGRAM);

	/* software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("a2600");
	SOFTWARE_LIST(config, "cass_list").set_original("a2600_cass");
}

void a2600_base_state::a2600_base_ntsc(machine_config &config)
{
	/* basic machine hardware */
	M6507(config, m_maincpu, m_xtal / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &a2600_base_state::a2600_mem);

	/* video hardware */
	TIA_NTSC_VIDEO(config, m_tia, 0, "tia");
	m_tia->read_input_port_callback().set(FUNC(a2600_state::a2600_read_input_port));
	m_tia->databus_contents_callback().set(FUNC(a2600_state::a2600_get_databus_contents));
	m_tia->vsync_callback().set(FUNC(a2600_state::a2600_tia_vsync_callback));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(m_xtal, 228, 26, 26 + 160 + 16, 262, 24 , 24 + 192 + 31);
	m_screen->set_screen_update("tia_video", FUNC(tia_video_device::screen_update));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	TIA(config, "tia", m_xtal/114).add_route(ALL_OUTPUTS, "mono", 0.90);

	/* devices */
	MOS6532(config, m_riot, m_xtal / 3);
	m_riot->pa_rd_callback().set(FUNC(a2600_state::switch_A_r));
	m_riot->pa_wr_callback().set(FUNC(a2600_state::switch_A_w));
	m_riot->pb_rd_callback().set_ioport("SWB");
	m_riot->pb_wr_callback().set(FUNC(a2600_state::switch_B_w));
	m_riot->irq_wr_callback().set(FUNC(a2600_state::irq_callback));

	VCS_CONTROL_PORT(config, m_joy1, vcs_control_port_devices, "joy");
	VCS_CONTROL_PORT(config, m_joy2, vcs_control_port_devices, "joy");
}


void a2600_base_state::a2600_base_pal(machine_config &config)
{
	/* basic machine hardware */
	M6507(config, m_maincpu, m_xtal / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &a2600_state::a2600_mem);

	/* video hardware */
	TIA_PAL_VIDEO(config, m_tia, 0, "tia");
	m_tia->read_input_port_callback().set(FUNC(a2600_state::a2600_read_input_port));
	m_tia->databus_contents_callback().set(FUNC(a2600_state::a2600_get_databus_contents));
	m_tia->vsync_callback().set(FUNC(a2600_state::a2600_tia_vsync_callback));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(m_xtal, 228, 26, 26 + 160 + 16, 312, 32, 32 + 228 + 31);
	m_screen->set_screen_update("tia_video", FUNC(tia_video_device::screen_update));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	TIA(config, "tia", m_xtal/114).add_route(ALL_OUTPUTS, "mono", 0.90);

	/* devices */
	MOS6532(config, m_riot, m_xtal / 3);
	m_riot->pa_rd_callback().set(FUNC(a2600_state::switch_A_r));
	m_riot->pa_wr_callback().set(FUNC(a2600_state::switch_A_w));
	m_riot->pb_rd_callback().set_ioport("SWB");
	m_riot->pb_wr_callback().set(FUNC(a2600_state::switch_B_w));
	m_riot->irq_wr_callback().set(FUNC(a2600_state::irq_callback));

	VCS_CONTROL_PORT(config, m_joy1, vcs_control_port_devices, "joy");
	VCS_CONTROL_PORT(config, m_joy2, vcs_control_port_devices, "joy");
}


void a2600_state::a2600(machine_config &config)
{
	a2600_base_ntsc(config);
	a2600_cartslot(config);
	subdevice<software_list_device>("cart_list")->set_filter("NTSC");
}

void a2600p_state::a2600p(machine_config &config)
{
	a2600_base_pal(config);
	a2600_cartslot(config);
	subdevice<software_list_device>("cart_list")->set_filter("PAL");
}


void a2600_pop_state::a2600_pop(machine_config &config)
{
	a2600_base_ntsc(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &a2600_pop_state::memory_map);
}


void tvboy_state::tvboy(machine_config &config)
{
	a2600_base_pal(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &tvboy_state::tvboy_mem);
}

void tvboy_state::tvboyn(machine_config &config)
{
	a2600_base_ntsc(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &tvboy_state::tvboy_mem);
}






class a2600_compumate_state : public a2600_base_state
{
public:
	a2600_compumate_state(const machine_config &mconfig, device_type type, const char *tag, XTAL xtal = 3.579545_MHz_XTAL)
		: a2600_base_state(mconfig, type, tag, xtal)
		, m_bank(*this, "bank")
		, m_swb(*this, "SWB")
		, m_keycolumns(*this, "COL%d", 0U)
		, m_controlkeys(*this, "CONTROLKEYS")
		, m_cass(*this, "cassette")
		, m_rom(*this, "maincpu")
	{ }

	void a2600_compumate_common(machine_config &config);
	void a2600_compumate(machine_config &config);
	void a2600p_compumate(machine_config &config);

	u8 inp_r(offs_t offset);
	u8 joy_r(offs_t offset);
	void joy_w(offs_t offset, u8 data);
	u8 mem_r(offs_t offset);
	void mem_w(offs_t offset, u8 data);

protected:
	virtual void machine_start() override ATTR_COLD;
//  virtual void machine_reset() override ATTR_COLD;

	void memory_map(address_map &map) ATTR_COLD;

	uint8_t rom_switch_r(offs_t offset);
	void rom_switch_w(offs_t offset, uint8_t data);

	optional_memory_bank m_bank;
	required_ioport m_swb;
	required_ioport_array<10> m_keycolumns;
	required_ioport m_controlkeys;
	optional_device<cassette_image_device> m_cass;
	required_memory_region m_rom;
	u8 m_ram[0x800] = {0};
	u8 m_rombank = 0;
	u8 m_ram_enable = 1;
	u8 m_write_enable = 1;
	u8 m_inc_keycol = 0;
	u8 m_ddr = 0;
	u8 m_porta = 0;
	u8 m_keycol = 0;
};


class a2600p_compumate_state : public a2600_compumate_state
{
public:
	a2600p_compumate_state(const machine_config &mconfig, device_type type, const char *tag) :
		a2600_compumate_state(mconfig, type, tag, 3.546894_MHz_XTAL)
	{ }

	//void a2600p_compumate(machine_config &config);
};


/*
    Compumate Information from z26 emulator
    (some incorrect information in the z26 source comments, but z26 code is correct)
    (Best information comes from Kevtris Atari 2600)

    there are 4 4K banks selectable at $1000 - $1FFFF
    bankswitching is done though the controller ports
    INPT0: D7 = CTRL key input (0 on startup / 1 = key pressed)
    INPT1: D7 = always HIGH input (tested at startup)
    INPT2: D7 = always HIGH input (tested at startup)
    INPT3: D7 = SHIFT key input (0 on startup / 1 = key pressed)
    INPT4: D7 = keyboard row 1 input (0 = key pressed)
    INPT5: D7 = keyboard row 3 input (0 = key pressed)
    SWCHA: D7 = tape recorder I/O ?
           D6 = 1 -> increase key column (0 to 9)
           D5 = 1 -> reset key column to 0 (if D4 = 0)
           D5 = 0 -> enable RAM writing (if D4 = 1)
           D4 = 1 -> map 2K of RAM at $1800 - $1fff
           D3 = keyboard row 4 input (0 = key pressed)
           D2 = keyboard row 2 input (0 = key pressed)
           D1 = bank select high bit
           D0 = bank select low bit

    bit 6 swa also controls the cassette out bit
    bit 7 swa reads from the cassette


    keyboard column numbering:
               1 2 4 8
    column 0 = 7 U J M
    column 1 = 6 Y H N
    column 2 = 8 I K ,
    column 3 = 2 W S X
    column 4 = 3 E D C
    column 5 = 0 P ENTER SPACE
    column 6 = 9 O L .
    column 7 = 5 T G B
    column 8 = 1 Q A Z
    column 9 = 4 R F V
*/


void a2600_compumate_state::joy_w(offs_t offset, u8 data)
{
	if (offset==0)
	{
		u8 last = m_porta;
		m_porta = data;
		m_rombank      = BIT(data, 0, 2);
		m_ram_enable   = BIT(data, 4);
		m_write_enable = BIT(data, 5);
		m_inc_keycol   = BIT(data, 6);

		m_cass->output(BIT(data, 6) ? -1.0 : 1.0);

		if (m_ram_enable && BIT(data, 5))
		{
			m_keycol = 0;
		}
		if (BIT(data,6) == 1 && BIT(last,6) == 0)
		{
			m_keycol = (m_keycol + 1) % 10;
		}

//      printf("joywrite: %x = %x    bank=%x  ramenable=%x  writeenable=%x inckeycol=%x   m_ddr=%x  keycol=%x  bit7=%x\n",offset,data,m_rombank,m_ram_enable,m_write_enable,BIT(data,6),m_ddr,m_keycol,BIT(data,7));
	}
	else if (offset==1) m_ddr = data;
}

u8 a2600_compumate_state::joy_r(offs_t offset)
{
	if (offset == 0)
		return (m_porta & 0x73) |
			((m_keycolumns[m_keycol]->read() & 0x2) != 0) << 2 |  // row 1
			((m_keycolumns[m_keycol]->read() & 0x8) != 0) << 3 |  // row 3
			((m_cass->input() > 0.01 ? 1 : 0) << 7);
	else if (offset == 1)
		return m_ddr;
	else
		return 0;
}

void a2600_compumate_state::mem_w(offs_t offset, u8 data)
{
//  printf("memwrite: %x = %x\n",offset,data);
	if (!m_ram_enable && m_write_enable && offset >= 0x800)
		m_ram[offset - 0x800] = data;
}

//  u8 *m_ram_ptr, *m_rom_ptr, *m_cec_ptr;
//m_rom_ptr = m_rom->base();

u8 a2600_compumate_state::mem_r(offs_t offset)
{
	if (offset < 0x800) return *(m_rom->base() + m_rombank * 0x1000 + offset);
	else
	{   if (!m_ram_enable) return m_ram[offset - 0x800];
		else return *(m_rom->base() + m_rombank * 0x1000 + offset);
	}
}

u8 a2600_compumate_state::inp_r(offs_t offset)
{
	switch(offset)
	{
		case 0: return (BIT(m_controlkeys->read(),0) << 7);
		case 1: return 0x80; break;  // not 1 it's 80
		case 2: return 0x80; break;  // or it writes 0 to bank JSR
		case 3: return (BIT(m_controlkeys->read(),1) << 7);
		case 4: return ((m_keycolumns[m_keycol]->read() & 0x1) != 0) << 7;  // row 0
		case 5: return ((m_keycolumns[m_keycol]->read() & 0x4) != 0) << 7;  // row 2
		default: return 0;
	}
}

/*
0038r   INPT0   X....... Paddle input 0
0039r   INPT1   X....... Paddle input 1
003Ar   INPT2   X....... Paddle input 2
003Br   INPT3   X....... Paddle input 3
003Cr   INPT4   X....... Latched inputs (joystick buttons)
003Dr   INPT5   X....... Latched inputs (joystick buttons)
*/

void a2600_compumate_state::memory_map(address_map &map) // 6507 has 13-bit address space, 0x0000 - 0x1fff
{
	map(0x0000, 0x007f).mirror(0x0f00).rw(m_tia, FUNC(tia_video_device::read), FUNC(tia_video_device::write));
	map(0x0080, 0x00ff).mirror(0x0d00).ram().share("riot_ram");
	map(0x0280, 0x029f).mirror(0x0d00).m("riot", FUNC(mos6532_device::io_map));
	map(0x0280, 0x0281).mirror(0x0d00).rw(FUNC(a2600_compumate_state::joy_r),FUNC(a2600_compumate_state::joy_w));
	map(0x0038, 0x003d).mirror(0x0d00).r(FUNC(a2600_compumate_state::inp_r));
	map(0x1000, 0x1fff).rw(FUNC(a2600_compumate_state::mem_r),FUNC(a2600_compumate_state::mem_w));
}

void a2600_compumate_state::machine_start()
{
	a2600_base_state::machine_start();

	m_rombank = 3;
	m_ram_enable = 1;
	m_write_enable = 1;
	m_inc_keycol = 0;
	m_ddr = 0;
	m_porta = 0;
	m_keycol = 0;

	save_pointer(NAME(m_ram), 0x800);

	save_item(NAME(m_rombank));
	save_item(NAME(m_ram_enable));
	save_item(NAME(m_inc_keycol));
	save_item(NAME(m_ddr));
	save_item(NAME(m_porta));
	save_item(NAME(m_keycol));
//
}


void a2600_compumate_state::a2600_compumate_common(machine_config &config)
{
	m_maincpu->set_addrmap(AS_PROGRAM, &a2600_compumate_state::memory_map);

	CASSETTE(config, m_cass);
	m_cass->set_formats(compumate_cassette_formats);
	m_cass->set_default_state(CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.01);
	m_cass->set_interface("compumate_cass");
}

void a2600_compumate_state::a2600_compumate(machine_config &config)
{
	a2600_base_ntsc(config);
	a2600_compumate_common(config);
}

void a2600_compumate_state::a2600p_compumate(machine_config &config)
{
	a2600_base_pal(config);
	a2600_compumate_common(config);
}




static INPUT_PORTS_START( a2600_compumate )

	PORT_START("CONTROLKEYS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("FUNC") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT)


/*
    keyboard column numbering:
    column 0 = 7 U J M
    column 1 = 6 Y H N
    column 2 = 8 I K ,
    column 3 = 2 W S X
    column 4 = 3 E D C
    column 5 = 0 P ENTER SPACE
    column 6 = 9 O L .
    column 7 = 5 T G B
    column 8 = 1 Q A Z
    column 9 = 4 R F V


a=[[7 U J M
6 Y H N
8 I K ,
2 W S X
3 E D C
0 P ENTER SPACE
9 O L .
5 T G B
1 Q A Z
4 R F V]]

count=0 for w in a:gmatch("[^\n]+") do b = w print("PORT_START(\"".."COL"..count.."\")") count=count+1 ibit=1 for c in b:gmatch("%S+") do print("PORT_BIT("..string.format("0x%x",ibit)..", IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_"..c..") PORT_CHAR('"..c.."')") ibit=ibit*2 end print() end


*/
	PORT_START("COL0")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_M) PORT_CHAR('M')

	PORT_START("COL1")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_N) PORT_CHAR('N')

	PORT_START("COL2")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',')

	PORT_START("COL3")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_X) PORT_CHAR('X')

	PORT_START("COL4")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_C) PORT_CHAR('C')

	PORT_START("COL5")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_ENTER) //PORT_CHAR('ENTER')
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_SPACE) //PORT_CHAR('SPACE')

	PORT_START("COL6")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_STOP) PORT_CHAR('.')

	PORT_START("COL7")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_B) PORT_CHAR('B')

	PORT_START("COL8")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')

	PORT_START("COL9")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_V) PORT_CHAR('V')

	PORT_START("SWB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Reset Game") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Select Game") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_CONFNAME( 0x08, 0x08, "TV Type" ) PORT_CODE(KEYCODE_C) PORT_TOGGLE
	PORT_CONFSETTING(    0x08, "Color" )
	PORT_CONFSETTING(    0x00, "B&W" )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_CONFNAME( 0x40, 0x00, "Left Diff. Switch" ) PORT_CODE(KEYCODE_3) PORT_TOGGLE
	PORT_CONFSETTING(    0x40, "A" )
	PORT_CONFSETTING(    0x00, "B" )
	PORT_CONFNAME( 0x80, 0x00, "Right Diff. Switch" ) PORT_CODE(KEYCODE_4) PORT_TOGGLE
	PORT_CONFSETTING(    0x80, "A" )
	PORT_CONFSETTING(    0x00, "B" )
INPUT_PORTS_END





ROM_START(a2600_compumate)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	//<rom name="compumate (1983) (spectravideo) (sv-010) ~.bin" size="16384" crc="bbe1f661" sha1="1947b26419fadfc6b2c9b5d552fd3d2bd220bc61"/>
	ROM_LOAD("compumate.bin", 0x0000, 0x4000, CRC(bbe1f661) SHA1(1947b26419fadfc6b2c9b5d552fd3d2bd220bc61))
ROM_END

ROM_START(a2600p_compumate)
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASEFF)
	//<rom name="compumate (1983) (spectravideo - universum) (sv-010) (pal).bin" size="16384" crc="bed37e4f" sha1="d85daed1b5397a779e8832628979f5396650090d"/>
	ROM_LOAD("compumatep.bin", 0x0000, 0x4000, CRC(bed37e4f) SHA1(d85daed1b5397a779e8832628979f5396650090d))
ROM_END





ROM_START(a2600)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)
ROM_END

#define rom_a2600p rom_a2600

ROM_START(a2600_pop)
	ROM_REGION(0x30000, "maincpu", 0)
	// Boot/Game selection
	ROM_LOAD("136003_101.c6", 0x000, 0x800, CRC(9d3cfba6) SHA1(b84c34aaedd84fca30b6e8223ee062439acf4fe0))
	ROM_COPY("maincpu", 0x000, 0x800, 0x800)
	// Games
	// Hashes taken from the software list. These all need to be verified.
	// Judging by ROM labels from the parts list there is the possibility
	// that AIR-SEA BATTLE, ASTEROIDS, DODGE'EM, and SUPERMAN may have been
	// modified for the point of purchase unit.
	ROM_LOAD("c012013.d6", 0x1000, 0x1000, BAD_DUMP CRC(a6db4b3a) SHA1(e07e48d463d30321239a8acc00c490f27f1f7422)) // Prog ROM 1 ADVENTURE (CX2613)
	ROM_LOAD("c011202a.e6", 0x2000, 0x800, BAD_DUMP CRC(1fc6c0f1) SHA1(a746fdc82b336a9d499bf17f50b41e0193ba595e)) // Prog ROM 2 AIR-SEA BATTLE (CX2602)
	ROM_COPY("maincpu", 0x2000, 0x2800, 0x800)
	// Original game is 8KB, special 4KB version or is there banking hardware for this game?
	ROM_LOAD("c016449a.f6", 0x3000, 0x1000, NO_DUMP) // Prog ROM 3 ASTEROIDS (CX2649)
	ROM_LOAD("c012017.j6", 0x4000, 0x1000, BAD_DUMP CRC(e2a7ffa6) SHA1(9b1da7fbd0bf6fcadf1b60c11eeb31b6a61a03c3)) // Prog ROM 4 BACKGAMMON (CX2617)
	ROM_LOAD("c011224.k6", 0x5000, 0x800, BAD_DUMP CRC(711a2340) SHA1(9375c5c5298e81b37224dbde9bc5af151181ca27)) // Prog ROM 5 BASKETBALL (CX2624)
	ROM_COPY("maincpu", 0x5000, 0x5800, 0x800)
	ROM_LOAD("c011228.l6", 0x6000, 0x800, BAD_DUMP CRC(fea0d6d5) SHA1(cf6ce244b3edaad7ad5e9ca5f01668135c2f93d0)) // Prog ROM 6 BOWLING (CX2628)
	ROM_COPY("maincpu", 0x6000, 0x6800, 0x800)
	ROM_LOAD("c011222.m6", 0x7000, 0x800, BAD_DUMP CRC(3037638c) SHA1(8d473b87b70e26890268e6c417c0bb7f01e402eb)) // Prog ROM 7 BREAKOUT (CX2622)
	ROM_COPY("maincpu", 0x7000, 0x7800, 0x800)
	ROM_LOAD("c011207.c5", 0x8000, 0x800, BAD_DUMP CRC(e914b8ca) SHA1(b89443a0029e765c2716774fe2582be37650115c)) // Prog ROM 8 CANYON BOMBER (CX2607)
	ROM_COPY("maincpu", 0x8000, 0x8800, 0x800)
	ROM_LOAD("c012052.d5", 0x9000, 0x1000, BAD_DUMP CRC(420b8248) SHA1(08598101e38756916613f37581ef1b61c719016f)) // Prog ROM 9 CASINO (CX2652)
	ROM_LOAD("c012030.e5", 0xa000, 0x1000, BAD_DUMP CRC(a4b9a830) SHA1(6821c334d9cb85da17fa2a960636620966b91f96)) // Prog ROM 10 CIRCUS ATARI (CX2630)
	ROM_LOAD("c011201.f5", 0xb000, 0x800, BAD_DUMP CRC(9c326a97) SHA1(ce7580059e8b41cb4a1e734c9b35ce3774bf777a)) // Prog ROM 11 COMBAT (CX2601)
	ROM_COPY("maincpu", 0xb000, 0xb800, 0x800)
	ROM_LOAD("c012037a.j5", 0xc000, 0x1000, BAD_DUMP CRC(bc3602b5) SHA1(0ffc02c54190e9dd51ac1e52c911d3d7d730a40a)) // Prog ROM 12 DODGE'EM (CX2637)
	ROM_LOAD("c011225.k5", 0xd000, 0x800, BAD_DUMP CRC(3b73ee02) SHA1(c6fe4ce24bc1ebd538258d98cfe829963323acca)) // Prog ROM 13 FOOTBALL (CX2625)
	ROM_COPY("maincpu", 0xd000, 0xd800, 0x800)
	ROM_LOAD("c011234.l5", 0xe000, 0x800, BAD_DUMP CRC(46a9f200) SHA1(a25d52770408314dec6f41aaf5f9f0a2a3e2c18f)) // Prog ROM 14 GOLF (CX2634)
	ROM_COPY("maincpu", 0xe000, 0xe800, 0x800)
	ROM_LOAD("c012062.m5", 0xf000, 0x1000, BAD_DUMP CRC(c2bcc789) SHA1(561bccf508e162bc70c42d85c170cf0d1d4691a3)) // Prog ROM 15 HANGMAN (CX2662)
	ROM_LOAD("c011223.c4", 0x10000, 0x800, BAD_DUMP CRC(45ace998) SHA1(f362d2b3a50e5ae3c2b412b6c08ecdcfee47a688)) // Prog ROM 16 HOMERUN (CX2623)
	ROM_COPY("maincpu", 0x10000, 0x10800, 0x800)
	ROM_LOAD("c011227.d4", 0x11000, 0x800, BAD_DUMP CRC(f05a41e1) SHA1(d4b0b2aa379893356c72414ee0065a3a91cf9f97)) // Prog ROM 17 H. CANNONBALL (CX2627)
	ROM_COPY("maincpu", 0x11000, 0x11800, 0x800)
	ROM_LOAD("c012035.e4", 0x12000, 0x1000, BAD_DUMP CRC(0098e428) SHA1(aba25089d87cd6fee8d206b880baa5d938aae255)) // Prog ROM 18 MAZE CRAZE (CX2635)
	ROM_LOAD("c012038.f4", 0x13000, 0x1000, BAD_DUMP CRC(cff14904) SHA1(faa06bb0643dbf556b13591c31917d277a83110b)) // Prog ROM 19 MISSILE CMD (CX2638)
	ROM_LOAD("c011233.j4", 0x14000, 0x800, BAD_DUMP CRC(600d8a96) SHA1(372771aeb4e2fb2cd1dead5497e3821e4236d5fc)) // Prog ROM 20 NIGHT DRIVER (CX2633)
	ROM_COPY("maincpu", 0x14000, 0x14800, 0x800)
	ROM_LOAD("c011239.k4", 0x15000, 0x800, BAD_DUMP CRC(171ae72f) SHA1(cbecf1a32d9366a3dd4ad643916cd59cdc820a8b)) // Prog ROM 21 OTHELLO (CX2639)
	ROM_COPY("maincpu", 0x15000, 0x15800, 0x800)
	ROM_LOAD("c011205.l4", 0x16000, 0x800, BAD_DUMP CRC(68dd7acd) SHA1(f8eeaaf4635ac39b4bdf7ded1348bce46313ef9f)) // Prog ROM 22 OUTLAW (CX2605)
	ROM_COPY("maincpu", 0x16000, 0x16800, 0x800)
	ROM_LOAD("c012016.m4", 0x17000, 0x1000, BAD_DUMP CRC(3f59bb60) SHA1(832283530f5dee332f29cf8c4854dd554f2030a0)) // Prog ROM 23 SOCCER (CX2616)
	ROM_LOAD("c011229.c3", 0x18000, 0x800, BAD_DUMP CRC(00312ea9) SHA1(4dde18d4abc139562fdd7a9d2fd49a1f00a9e64a)) // Prog ROM 24 SKYDIVER (CX2629)
	ROM_COPY("maincpu", 0x18000, 0x18800, 0x800)
	ROM_LOAD("c011206.d3", 0x19000, 0x800, BAD_DUMP CRC(177dbdf8) SHA1(a2b13017d759346174e3d8dd53b6347222d3b85d)) // Prog ROM 25 SLOT RACERS (CX2606)
	ROM_COPY("maincpu", 0x19000, 0x19800, 0x800)
	ROM_LOAD("c012032.e3", 0x1a000, 0x1000, BAD_DUMP CRC(a6e867b3) SHA1(31d9668fe5812c3d2e076987ca327ac6b2e280bf)) // Prog ROM 26 SPACE INVADERS (CX2632)
	ROM_LOAD("c011212.f3", 0x1b000, 0x800, BAD_DUMP CRC(47592880) SHA1(bffb3d41916c83398624151eb00aa2a3acd23ab8)) // Prog ROM 27 STREET RACER (CX2612)
	ROM_COPY("maincpu", 0x1b000, 0x1b800, 0x800)
	ROM_LOAD("c012031b.j3", 0x1c000, 0x1000, BAD_DUMP CRC(39562bd7) SHA1(b9dee027c8d7dd2a46be111ab0b8363c1becc081)) // Prog ROM 28 SUPERMAN (CX2631)
	ROM_LOAD("c011218.k3", 0x1d000, 0x800, BAD_DUMP CRC(58805709) SHA1(21d983f2f52b84c22ecae84b0943678ae2c31c10)) // Prog ROM 29 3D TIC-TAC-TOE (CX2618)
	ROM_COPY("maincpu", 0x1d000, 0x1d800, 0x800)
	ROM_LOAD("c012036.l3", 0x1e000, 0x1000, BAD_DUMP CRC(3df33335) SHA1(babae88a832b76d8c5af6ea63b8f10a0da5bb992)) // Prog ROM 30 VIDEO CHECKERS (CX2636)
	ROM_LOAD("c012045.m3", 0x1f000, 0x1000, BAD_DUMP CRC(b6226a54) SHA1(043ef523e4fcb9fc2fc2fda21f15671bf8620fc3)) // Prog ROM 31 VIDEO CHESS (CX2645)
	ROM_LOAD("c011221.c2", 0x20000, 0x800, BAD_DUMP CRC(e4bc89c4) SHA1(1ffe89d79d55adabc0916b95cc37e18619ef7830)) // Prog ROM 32 VIDEO OLYMPICS (CX2621)
	ROM_COPY("maincpu", 0x20000, 0x20800, 0x800)
	ROM_LOAD("c012048.d2", 0x21000, 0x1000, BAD_DUMP CRC(10d95426) SHA1(2c16c1a6374c8e22275d152d93dd31ffba26271f)) // Prog ROM 33 VIDEO PINBALL (CX2648)
	ROM_LOAD("c012010.e2", 0x22000, 0x1000, BAD_DUMP CRC(cf174b57) SHA1(2d7563d337cbc0cdf4fc14f69853ab6757697788)) // Prog ROM 34 WAR LORDS (CX2610)
	ROM_LOAD("c012050.f2", 0x23000, 0x1000, BAD_DUMP CRC(2e8b4b5f) SHA1(08bcbc8954473e8f0242b881315b0af4466998ae)) // Prog ROM 35 BERZERK (CX2650)
	ROM_LOAD("c012054.j2", 0x24000, 0x1000, BAD_DUMP CRC(aa62d961) SHA1(1476c869619075b551b20f2c7f95b11e0d16aec1)) // Prog ROM 36 HAUNTED HOUSE (CX2654)
	ROM_LOAD("c012058.k2", 0x25000, 0x1000, BAD_DUMP CRC(ccc90c98) SHA1(18fac606400c08a0469aebd9b071ae3aec2a3cf2)) // Prog ROM 37 MATH GRAND PRIX (CX2658)
	ROM_LOAD("c012009.l2", 0x26000, 0x1000, BAD_DUMP CRC(0df43d8e) SHA1(79facc1bf70e642685057999f5c2b8e94b102439)) // Prog ROM 38 DEFENDER (CX2609)
	ROM_LOAD("c012055.m2", 0x27000, 0x1000, BAD_DUMP CRC(dfa1c825) SHA1(e2cd8996c1cf929e29130690024d1ec23d3b0bde)) // Prog ROM 39 YARS' REVENGE (CX2655)
	ROM_LOAD("c012046.c1", 0x28000, 0x1000, BAD_DUMP CRC(ddc9a881) SHA1(0940fea7f04cdb6d4b90c5ad1a7e344e68f6dbb1)) // Prog ROM 40 PAC-MAN (CX2646)
	ROM_LOAD("c012008.d1", 0x29000, 0x1000, BAD_DUMP CRC(b0f20d31) SHA1(ac2aad2196c155c1d87d6f42fa88891825f4fde6)) // Prog ROM 41 SUPER BREAKOUT (CX2608)
	ROM_LOAD("c012015.e1", 0x2a000, 0x1000, BAD_DUMP CRC(9b97c3da) SHA1(b45582de81c48b04c2bb758d69021e8088c70ce7)) // Prog ROM 42 DEMON/DIAMOND (CX2615)
	// empty slots f1, j1, k1, l1, m1 ?
	// Sears Video Game Selection Center adds 3 games in f1, j1, and k1
	ROM_LOAD("subcommander.f1", 0x2b000, 0x1000, BAD_DUMP CRC(3262960a) SHA1(b22ba7cbde60a21ecbbe3953cc4a5c0bf007cc26)) // Prog ROM 43
	ROM_LOAD("stellartrack.j1", 0x2c000, 0x1000, BAD_DUMP CRC(a76e93bf) SHA1(7d97d014c22a2ed3a5bc4b310f5a7be1b1d3520f)) // Prog ROM 44
	ROM_LOAD("steeplechase.k1", 0x2d000, 0x800,  BAD_DUMP CRC(5f6c32c3) SHA1(e56ef1c0313d6d04e25446c4e34f9bb7eda8efac)) // Prog ROM 45
	ROM_COPY("maincpu", 0x2d000, 0x2d800, 0x800)
ROM_END

ROM_START(tvboy)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)

	ROM_REGION(0x80000, "mainrom", 0)
	ROM_LOAD("tvboy.bin", 0x00000, 0x80000, CRC(2f3d1d52) SHA1(fb26778434fade4cec28f82c53db4cc2f23b8b2b))
ROM_END

ROM_START(tvboyn)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)

	ROM_REGION(0x80000, "mainrom", 0)
	ROM_LOAD("ns-31_n_tv-bot_127g-nics.59874.bin", 0x00000, 0x80000, CRC(96744687) SHA1(47e7a01e635156d2dd6c7e1059653f286370537d))
ROM_END

ROM_START(tvboyii)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)

	ROM_REGION(0x80000, "mainrom", 0)
	// Games 57, 64, and 119 differ from tvboy with crc 2f3d1d52.
	/* Game 29, Enduro, differs only in one LDA instruction at 0x179 (and then some shifted code).
	   Here the LDA uses zero page while the original ROM and all other known versions use absolute addressing.
	   Does this cause a timing issue? It appears bugged in MAME. Does it work on hardware? */
	ROM_LOAD("hy23400p.bin", 0x00000, 0x80000, CRC(f8485173) SHA1(cafbaa0c5437f192cb4fb49f9a672846aa038870))
ROM_END

ROM_START( stvboy )
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)

	ROM_REGION(0x80000, "mainrom", 0)
	// Only game 91 differs from tvboyii with crc f8485173. Otherwise the dumps are identical. Game 29, Enduro, has the same issue mentioned above.
	ROM_LOAD("supertvboy.bin", 0x00000, 0x80000, CRC(af2e73e8) SHA1(04b9ddc3b30b0e5b81b9f868d455e902a0151491))
ROM_END

} // anonymous namespace

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS         INIT        COMPANY     FULLNAME
CONS( 1977, a2600,  0,      0,      a2600,   a2600, a2600_state,  empty_init, "Atari",    "Atari 2600 (NTSC)",  MACHINE_SUPPORTS_SAVE )
CONS( 1978, a2600p, a2600,  0,      a2600p,  a2600, a2600p_state, empty_init, "Atari",    "Atari 2600 (PAL)",   MACHINE_SUPPORTS_SAVE )

// Released in 1981/1982
// Games 35-42 are copyright 1982 and looking at the game list they seem to be
// added later.
GAME( 198?, a2600_pop, 0,      a2600_pop, a2600_pop, a2600_pop_state, empty_init, ROT0, "Atari",    "Atari 2600 Point of Purchase Display",   MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )

// Clones
CONS( 199?, tvboy,   0,     0,      tvboy,   tvboy, tvboy_state,  empty_init, "Systema?", "TV Boy (PAL)",       MACHINE_SUPPORTS_SAVE ) // It's unknown what unit this came from. It could be Akor instead?
CONS( 199?, tvboyn,  tvboy, 0,      tvboyn,  tvboy, tvboy_state,  empty_init, "Nics",     "TV Boy (Nics, NTSC)",MACHINE_SUPPORTS_SAVE )
CONS( 199?, tvboyii, tvboy, 0,      tvboy,   tvboy, tvboy_state,  empty_init, "Systema",  "TV Boy II (PAL)",    MACHINE_SUPPORTS_SAVE )
CONS( 1995, stvboy,  0,     0,      tvboy,   tvboy, tvboy_state,  empty_init, "Akor",     "Super TV Boy (PAL)", MACHINE_SUPPORTS_SAVE )



//GAME(YEAR, NAME, PARENT, MACHINE, INPUT, CLASS, INIT, MONITOR, COMPANY, FULLNAME, FLAGS)

GAME( 1983, a2600_compumate, 0,                 a2600_compumate, a2600_compumate, a2600_compumate_state,   empty_init, ROT0, "SpectraVideo", "Atari 2600 Compumate NTSC", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1983, a2600p_compumate, a2600_compumate,  a2600p_compumate, a2600_compumate, a2600p_compumate_state, empty_init, ROT0, "SpectraVideo", "Atari 2600 Compumate PAL",  MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
