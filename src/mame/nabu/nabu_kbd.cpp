// license:BSD-3-Clause
// copyright-holders:Katherine Rohl

#include "emu.h"
#include "nabu_kbd.h"
#include "bus/vcs_ctrl/ctrl.h"

#define VERBOSE 1
#include "logmacro.h"

namespace {

ROM_START(nabu_kbd)
	ROM_REGION(0x800, "mbcpu", 0)
	ROM_LOAD("keyboard_rev_a.bin", 0x0000, 0x0800, CRC(eead3abc) SHA1(2f6ff63ca2f2ac90f3e03ef4f2b79883205e8a4e))
ROM_END

class nabu_pc_keyboard_device : public device_t, public device_rs232_port_interface
{
public:
	nabu_pc_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_WRITE_LINE_MEMBER( input_txd ) override;

	required_device<m6803_cpu_device> m_cpu;
	required_ioport_array<8> m_rows;
	required_ioport m_modkeys;
	required_device<vcs_control_port_device> m_joy1;
	required_device<vcs_control_port_device> m_joy2;


protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;


    TIMER_CALLBACK_MEMBER(watchdog_expired);

private:
    emu_timer *m_watchdog_timer;
	void ser_tx_w(int state) { output_rxd(state); }


    void watchdog_reset_w(u8 state) { reset_timer(); }

	uint8_t p1_data_r();
	void p1_data_w(uint8_t data);

	int m_p1_data = 0;

	void m6803_mem(address_map &map);
	int m_rx_state;

    void reset_timer();

	uint8_t joyport1_r();
	uint8_t joyport2_r();
	uint8_t joyport3_r();
	uint8_t joyport4_r();
};

nabu_pc_keyboard_device::nabu_pc_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NABU_PC_KEYBOARD, tag, owner, clock)
	, device_rs232_port_interface(mconfig, *this)
	, m_cpu(*this, "mbdcpu")
	, m_rows(*this, "X%u", 0U)
	, m_modkeys(*this, "MODKEYS")
	, m_joy1(*this, "joyport1")
	, m_joy2(*this, "joyport2")
{
}

void nabu_pc_keyboard_device::device_add_mconfig(machine_config &config)
{
	M6803(config, m_cpu, XTAL(3'579'545));
	m_cpu->set_addrmap(AS_PROGRAM, &nabu_pc_keyboard_device::m6803_mem);
	m_cpu->out_ser_tx_cb().set(FUNC(nabu_pc_keyboard_device::ser_tx_w));
	m_cpu->in_p1_cb().set(FUNC(nabu_pc_keyboard_device::p1_data_r));
	m_cpu->out_p1_cb().set(FUNC(nabu_pc_keyboard_device::p1_data_w));

	VCS_CONTROL_PORT(config, m_joy1, vcs_control_port_devices, "joy");
	VCS_CONTROL_PORT(config, m_joy2, vcs_control_port_devices, "joy");

}

uint8_t nabu_pc_keyboard_device::p1_data_r()
{
// debugger command to see writes to $13 to send data
// wp 13:rs232:nabu_kbd:mbdcpu,1,w,1,{printf "write (%x) = %x",wpaddr,wpdata;g}

	int col = BIT(m_p1_data,0,3);
	int row = BIT(m_p1_data,3,3);

	if (BIT(m_p1_data,6)) // 0x40, 0x41, 0x42
	{
		return BIT(m_modkeys->read(),col) << 7;  // return in the high bit
	}
	else
	{
		return BIT(m_rows[row]->read(),col) << 7;
	}
}

void nabu_pc_keyboard_device::p1_data_w(uint8_t data)
{
	// sets the keyboard matrix drivers, bits 2..0 select col, bits 5..3 select row
	// bit 6 selects special keys like ctrl, shift and caps lock
	m_p1_data = data;
}

static INPUT_PORTS_START( nabu_kbd )

	// ROW 0 = 00, ESC,  E0 (->),  P, W, R,  I,  ]
	// ROW 1 = U,    Q,  E1 (<-),  [, E, T,  O,  DEL
	// ROW 2 = 8,    2,  E2 (UP),  =, 4, 6,  0,  EA (TV/NABU)
	// ROW 3 = 7,    1,  E7 (YES), -, 3, 5,  9,  E9 (PAUSE)
	// ROW 4 = G,  TAB,  E6 (NO),  L, A, D,  J,  '
	// ROW 5 = SPC,  X,  E5 (<|),  /, V, 00, ,,  E8 (SYM)
	// ROW 6 = N,    C,  E4 (|>),  ., B, Z,  M,  00
	// ROW 7 = H,    Y,  E3 (DN),  ;, S, F,  K,  0D (GO/ENTER)


	// ROW 0 =  ESC,  E0, P, W, R, I, ]
	PORT_START("X0")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc S-Break") PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)  PORT_CHAR(0xe0)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)  PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)  PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)  PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)  PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')

	// ROW 1 = U, Q,  E1, [, E, T, O, DEL
	PORT_START("X1")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)  PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)  PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)  PORT_CHAR(0xe1)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)  PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)  PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)  PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Delete")   PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)

	// ROW 2 = 8, 2,  E2, =, 4, 6, 0, EA
	PORT_START("X2")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)  PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)  PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)  PORT_CHAR(0xe2)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Equals S-Restart") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)  PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)  PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)  PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("TV/Nabu S-Help")  PORT_CODE(KEYCODE_F3) PORT_CHAR(0xea)

	// ROW 3 = 7, 1,  E7, -, 3, 5, 9, E9
	PORT_START("X3")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("YES") PORT_CODE(KEYCODE_F1)  PORT_CHAR(0xe7)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)  PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Pause S-Exit")   PORT_CODE(KEYCODE_PAUSE)  PORT_CHAR(0xe9)

	// ROW 4 = G,TAB, E6, L, A, D, J, '
	PORT_START("X4")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("NO") PORT_CODE(KEYCODE_F2)  PORT_CHAR(0xe6)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('\"')

	// ROW 5 = SPC,X, E5, /, V, 00, ',', E8
	PORT_START("X5")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)  PORT_CHAR(' ')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)  PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("<|||")  PORT_CODE(KEYCODE_F11)  PORT_CHAR(0xe5)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)  PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)  PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)  PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SYM")  PORT_CODE(KEYCODE_F4)  PORT_CHAR(0xe8)

    // ROW 6 = N, C,  E4, ., B, Z, M, 00
	PORT_START("X6")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)  PORT_CHAR('N') PORT_CHAR('^')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)  PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("|||>") PORT_CODE(KEYCODE_F12)  PORT_CHAR(0xe4)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)   PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)  PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)  PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)  PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_UNUSED)

    // ROW 7 = H, Y,  E3, ;, S, F, K, 0D
	PORT_START("X7")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)  PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)  PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)  PORT_CHAR(0xe3)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)  PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)  PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)  PORT_CHAR('K') PORT_CHAR('k')
    PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Go")   PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)

    // MODIFIER KEYS
	PORT_START("MODKEYS")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)   PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("Control")   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)   PORT_CODE(KEYCODE_LSHIFT)   PORT_CODE(KEYCODE_RSHIFT)   PORT_NAME("Shift")     PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)   PORT_CODE(KEYCODE_CAPSLOCK)                             PORT_NAME("Caps Lock") PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
INPUT_PORTS_END


ioport_constructor nabu_pc_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(nabu_kbd);
}

const tiny_rom_entry *nabu_pc_keyboard_device::device_rom_region() const
{
	return ROM_NAME(nabu_kbd);
}

void nabu_pc_keyboard_device::device_start()
{
	m_watchdog_timer = timer_alloc(FUNC(nabu_pc_keyboard_device::watchdog_expired), this);
}

TIMER_CALLBACK_MEMBER(nabu_pc_keyboard_device::watchdog_expired)
{
    LOG("keyboard watchdog reset\n");
	m_cpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
//	m_cpu->pulse_input_line(INPUT_LINE_NMI, attotime::from_msec(10));
}

void nabu_pc_keyboard_device::device_reset()
{
	output_dcd(0);
	output_dsr(0);
	output_cts(0);

    reset_timer();
}

void nabu_pc_keyboard_device::reset_timer()
{
	m_watchdog_timer->adjust( attotime::from_seconds( 5 ) );
//	m_watchdog_timer->adjust( attotime::from_msec( 5 ) );
}

uint8_t nabu_pc_keyboard_device::joyport1_r()
{
	//int j = m_joy1->read_joy();
	//printf("BUTTON = %x\n",BIT(j,5));
	return bitswap<8>(m_joy1->read_joy(),5,6,6,6,0,3,1,2);  // fire button comes in as the high bit 6 = don't care bits
}
uint8_t nabu_pc_keyboard_device::joyport2_r()
{
	return bitswap<8>(m_joy2->read_joy(),5,6,6,6,0,3,1,2);  // fire button comes in as the high bit 6 = don't care bits
}
uint8_t nabu_pc_keyboard_device::joyport3_r()
{
	return 0xff;
}
uint8_t nabu_pc_keyboard_device::joyport4_r()
{
	return 0xff;
}

void nabu_pc_keyboard_device::m6803_mem(address_map &map)
{
	// TODO: this is definitely wrong

    // Somewhere there's a 555 watchdog timer.
    // The MCU writes to $7000, $8000, $9000. It's one of those, presumably.

	// $50/$51/$52/$5300 - 4 joystick ports? 2 are populated, 2 are unpopulated.
	map(0x5000, 0x5000).r(FUNC(nabu_pc_keyboard_device::joyport1_r));
	map(0x5100, 0x5100).r(FUNC(nabu_pc_keyboard_device::joyport2_r));
	map(0x5200, 0x5200).r(FUNC(nabu_pc_keyboard_device::joyport3_r));
	map(0x5300, 0x5300).r(FUNC(nabu_pc_keyboard_device::joyport4_r));

    map(0x7000, 0x7000).w(FUNC(nabu_pc_keyboard_device::watchdog_reset_w));
	map(0xf800, 0xffff).rom().region("mbcpu", 0);
}

WRITE_LINE_MEMBER(nabu_pc_keyboard_device::input_txd)
{
	m_rx_state = (state & 1);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(NABU_PC_KEYBOARD, device_rs232_port_interface, nabu_pc_keyboard_device, "nabu_kbd", "NABU PC Keyboard")
