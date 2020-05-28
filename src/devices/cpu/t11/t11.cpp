// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*** t11: Portable DEC T-11 emulator ******************************************

    System dependencies:    long must be at least 32 bits
                            word must be 16 bit unsigned int
                            byte must be 8 bit unsigned int
                            long must be more than 16 bits
                            arrays up to 65536 bytes must be supported
                            machine must be twos complement

*****************************************************************************/

#include "emu.h"
#include "t11.h"
#include "t11dasm.h"
#include "debugger.h"


/*************************************
 *
 *  Macro shortcuts
 *
 *************************************/

/* registers of various sizes */
#define REGD(x) m_reg[x].d
#define REGW(x) m_reg[x].w.l
#define REGB(x) m_reg[x].b.l

/* PC, SP, and PSW definitions */
#define SP      REGW(6)
#define PC      REGW(7)
#define SPD     REGD(6)
#define PCD     REGD(7)
#define PSW     m_psw.b.l


DEFINE_DEVICE_TYPE(T11,      t11_device,      "t11",      "DEC T11")
DEFINE_DEVICE_TYPE(K1801VM2, k1801vm2_device, "k1801vm2", "K1801VM2")


k1801vm2_device::k1801vm2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: t11_device(mconfig, K1801VM2, tag, owner, clock)
{
}

t11_device::t11_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 16, 16, 0)
	, c_initial_mode(0)
	, m_cp_state(0)
	, m_vec_active(false)
	, m_pf_active(false)
	, m_hlt_active(false)
	, m_out_reset_func(*this)
	, m_in_iack_func(*this)
{
	m_program_config.m_is_octal = true;
	for (auto &reg : m_reg)
		reg.d = 0;
	m_psw.d = 0;
	m_ppc.d = 0;
}

t11_device::t11_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: t11_device(mconfig, T11, tag, owner, clock)
{
}

device_memory_interface::space_config_vector t11_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}


/*************************************
 *
 *  Low-level memory operations
 *
 *************************************/

int t11_device::ROPCODE()
{
	PC &= 0xfffe;
	int val = m_cache.read_word(PC);
	PC += 2;
	return val;
}


int t11_device::RBYTE(int addr)
{
	return m_program.read_byte(addr);
}


void t11_device::WBYTE(int addr, int data)
{
	m_program.write_byte(addr, data);
}


int t11_device::RWORD(int addr)
{
	return m_program.read_word(addr & 0xfffe);
}


void t11_device::WWORD(int addr, int data)
{
	m_program.write_word(addr & 0xfffe, data);
}



/*************************************
 *
 *  Low-level stack operations
 *
 *************************************/

void t11_device::PUSH(int val)
{
	SP -= 2;
	WWORD(SPD, val);
}


int t11_device::POP()
{
	int result = RWORD(SPD);
	SP += 2;
	return result;
}



/*************************************
 *
 *  Flag definitions and operations
 *
 *************************************/

/* flag definitions */
#define CFLAG 1
#define VFLAG 2
#define ZFLAG 4
#define NFLAG 8

/* extracts flags */
#define GET_C (PSW & CFLAG)
#define GET_V (PSW & VFLAG)
#define GET_Z (PSW & ZFLAG)
#define GET_N (PSW & NFLAG)

/* clears flags */
#define CLR_C (PSW &= ~CFLAG)
#define CLR_V (PSW &= ~VFLAG)
#define CLR_Z (PSW &= ~ZFLAG)
#define CLR_N (PSW &= ~NFLAG)

/* sets flags */
#define SET_C (PSW |= CFLAG)
#define SET_V (PSW |= VFLAG)
#define SET_Z (PSW |= ZFLAG)
#define SET_N (PSW |= NFLAG)



/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

struct irq_table_entry
{
	uint8_t   priority;
	uint8_t   vector;
};

static const struct irq_table_entry irq_table[] =
{
	{ 0<<5, 0000 },
	{ 4<<5, 0070 },
	{ 4<<5, 0064 },
	{ 4<<5, 0060 },
	{ 5<<5, 0134 },
	{ 5<<5, 0130 },
	{ 5<<5, 0124 },
	{ 5<<5, 0120 },
	{ 6<<5, 0114 },
	{ 6<<5, 0110 },
	{ 6<<5, 0104 },
	{ 6<<5, 0100 },
	{ 7<<5, 0154 },
	{ 7<<5, 0150 },
	{ 7<<5, 0144 },
	{ 7<<5, 0140 }
};

void t11_device::t11_check_irqs()
{
	// HLT is nonmaskable
	if (m_ext_halt)
	{
		m_ext_halt = false;

		// push the old state, set the new one
		PUSH(PSW);
		PUSH(PC);
		PCD = m_initial_pc + 4;
		PSW = 0340;

		// count cycles and clear the WAIT flag
		m_icount -= 114;
		m_wait_state = 0;

		return;
	}

	// PF has next-highest priority
	int priority = PSW & 0340;
	if (m_power_fail && priority != 0340)
	{
		m_power_fail = false;
		take_interrupt(T11_PWRFAIL);
		return;
	}

	// compare the priority of the CP interrupt to the PSW
	const struct irq_table_entry *irq = &irq_table[m_cp_state & 15];
	if (irq->priority > priority)
	{
		// call the callback
		standard_irq_callback(m_cp_state & 15);

		// T11 encodes the interrupt level on DAL<12:8>
		uint8_t iaddr = bitswap<4>(~m_cp_state & 15, 0, 1, 2, 3);
		if (!m_vec_active)
			iaddr |= 16;

		// vector is input on DAL<7:2>
		uint8_t vector = m_in_iack_func(iaddr);

		// nonvectored or vectored interrupt depending on VEC
		if (BIT(iaddr, 4))
			take_interrupt(irq->vector);
		else
			take_interrupt(vector & ~3);
	}
}

void t11_device::take_interrupt(uint8_t vector)
{
	// fetch the new PC and PSW from that vector
	assert((vector & 3) == 0);
	uint16_t new_pc = RWORD(vector);
	uint16_t new_psw = RWORD(vector + 2);

	// push the old state, set the new one
	PUSH(PSW);
	PUSH(PC);
	PCD = new_pc;
	PSW = new_psw;

	// count cycles and clear the WAIT flag
	m_icount -= 114;
	m_wait_state = 0;
}



/*************************************
 *
 *  Core opcodes
 *
 *************************************/

/* includes the static function prototypes and the master opcode table */
#include "t11table.hxx"

/* includes the actual opcode implementations */
#include "t11ops.hxx"



/*************************************
 *
 *  Low-level initialization/cleanup
 *
 *************************************/

void t11_device::device_start()
{
	static const uint16_t initial_pc[] =
	{
		0xc000, 0x8000, 0x4000, 0x2000,
		0x1000, 0x0000, 0xf600, 0xf400
	};

	m_initial_pc = initial_pc[c_initial_mode >> 13];
	space(AS_PROGRAM).cache(m_cache);
	space(AS_PROGRAM).specific(m_program);
	m_out_reset_func.resolve_safe();
	m_in_iack_func.resolve_safe(0377);

	save_item(NAME(m_ppc.w.l));
	save_item(NAME(m_reg[0].w.l));
	save_item(NAME(m_reg[1].w.l));
	save_item(NAME(m_reg[2].w.l));
	save_item(NAME(m_reg[3].w.l));
	save_item(NAME(m_reg[4].w.l));
	save_item(NAME(m_reg[5].w.l));
	save_item(NAME(m_reg[6].w.l));
	save_item(NAME(m_reg[7].w.l));
	save_item(NAME(m_psw.w.l));
	save_item(NAME(m_initial_pc));
	save_item(NAME(m_wait_state));
	save_item(NAME(m_cp_state));
	save_item(NAME(m_vec_active));
	save_item(NAME(m_pf_active));
	save_item(NAME(m_hlt_active));
	save_item(NAME(m_power_fail));
	save_item(NAME(m_ext_halt));

	// Register debugger state
	state_add( T11_PC,  "PC",  m_reg[7].w.l).formatstr("%06O");
	state_add( T11_SP,  "SP",  m_reg[6].w.l).formatstr("%06O");
	state_add( T11_PSW, "PSW", m_psw.b.l).formatstr("%03O");
	state_add( T11_R0,  "R0",  m_reg[0].w.l).formatstr("%06O");
	state_add( T11_R1,  "R1",  m_reg[1].w.l).formatstr("%06O");
	state_add( T11_R2,  "R2",  m_reg[2].w.l).formatstr("%06O");
	state_add( T11_R3,  "R3",  m_reg[3].w.l).formatstr("%06O");
	state_add( T11_R4,  "R4",  m_reg[4].w.l).formatstr("%06O");
	state_add( T11_R5,  "R5",  m_reg[5].w.l).formatstr("%06O");

	state_add(STATE_GENPC, "GENPC", m_reg[7].w.l).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_ppc.w.l).noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_psw.b.l).formatstr("%8s").noshow();

	set_icountptr(m_icount);
}

void t11_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c%c%c%c%c",
				m_psw.b.l & 0x80 ? '?':'.',
				m_psw.b.l & 0x40 ? 'I':'.',
				m_psw.b.l & 0x20 ? 'I':'.',
				m_psw.b.l & 0x10 ? 'T':'.',
				m_psw.b.l & 0x08 ? 'N':'.',
				m_psw.b.l & 0x04 ? 'Z':'.',
				m_psw.b.l & 0x02 ? 'V':'.',
				m_psw.b.l & 0x01 ? 'C':'.'
			);
			break;
	}
}

void k1801vm2_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c%c%c%c%c%c",
				m_psw.b.l & 0x100 ? 'H':'.',
				m_psw.b.l & 0x80 ? 'P':'.',
				m_psw.b.l & 0x40 ? '?':'.',
				m_psw.b.l & 0x20 ? '?':'.',
				m_psw.b.l & 0x10 ? 'T':'.',
				m_psw.b.l & 0x08 ? 'N':'.',
				m_psw.b.l & 0x04 ? 'Z':'.',
				m_psw.b.l & 0x02 ? 'V':'.',
				m_psw.b.l & 0x01 ? 'C':'.'
			);
			break;
	}
}


/*************************************
 *
 *  CPU reset
 *
 *************************************/

void t11_device::device_reset()
{
	// initial SP is 376 octal, or 0xfe
	SP = 0376;

	// initial PC comes from the setup word
	PC = m_initial_pc;

	// PSW starts off at highest priority
	PSW = 0340;

	m_wait_state = 0;
	m_power_fail = false;
	m_ext_halt = false;
}

void k1801vm2_device::device_reset()
{
	t11_device::device_reset();

	PC = RWORD(c_initial_mode);
	PSW = RWORD(c_initial_mode+2);
}


/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

void t11_device::execute_set_input(int irqline, int state)
{
	switch (irqline)
	{
	case CP0_LINE:
	case CP1_LINE:
	case CP2_LINE:
	case CP3_LINE:
		// set the appropriate bit
		if (state == CLEAR_LINE)
			m_cp_state &= ~(1 << irqline);
		else
			m_cp_state |= 1 << irqline;
		break;

	case VEC_LINE:
		m_vec_active = (state != CLEAR_LINE);
		break;

	case PF_LINE:
		if (state != CLEAR_LINE && !m_pf_active)
			m_power_fail = true;
		m_pf_active = (state != CLEAR_LINE);
		break;

	case HLT_LINE:
		if (state != CLEAR_LINE && !m_hlt_active)
			m_ext_halt = true;
		m_hlt_active = (state != CLEAR_LINE);
		break;
	}
}



/*************************************
 *
 *  Core execution
 *
 *************************************/

void t11_device::execute_run()
{
	t11_check_irqs();

	if (m_wait_state)
	{
		m_icount = 0;
		return;
	}

	while (m_icount > 0)
	{
		uint16_t op;

		m_ppc = m_reg[7];   /* copy PC to previous PC */

		debugger_instruction_hook(PCD);

		op = ROPCODE();
		(this->*s_opcode_table[op >> 3])(op);
	}
}

std::unique_ptr<util::disasm_interface> t11_device::create_disassembler()
{
	return std::make_unique<t11_disassembler>();
}
