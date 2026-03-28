// license:BSD-3-Clause
// copyright-holders:Golden Child
/******************************************************************************

    Epson MX-80 Dot Matrix printer


 Vertical alignment isn't consistent between lines so graphics printouts look poor.

 This code is ugly but I thought I'd put it up in case my hard drive fails.

    Documentation reference:
        Epson MX-80 Technical Manual
        Epson MX-100 Technical Manual
        Sams Computerfacts CP1 MX-80 IIIF/T
        Sams Computerfacts CP2 MX-100
        Sams Computerfacts CP3 IBM 5152-002

 Main CPU is a 8049/8039 running at 6 MHz.
 Slave CPU is an 8041 running at 6 Mhz.
 --

From the MX-100 Technical manual:

The ROM size is a total of 6k bytes. The following three combinations of CPU and
ROM(s) are available.
(a) 8049 (2-k M-ROM) + 2332 (4-k M-ROM) (1B)
(b) 8039 (2-k M-ROM) + 2716 (1B) + 2716 (2B)
(c) 8049 (2-k UM-ROM) + 2716 (1B) + 2716 (2B) + 2716 (3B)

(what does UM-ROM mean?  unused? unprogrammed?)

NOTES: 1. 18, 2B and 3B represent the socket numbers indicated on the PCB.
2. 2716 is a 2-k PROM.
3. ROM types may vary depending on the user's specifications. Care must
be taken with respect to the compatibility of ROMs.

8049 Program Memory: 2K x 8 Internal Mask ROM

Jumper J1 to select mask rom/external rom on EA pin (External Access)

Active LOW (EA = Gnd): The 8049 accesses internal memory. It automatically switches to external memory only when the program counter exceeds the internal 2K boundary (address 07FFh).


Ports P20 to P23 in the 8041 are used to send the status signals of the printer to the CPU
8049. P20 (PRAV) is sent out after the carriage has finished acceleration and informs the
CPU 8049 of the start of print data transfer. P21 (DIRAV) is sent out whenever the printing
operation is finished and is used by the CPU 8049 to decide the next printing direction.
P22 (DIR) represents a signal indicating the moving direction of the carriage. If it is High,
the carriage will move from left to right. If it is Low, the carriage will move from right to left.
P23 (ERR) signals the CPU 8049 that there is a malfunction in the carriage stepper motor
or the LSI 8041.



================
MX80
================

******************************************************************************/

#include "emu.h"
#include "epson_mx80.h"
#include "sound/dac.h"
#include "speaker.h"
#include "ctronics.h"
#include "sound/beep.h"
#include "machine/bitmap_printer.h"
#include "machine/timer.h"
#include "machine/steppers.h"
#include "machine/i8155.h"
#include "cpu/mcs48/mcs48.h"



namespace {


class spring_stepper {

public:
	spring_stepper()
	{
	}

	void update(double xpos, double m_spring_k, double m_spring_damping, double m_mass, bool init = false)
	{
		if (m_position <= -3000) m_position = xpos;
		if (m_position >= 3000) m_position = xpos;
		static constexpr double quantum = 50E-6; // 25 usec
		static constexpr double timetotal = 100E-6;

		for (int i = 0; i < (timetotal / quantum); i++)
		{
			m_force = - m_spring_k * (m_position - xpos) - m_spring_damping * m_velocity;
			m_acceleration = m_force / m_mass;
			m_velocity = m_velocity + (m_acceleration * quantum);
			m_position += (m_velocity * quantum);  // delta = velocity * time
		}
	}

	double m_force;
	double m_position = -3000;
	double m_velocity = 0;
	double m_acceleration;
};


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class epson_mx80_device :  public device_t, public device_centronics_peripheral_interface
{
public:
	static constexpr feature_type unemulated_features() { return feature::PRINTER; }

	// construction/destruction
	epson_mx80_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_INPUT_CHANGED_MEMBER(online_switch);
	DECLARE_INPUT_CHANGED_MEMBER(reset_switch);

	/* Centronics stuff */

	virtual void input_init(int state) override;
	virtual void input_strobe(int state) override;
	virtual void input_data0(int state) override { if (state) m_centronics_data |= 0x01; else m_centronics_data &= ~0x01; }
	virtual void input_data1(int state) override { if (state) m_centronics_data |= 0x02; else m_centronics_data &= ~0x02; }
	virtual void input_data2(int state) override { if (state) m_centronics_data |= 0x04; else m_centronics_data &= ~0x04; }
	virtual void input_data3(int state) override { if (state) m_centronics_data |= 0x08; else m_centronics_data &= ~0x08; }
	virtual void input_data4(int state) override { if (state) m_centronics_data |= 0x10; else m_centronics_data &= ~0x10; }
	virtual void input_data5(int state) override { if (state) m_centronics_data |= 0x20; else m_centronics_data &= ~0x20; }
	virtual void input_data6(int state) override { if (state) m_centronics_data |= 0x40; else m_centronics_data &= ~0x40; }
	virtual void input_data7(int state) override { if (state) m_centronics_data |= 0x80; else m_centronics_data &= ~0x80; }


protected:
	// constructor to pass along a device type
	epson_mx80_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// for cr timer
//  virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual bool supports_pin35_5v() override { return true; }

protected:
	uint8_t t0_r();
	uint8_t t1_r();
	uint8_t port1_r();
	uint8_t port2_r();
	uint8_t bus_r();
	void port1_w(uint8_t data);
	void port2_w(uint8_t data);
	void bus_w(uint8_t data);


	void mx80_io_mem(address_map &map);
	void mx80_data_mem(address_map &map);
	void mx80_prog_mem(address_map &map);

	uint8_t data_r(offs_t offset);
	uint8_t io_r(offs_t offset);
	void io_w(offs_t offset, uint8_t data);
	//uint8_t dipsw_r(offs_t offset);
	//uint8_t switches_r(offs_t offset);
	uint8_t prog_mem_r(offs_t offset);


	uint8_t i8155_pa_r();
	void i8155_pa_w(uint8_t data);
	uint8_t i8155_pb_r();
	void i8155_pb_w(uint8_t data);
	uint8_t i8155_pc_r();
	void i8155_pc_w(uint8_t data);

	required_device<i8039_device> m_maincpu;
	required_device<i8041ah_device> m_slavecpu;
	required_device<i8155_device> m_i8155;
	required_device<beep_device> m_beeper;
	required_device<stepper_device> m_ribbon_stepper;
	required_device<bitmap_printer_device> m_bitmap_printer;
	int m_irq_state;

	u32 m_printhead;
	u8 m_printhead_counter;

	u32 drawcmy(u32 inpix, u32 drawcolor);

	void co0_w(int state);
	void co0_w_old(int state);
	//DECLARE_WRITE_LINE_MEMBER(co0_w);
	//DECLARE_WRITE_LINE_MEMBER(co0_w_old);

	static constexpr int CR_OFFSET = (-14);
	static constexpr int PAPER_WIDTH = 1024;     // 120 dpi * 8.5333 inches
	static constexpr int PAPER_HEIGHT = (11*72); //  72 dpi * 11 inches


	uint8_t slave_bus_r();
	void slave_bus_w(uint8_t data);
	uint8_t slave_port1_r();
	void slave_port1_w(uint8_t data);
	uint8_t slave_port2_r();
	void slave_port2_w(uint8_t data);
	uint8_t slave_t0_r();
	uint8_t slave_t1_r();

	u8 home_sensor();

	/* Centronics stuff */
	uint8_t m_centronics_data;
	int m_centronics_busy;
	int m_centronics_nack;
	uint8_t m_centronics_init = true;
	uint8_t m_centronics_strobe;
	uint8_t m_centronics_data_latch;
	uint8_t m_centronics_data_latched;
	uint8_t m_data_ready;  // ic 6b flip flop activates INT1 line

	void update_busy(){ output_busy(((m_centronics_data_latched) | BIT(m_8049_p1, 6)) ^ ioport("INVERTBUSY")->read()); };
	void update_ack(){ output_ack(((m_centronics_data_latched) | BIT(m_8049_p1, 5)) ^ ioport("INVERTACK")->read()); };


	uint8_t pts_r();

	// uint8_t centronics_data_r(offs_t offset);  // NOT USED

	u8 m_pb0;

	u8 m_manual_pts;

	// stuff to track the cr movement
	double lastmovecrtime = -1.0;
	int lastmovecrpos = 0;
	double lastmovecrposd = 0.0;
	double lastmovecrspeed = 0;

public:
	void trackcrpos();  // call this after each update to the cr stepper
[[maybe_unused]]    double calccrpos(); // calculate interim positions in between discrete cr stepper steps
protected:

	u8 m_8049_p1 = 0;
	u8 m_8049_p2 = 0;

	u8 m_slave_p1 = 0;
	u8 m_slave_p2 = 0;

	u8 m_8155_pa = 0;
	u8 m_8155_pb = 0;
	u8 m_8155_pc = 0;

	TIMER_CALLBACK_MEMBER(slave_write_data_sync);
	TIMER_CALLBACK_MEMBER(slave_write_command_sync);
	uint8_t slave_r(offs_t offset);
	void slave_w(offs_t offset, uint8_t data);

	spring_stepper m_cr_stepper = spring_stepper();
	TIMER_DEVICE_CALLBACK_MEMBER(cr_stepper_periodic);
	int fix16(int x){ if (x>=32768) x = x - 65536; return x; }
};

TIMER_DEVICE_CALLBACK_MEMBER(epson_mx80_device::cr_stepper_periodic)
{
	m_cr_stepper.update(m_bitmap_printer->m_xpos,
		ioport("SPRINGCONSTANT")->read() / 0.1,
		ioport("SPRINGDAMPING")->read() / 100.0,
		ioport("SPRINGMASS")->read() / 10000.0);
//      m_spring_k       = ioport("SPRINGCONSTANT")->read() / 100.0;
//      m_spring_damping = ioport("SPRINGDAMPING")->read() / 100.0;
//      m_mass           = ioport("SPRINGMASS")->read() / 100.0;
};


class epson_mx80dots_device : public epson_mx80_device
{
public:
	// construction/destruction
	epson_mx80dots_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		epson_mx80_device(mconfig, EPSON_MX80_DOTS, tag, owner, clock)
	{
	}

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	void mx80dots_prog_mem(address_map &map);
	uint8_t dots_perfect_prog_mem_r(offs_t offset);

	//DECLARE_WRITE_LINE_MEMBER(prog_w);
	virtual void prog_w(int state);

	u8 m_dots_bank = 0;
	uint8_t m_prog_line = 1;  // status of prog_line
};


class epson_mx80_iii_device : public epson_mx80_device
{
public:
	// construction/destruction
	epson_mx80_iii_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		epson_mx80_device(mconfig, EPSON_MX80_III,  tag, owner, clock)
	{
	}

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};





//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************


//-------------------------------------------------
//  ROM( epson_mx80 )
//-------------------------------------------------





ROM_START( epson_mx80 )
	ROM_REGION(0x1800, "mx80_rom", 0)
	ROM_LOAD("mx80_graftrax_rom_1.bin", 0x0000, 0x800, CRC(0843ea56) SHA1(23948acb55760ddbfc59c57c1553b2e733983201))
	ROM_LOAD("mx80_graftrax_rom_2.bin", 0x0800, 0x800, CRC(8435f945) SHA1(c1f0a74bf66d114d0f349bec27c7756997352602))
	ROM_LOAD("mx80_graftrax_rom_3.bin", 0x1000, 0x800, CRC(f1d1c0e7) SHA1(1f4e24575c412368f0b628d2a276c9943debd0ae))

	ROM_REGION(0x400, "i8041_slave", 0)
	ROM_LOAD("8041_mx80.bin", 0x0000, 0x400, CRC(5844ef51) SHA1(1025d34b3ab684a06589b5890c604ce114399d23))
ROM_END




uint8_t epson_mx80_device::prog_mem_r(offs_t offset)
{
	u8 *mx80_rom_ptr = memregion("mx80_rom")->base();

	[[maybe_unused]] auto offset_low = offset & ((1 << 11) - 1); // get low 11 bits  not 1<<12 - 1 but 1<<11 - 1
	auto offset_upper = (offset & (0xf800)) >> 11;

	u8 retval = (offset_upper == 0) ?
		mx80_rom_ptr[ offset_low | 0x800 * 2] :
			(m_8049_p2 & 0x10) ?
				mx80_rom_ptr[offset_low | 0x0] : mx80_rom_ptr[offset_low | 0x800];
	return retval;
}


ROM_START( epson_mx80_iii )

	ROM_REGION(0x1800, "mx80_rom", 0)
	ROM_LOAD("a2_ha1_1b.bin", 0x0000, 0x800, CRC(5a8a8dec) SHA1(c7f7505e0e6a5916fe17c0a614207a5d1c97e130))
	ROM_LOAD("a1_ha2_2b.bin", 0x0800, 0x800, CRC(a6b448bc) SHA1(bbe8c211b726fbd2fcc92891b8fd475120678e34))
	ROM_LOAD("a2_ha3_3b.bin", 0x1000, 0x800, CRC(a9a2ac25) SHA1(4f66bf77b628c6ca8e61373d49efd28133463228))

	// seems to work (at least with the 8041 dump from the mx80)
	ROM_REGION(0x400, "i8041_slave", 0)
	ROM_LOAD("8041_mx80.bin", 0x0000, 0x400, CRC(5844ef51) SHA1(1025d34b3ab684a06589b5890c604ce114399d23))

	//ROM_REGION(0x400, "i8041_slave", 0)
	//ROM_LOAD("d8041c_9b.bin", 0x0000, 0x400, CRC(d26dd7fb) SHA1(a1af97b475fe6223867040429b392ce90f4d5d3c))
ROM_END


ROM_START( epson_mx80dots )
	ROM_REGION(0x4000, "dots_perfect", 0)
	ROM_LOAD("dots_perfect_mx80_27128.bin", 0x0000, 0x4000, CRC(49bae08f) SHA1(702b3fd3e8fefd983306028915a49cb26880bb16))

	ROM_REGION(0x400, "i8041_slave", 0)
	ROM_LOAD("8041_mx80.bin", 0x0000, 0x400, CRC(5844ef51) SHA1(1025d34b3ab684a06589b5890c604ce114399d23))
ROM_END



//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *epson_mx80_device::device_rom_region() const
{
	return ROM_NAME( epson_mx80 );
}

const tiny_rom_entry *epson_mx80_iii_device::device_rom_region() const
{
	return ROM_NAME( epson_mx80_iii );
}



const tiny_rom_entry *epson_mx80dots_device::device_rom_region() const
{
	return ROM_NAME( epson_mx80dots );
}


//-------------------------------------------------
//  ADDRESS_MAP( mx80_mem )
//-------------------------------------------------

void epson_mx80_device::mx80_data_mem(address_map &map)
{
	map(0x00, 0x7f).r(FUNC(epson_mx80_device::data_r));
}

void epson_mx80_device::mx80_io_mem(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(epson_mx80_device::io_r),FUNC(epson_mx80_device::io_w));
}

void epson_mx80_device::mx80_prog_mem(address_map &map)
{
	map(0x000, 0xfff).r(FUNC(epson_mx80_device::prog_mem_r));  // can't do more than 12 bits
}

void epson_mx80dots_device::mx80dots_prog_mem(address_map &map)
{
	map(0x000, 0xfff).r(FUNC(epson_mx80dots_device::dots_perfect_prog_mem_r));  // can't do more than 12 bits
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void epson_mx80_device::device_add_mconfig(machine_config &config)
{
	// 8049 main cpu
//  i8049_device &main(I8049(config, m_maincpu, 6000000)); // 6 Mhz
	i8039_device &main(I8039(config, m_maincpu, 6000000)); // 6 Mhz can be 8039 or 8049 according to schematic

	main.set_addrmap(AS_PROGRAM, &epson_mx80_device::mx80_prog_mem);
	main.set_addrmap(AS_DATA, &epson_mx80_device::mx80_data_mem);
	main.set_addrmap(AS_IO, &epson_mx80_device::mx80_io_mem);

	main.p1_in_cb().set(FUNC(epson_mx80_device::port1_r));
	main.p1_out_cb().set(FUNC(epson_mx80_device::port1_w));
	main.p2_in_cb().set(FUNC(epson_mx80_device::port2_r));
	main.p2_out_cb().set(FUNC(epson_mx80_device::port2_w));
	main.bus_in_cb().set(FUNC(epson_mx80_device::bus_r));
	main.bus_out_cb().set(FUNC(epson_mx80_device::bus_w));
	main.t0_in_cb().set(FUNC(epson_mx80_device::t0_r));  // home sensor
	main.t1_in_cb().set(FUNC(epson_mx80_device::t1_r));  // pts sensor

	[[maybe_unused]] i8041ah_device &slave(I8041AH(config, m_slavecpu, 6000000));  // a zero clock rate won't stop on debugger focus
	slave.p1_in_cb().set(FUNC(epson_mx80_device::slave_port1_r));
	slave.p1_out_cb().set(FUNC(epson_mx80_device::slave_port1_w));
	slave.p2_in_cb().set(FUNC(epson_mx80_device::slave_port2_r));
	slave.p2_out_cb().set(FUNC(epson_mx80_device::slave_port2_w));
	slave.bus_in_cb().set(FUNC(epson_mx80_device::slave_bus_r));
	slave.bus_out_cb().set(FUNC(epson_mx80_device::slave_bus_w));
	slave.t0_in_cb().set(FUNC(epson_mx80_device::slave_t0_r));  // home sensor
	slave.t1_in_cb().set(FUNC(epson_mx80_device::slave_t1_r));  // pts sensor


	// 8155 timer input is connected to the PTS sensor, not emulated
	[[maybe_unused]] i8155_device &i8155(I8155(config, m_i8155, 0));
	i8155.in_pa_callback().set(FUNC(epson_mx80_device::i8155_pa_r));
	i8155.out_pa_callback().set(FUNC(epson_mx80_device::i8155_pa_w));
	i8155.in_pb_callback().set(FUNC(epson_mx80_device::i8155_pb_r));
	i8155.out_pb_callback().set(FUNC(epson_mx80_device::i8155_pb_w));
	i8155.in_pc_callback().set(FUNC(epson_mx80_device::i8155_pc_r));
	i8155.out_pc_callback().set(FUNC(epson_mx80_device::i8155_pc_w));


//  config.set_default_layout(layout_mx80);

	/* audio hardware */
	BITMAP_PRINTER(config, m_bitmap_printer, PAPER_WIDTH, PAPER_HEIGHT, 120, 72);  // do 72 dpi
	m_bitmap_printer->set_pf_stepper_ratio(1,6);  // pf stepper moves at 216 dpi so at 72dpi half steps
	m_bitmap_printer->set_cr_stepper_ratio(1,1);
	//m_bitmap_printer->set_screen_update(FUNC(epson_mx80_device::screen_update_bitmap));

	/* audio hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.25);

	/* audio hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 4000); // measured at 4000 Hz */
	m_beeper->add_route(ALL_OUTPUTS, "mono", 1.0);

	//TIMER(config, "cr_stepper_update").configure_periodic(FUNC(epson_mx80_device::cr_stepper_periodic), attotime::from_usec(100));


	STEPPER(config, m_ribbon_stepper);

	TIMER(config, "cr_stepper_update").configure_periodic(FUNC(epson_mx80_device::cr_stepper_periodic), attotime::from_usec(100));

}

/*
void epson_mx80alt_device::device_add_mconfig(machine_config &config)
{
    epson_mx80_device::device_add_mconfig(config);
    m_maincpu->set_addrmap(AS_PROGRAM, &epson_mx80alt_device::mx80alt_prog_mem);
}
*/

void epson_mx80dots_device::device_add_mconfig(machine_config &config)
{
	epson_mx80_device::device_add_mconfig(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &epson_mx80dots_device::mx80dots_prog_mem);
	m_maincpu->prog_out_cb().set(FUNC(epson_mx80dots_device::prog_w));  // prog output to switch banks (dots perfect)
}


uint8_t epson_mx80_device::i8155_pa_r()  // this is a hack for testing
{
	m_8155_pa = m_centronics_data_latch;
	m_centronics_data_latched = false;
	//m_maincpu->set_input_line(UPD7810_INTF1, m_centronics_data_latched);  // it's rising edge
	m_maincpu->set_input_line(MCS48_INPUT_IRQ, m_centronics_data_latched);  // it's rising edge
	update_busy();
	update_ack();
	return m_8155_pa;
}

void epson_mx80_device::i8155_pa_w(uint8_t data)
{
	m_8155_pa = data;
}
uint8_t epson_mx80_device::i8155_pb_r()
{
	return m_8155_pb;
}
void epson_mx80_device::i8155_pb_w(uint8_t data)
{
	m_8155_pb = data;
	m_printhead = (m_printhead & 0xff00) | data;
}
uint8_t epson_mx80_device::i8155_pc_r()
{
	return m_8155_pc;
}
void epson_mx80_device::i8155_pc_w(uint8_t data)
{
	m_8155_pc = data;
	m_printhead = (m_printhead & 0xff) | (BIT(data, 4) << 15);
}



uint8_t epson_mx80_device::slave_t1_r()
{
	return home_sensor();
}

uint8_t epson_mx80_device::slave_t0_r() // print timing sensor
{
	return pts_r();
}

uint8_t epson_mx80_device::t0_r()
{
// mx80 - connects to PA7  -- possible serial?
	return 0;
}

uint8_t epson_mx80_device::t1_r() // print timing sensor
{
	return pts_r();  // connected to same line
}



uint8_t epson_mx80_device::data_r(offs_t offset)
{
	printf("DATA R %x\n",offset);
	return 0;
}

uint8_t epson_mx80_device::io_r(offs_t offset)
{

// p20-23 = select IO device  0x700 = 8155 0xb00 = 8041 0xd00 = dipsw1 0xe00 = switches
// p24 = bank switching
// p25 = IO / not M    or  A0
// p26 = head trigger
// p27 = err
	//printf("IO R %x   p2 0-3=%x  p25=%x\n",offset, BIT(m_8049_p2,0,4),BIT(m_8049_p2,5));

	u8 retval;

	switch (BIT(m_8049_p2,0,4))
	{
		case 0x7 : // read from 8155  io / m' is on p25
			if (BIT(m_8049_p2, 5))
			{
				return m_i8155->io_r(offset);
			}
			else
			{
				return m_i8155->memory_r(offset);
			}
			break;
		case 0xb : // read from 8041

			retval = m_slavecpu->upi41_master_r(BIT(m_8049_p2,5));
			//printf("io_r 0xb slave retval=%x\n",retval);
			return retval;
			break;

		case 0xd : // read dip switches
			return ioport("DIPSW1")->read() ^ 0xff;  // all the switches from DIPSW1
			break;

		case 0xe : // read switches
			return  ioport("ONLINE")->read()   << 0 |
					ioport("PAPERERR")->read() << 1 |
					ioport("LINEFEED")->read() << 2 |
					ioport("FORMFEED")->read() << 3 |
					0x00; // 0xf0;  // pulled high
			break;
	}

	return 0;
}


void epson_mx80_device::io_w(offs_t offset, uint8_t data)
{

	printf("IO W offset %x = %x  %x\n",offset, data, BIT(m_8049_p2, 0, 4));

	switch (BIT(m_8049_p2,0,4)) // 4 is num of bits
	{
		case 0x7 : // read from 8155  io / m' is on p25
			if (BIT(m_8049_p2, 5))
			{
				return m_i8155->io_w(offset, data);
			}
			else
			{
				return m_i8155->memory_w(offset, data);
			}
			break;
		case 0xb : // read from 8041
			slave_w(BIT(m_8049_p2,5), data);
			break;
			return m_slavecpu->upi41_master_w(BIT(m_8049_p2,5), data);
			break;
	}
}



uint8_t epson_mx80_device::slave_port1_r()
{
	u8 retval = m_slave_p1;  // reads the value of the port
	logerror("8041 slave_PORT1_read =%02x (%s)\n", retval, machine().describe_context());
	return retval;
}



void epson_mx80_device::slave_port1_w(uint8_t data)
{
	// sp10 - p11 = cr_stepper
	// sp12 - p13 = 80/132 columns
	// sp14 - 15 = line feed
	// sp16 = select line feed
	// sp17 buzzer

	int origpos = m_bitmap_printer->m_xpos;

	int olddata = m_slave_p1;

	m_slave_p1 = data;

	m_bitmap_printer->update_cr_stepper(
		( BIT(data, 0) << 3) |
		(!BIT(data, 0) << 2) |
		( BIT(data, 1) << 1) |
		(!BIT(data, 1) << 0));
	trackcrpos();

	m_bitmap_printer->update_pf_stepper(
		( BIT(data, 4) << 3) |
		(!BIT(data, 4) << 2) |
		( BIT(data, 5) << 1) |
		(!BIT(data, 5) << 0));

	m_beeper->set_state(!BIT(data,7));  // maybe inverted?

	logerror("8041 slave_port1_w =%02x (%s)   olddata=%x\n", data, machine().describe_context(), olddata);

	if (m_bitmap_printer->m_xpos != origpos) printf("new xpos=%x orig xpos=%x\n", m_bitmap_printer->m_xpos, origpos);

}




	// p10 = online lamp
	// p11 = prav' sp20
	// p12 = err' sp23
	// p13 = dirav' sp21
	// p14 = dir sp22
	// p15 = ack
	// p16 = busy
	// p17 = auto feed (connects also to SW2-3)

	// p20-23
	// p24 bank switch
	// p25 io/m' a0
	// p26 head trigger
	// p27 err' (centronics)


uint8_t epson_mx80_device::slave_port2_r()
{
	// sp20 PRAV'
	// sp21 DIRAV'
	// sp22 DIR
	// sp23 ERR'
	// sp24 SW 2-4
	// sp25 SW 2-2
	// sp26 SW 2-1
	// sp27 parallel/serial'

	u8 dipsw2 = ioport("DIPSW2")->read() ^ 0xff;

	u8 retval =
				(m_slave_p2 & 0xf) |
				BIT(dipsw2, 3) << 4 |
				BIT(dipsw2, 1) << 5 |
				BIT(dipsw2, 0) << 6 |
				1 << 7; // parallel / serial

	logerror("slave_PORT2_read =%02x (%s)\n", retval, machine().describe_context());
	return retval;
}

void epson_mx80_device::slave_port2_w(uint8_t data)
{
	// sp20 PRAV'
	// sp21 DIRAV'
	// sp22 DIR
	// sp23 ERR'
	// sp24 SW 2-4
	// sp25 SW 2-2
	// sp26 SW 2-1
	// sp27 parallel/serial'

	m_slave_p2 = data;
	logerror("8041 slave_p2_w =%02x (%s)\n", data, machine().describe_context());
}

uint8_t epson_mx80_device::slave_bus_r()
{
	u8 retval = 0;
	logerror("slave_BUS_read =%02x (%s)\n", retval, machine().describe_context());
	return retval;
}

void epson_mx80_device::slave_bus_w(uint8_t data)
{
	logerror("8041 SLAVE bus_w =%02x (%s)\n", data, machine().describe_context());
}


INPUT_CHANGED_MEMBER(epson_mx80_device::online_switch)
{
	printf("ONLINE SWITCH WRITE %x\n",newval);
}

INPUT_CHANGED_MEMBER(epson_mx80_device::reset_switch)
{
	if (newval)
	{
		m_maincpu->reset();
		m_slavecpu->reset();
		device_reset();
	}
}


//-------------------------------------------------
//  INPUT_PORTS( epson_mx80 )
//-------------------------------------------------

#define PORT_ADJUSTER_16MASK(_default, _name)                   \
		configurer.field_alloc(IPT_ADJUSTER, (_default), 0xffff, (_name)); \
		configurer.field_set_min_max(0, 100);

INPUT_PORTS_START( epson_mx80_common )
	PORT_START("ONLINE")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ON LINE") PORT_CODE(KEYCODE_0_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(epson_mx80_device::online_switch), 0)

	PORT_START("FORMFEED")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Form Feed") PORT_CODE(KEYCODE_7_PAD)

	PORT_START("LINEFEED")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Line Feed") PORT_CODE(KEYCODE_9_PAD)

	PORT_START("PAPERERR")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Paper Error") PORT_CODE(KEYCODE_1_PAD)

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RESET") PORT_CODE(KEYCODE_8_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(epson_mx80_device::reset_switch), 0)

PORT_START("INVERT1")
	PORT_DIPNAME(0x01, 0x00, "INVERT1") PORT_DIPLOCATION("INVERT 1:!1")
	PORT_DIPSETTING(   0x00, DEF_STR(Off))
	PORT_DIPSETTING(   0x01, DEF_STR(On))

	PORT_START("INVERTHOME")
	PORT_DIPNAME(0x01, 0x00, "INVERT_HOME_8042") PORT_DIPLOCATION("INVERTHOME:!1")
	PORT_DIPSETTING(   0x00, DEF_STR(Off))
	PORT_DIPSETTING(   0x01, DEF_STR(On))

	PORT_START("INVERTACK")
	PORT_DIPNAME(0x01, 0x00, "INVERT_ACK") PORT_DIPLOCATION("INVERTACK:!1")
	PORT_DIPSETTING(   0x00, DEF_STR(Off))
	PORT_DIPSETTING(   0x01, DEF_STR(On))

	PORT_START("INVERTBUSY")
	PORT_DIPNAME(0x01, 0x00, "INVERT_BUSY") PORT_DIPLOCATION("INVERTBUSY:!1")
	PORT_DIPSETTING(   0x00, DEF_STR(Off))
	PORT_DIPSETTING(   0x01, DEF_STR(On))

	PORT_START("DELAY1")
	PORT_ADJUSTER_16MASK(99, "DELAY1")
	PORT_MINMAX(0,255)

	PORT_START("DELAY2")
	PORT_ADJUSTER_16MASK(1, "DELAY2")
	PORT_MINMAX(0,512)

	PORT_START("DELAY3")
	PORT_ADJUSTER_16MASK(100, "DELAY3")
	PORT_MINMAX(0,512)

	PORT_START("RIGHTOFFSET")
	PORT_ADJUSTER_16MASK(2, "Rightward Offset")
	PORT_MINMAX(-16,16)

	PORT_START("SPRINGCONSTANT")
	PORT_ADJUSTER_16MASK(18, "Spring Constant")
	PORT_MINMAX(0,10000)

	PORT_START("SPRINGDAMPING")
	PORT_ADJUSTER_16MASK(18, "Spring Damping")
	PORT_MINMAX(0,64000)

	PORT_START("SPRINGMASS")
	PORT_ADJUSTER_16MASK(18, "Mass")
	PORT_MINMAX(1,1000)

	PORT_START("MULTIPLIER")
	PORT_ADJUSTER_16MASK(1, "MULTIPLIER")
	PORT_MINMAX(-10,10)

	PORT_START("MULTIPLIERDIV")
	PORT_ADJUSTER_16MASK(1, "MULTIPLIERDIV")
	PORT_MINMAX(1,10)

	PORT_START("PTS0")
	PORT_ADJUSTER_16MASK(45, "PTS0")
	PORT_MINMAX(1,100)

	PORT_START("PTS1")
	PORT_ADJUSTER_16MASK(55, "PTS1")
	PORT_MINMAX(1,100)
/*
    PORT_START("SELECTIN")
    PORT_DIPNAME(0xff, 0xff, "Select In") PORT_DIPLOCATION("SW 1:!1")
    PORT_DIPSETTING( 0x00, DEF_STR(Off))
    PORT_DIPSETTING( 0xff, DEF_STR(On))  // didn't work when I changed this value to 01 because it's an ANALOG read
*/
INPUT_PORTS_END


INPUT_PORTS_START( epson_mx80 )
	PORT_INCLUDE (epson_mx80_common)

	PORT_START("DIPSW1")
	PORT_DIPNAME(0x01, 0x00, "Line Spacing") PORT_DIPLOCATION("SW 1:!1")
	PORT_DIPSETTING(   0x00, "1/6\"")
	PORT_DIPSETTING(   0x01, "1/8\"")
	PORT_DIPNAME(0x02, 0x00, "Form Length") PORT_DIPLOCATION("SW 1:!2")
	PORT_DIPSETTING(   0x00, "11\"")
	PORT_DIPSETTING(   0x02, "12\"")
	PORT_DIPNAME(0x0c, 0x00, "Character Size") PORT_DIPLOCATION("SW 1:!3,!4")
	PORT_DIPSETTING(   0x00, "Normal (80)")
	PORT_DIPSETTING(   0x08, "Condensed (132)")
	PORT_DIPSETTING(   0x0c, "Emphasized (80)")
	PORT_DIPNAME(0x10, 0x00, "Always leave OFF") PORT_DIPLOCATION("SW 1:!5")
	PORT_DIPSETTING(   0x00, DEF_STR(Off))
	PORT_DIPSETTING(   0x10, DEF_STR(On))
	PORT_DIPNAME(0x20, 0x00, "Paper End Detector") PORT_DIPLOCATION("SW 1:!6")
	PORT_DIPSETTING(   0x00, "Valid")
	PORT_DIPSETTING(   0x20, "Invalid")
	PORT_DIPNAME(0x40, 0x40, "Intl. character set") PORT_DIPLOCATION("SW 1:!7")
	PORT_DIPSETTING(   0x00, "Denmark/Sweden/Italy/Spain")
	PORT_DIPSETTING(   0x40, "USA/France/Germany/England")
	PORT_DIPNAME(0x80, 0x80, "Select In") PORT_DIPLOCATION("SW 1:!8")
	PORT_DIPSETTING(   0x00, "Not Fixed")
	PORT_DIPSETTING(   0x80, "Fixed")

	PORT_START("DIPSW2")
	PORT_DIPNAME(0x03, 0x03, "Intl. character set") PORT_DIPLOCATION("SW 2:!1,!2")
	PORT_DIPSETTING(   0x00, "England/Spain")
	PORT_DIPSETTING(   0x01, "France/Sweden")
	PORT_DIPSETTING(   0x02, "Germany/Italy")
	PORT_DIPSETTING(   0x03, "USA/Denmark")
	PORT_DIPNAME(0x04, 0x00, "Auto Feed Signal") PORT_DIPLOCATION("SW 2:!3")
	PORT_DIPSETTING(   0x00, "Not Fixed")
	PORT_DIPSETTING(   0x04, "Fixed")
	PORT_DIPNAME(0x08, 0x08, "Skip-over-perforation") PORT_DIPLOCATION("SW 2:!4")
	PORT_DIPSETTING(   0x00, "1 inch")
	PORT_DIPSETTING(   0x08, DEF_STR(None))
INPUT_PORTS_END


INPUT_PORTS_START( epson_mx80dots )
	PORT_INCLUDE (epson_mx80_common)

	PORT_START("DIPSW1")
	PORT_DIPNAME(0x01, 0x00, "Compressed") PORT_DIPLOCATION("SW 1:!1")
	PORT_DIPSETTING(   0x00, DEF_STR(Off))
	PORT_DIPSETTING(   0x01, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x00, "Character Set") PORT_DIPLOCATION("SW 1:!2")
	PORT_DIPSETTING(   0x00, "Epson")
	PORT_DIPSETTING(   0x02, "IBM")
	PORT_DIPNAME(0x04, 0x04, "Paper End Detector") PORT_DIPLOCATION("SW 1:!3")
	PORT_DIPSETTING(   0x00, "Valid")
	PORT_DIPSETTING(   0x04, "Disabled")
	PORT_DIPNAME(0x08, 0x00, "Italic") PORT_DIPLOCATION("SW 1:!4")
	PORT_DIPSETTING(   0x00, DEF_STR(Off))
	PORT_DIPSETTING(   0x08, DEF_STR(On))
	PORT_DIPNAME(0x10, 0x00, "Emphasized") PORT_DIPLOCATION("SW 1:!5")
	PORT_DIPSETTING(   0x00, DEF_STR(Off))
	PORT_DIPSETTING(   0x10, DEF_STR(On))
	PORT_DIPNAME(0x20, 0x20, "Buzzer") PORT_DIPLOCATION("SW 1:!6")
	PORT_DIPSETTING(   0x00, DEF_STR(Off))
	PORT_DIPSETTING(   0x20, DEF_STR(On))
	PORT_DIPNAME(0x40, 0x00, "Zero font slash") PORT_DIPLOCATION("SW 1:!7")
	PORT_DIPSETTING(   0x00, DEF_STR(Off))
	PORT_DIPSETTING(   0x40, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x80, "Select In") PORT_DIPLOCATION("SW 1:!8")
	PORT_DIPSETTING(   0x00, "Not Fixed")
	PORT_DIPSETTING(   0x80, "Fixed")

	PORT_START("DIPSW2")
	PORT_DIPNAME(0x01, 0x00, "Paper Width") PORT_DIPLOCATION("SW 2:!1")
	PORT_DIPSETTING(   0x00, "8.5\"")
	PORT_DIPSETTING(   0x01, "13.5\"")
	PORT_DIPNAME(0x02, 0x00, "Print Mode") PORT_DIPLOCATION("SW 2:!2")
	PORT_DIPSETTING(   0x00, "Draft")
	PORT_DIPSETTING(   0x02, "NLQ")
	PORT_DIPNAME(0x04, 0x04, "Auto LF with CR") PORT_DIPLOCATION("SW 2:!3")
	PORT_DIPSETTING(   0x00, "Fixed On")
	PORT_DIPSETTING(   0x04, "From Host")
	PORT_DIPNAME(0x08, 0x08, "Skip-over-perforation") PORT_DIPLOCATION("SW 2:!4")
	PORT_DIPSETTING(   0x00, "1 inch")
	PORT_DIPSETTING(   0x08, DEF_STR(None))
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor epson_mx80_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( epson_mx80 );
}

ioport_constructor epson_mx80dots_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( epson_mx80dots );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  epson_mx80_device - constructor
//-------------------------------------------------

epson_mx80_device::epson_mx80_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	epson_mx80_device(mconfig, EPSON_MX80, tag, owner, clock)
{
}

// constructor that passes device type
epson_mx80_device::epson_mx80_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_centronics_peripheral_interface(mconfig, *this),
	m_maincpu(*this, "maincpu"),
	m_slavecpu(*this, "i8041_slave"),
	m_i8155(*this, "i8155_io"),
	m_beeper(*this, "beeper"),
	m_ribbon_stepper(*this, "ribbon_stepper"),
	m_bitmap_printer(*this, "bitmap_printer"),
	m_irq_state(0),
	m_printhead(0xffffff),
	m_printhead_counter(0)
{
}






//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void epson_mx80_device::device_start()
{
	m_irq_state = ASSERT_LINE;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void epson_mx80_device::device_reset()
{
	output_perror(false);
//  output_fault (1);
	output_select(true);

	m_centronics_data_latched = 0;
	update_busy();
	update_ack();
	//m_maincpu->set_input_line(UPD7810_INTF1, m_centronics_data_latched);

}

uint8_t epson_mx80_device::port1_r()
{
	// p10 = online lamp
	// p11 = prav' p20
	// p12 = err' p23
	// p13 = dirav' p21
	// p14 = dir p22
	// p15 = ack
	// p16 = busy
	// p17 = auto feed (connects also to SW2-3)

	return  BIT(m_8049_p1, 0) << 0 |  // reads online lamp status at 1:61a   YES!  THIS FIXED THE ONLINE SWITCH!
			BIT(m_slave_p2, 0) << 1 |
			BIT(m_slave_p2, 3) << 2 |
			BIT(m_slave_p2, 1) << 3 |
			BIT(m_slave_p2, 2) << 4 |
			BIT(m_8049_p1, 5) << 5 |
			BIT(m_8049_p1, 6) << 6 |
			BIT(ioport("DIPSW2")->read(),2) << 7;  // p17 = auto feed
}

uint8_t epson_mx80_device::port2_r()
{

	// p20-23
	// p24 bank switch
	// p25 io/m' a0
	// p26 head trigger
	// p27 err' (centronics)

	u8 result = m_8049_p2;
	logerror("P2 R = %x %s\n", result, machine().describe_context());
	return result;
}

uint8_t epson_mx80_device::bus_r()
{
	u8 retval = 0;


	switch (BIT(m_8049_p2,0,4))
	{
		case 0x7 : // read from 8155  io / m' is on p25
			if (BIT(m_8049_p2, 5))
			{
//              retval = m_i8155->io_r(offset);
				printf("BUS READING FROM 8155 IO retval=%x\n",retval);
				break;
			}
			else
			{
//              retval = m_i8155->memory_r(offset);
				printf("BUS READING FROM 8155 MEMORY retval=%x\n",retval);
				break;
			}
			break;
		case 0xb : // read from 8041
			return slave_r(BIT(m_8049_p2,5));
//          retval = m_slavecpu->upi41_master_r(BIT(m_8049_p2,5));
			printf("BUS READING FROM 8041 MASTER retval=%x\n",retval);
			printf("retval=%x\n",retval);
			break;

		case 0xd : // read dip switches
			retval = ioport("DIPSW1")->read() ^ 0xff;  // all the switches from DIPSW1
			break;

		case 0xe : // read switches
			retval = ioport("ONLINE")->read() << 0 |
					ioport("PAPERERR")->read() << 1 |
					ioport("LINEFEED")->read() << 2 |
					ioport("FORMFEED")->read() << 3;
			break;
	}

	logerror("BUS R = %x %s    P20-23=%x\n", retval, machine().describe_context(), BIT(m_8049_p2, 0, 4));
	return retval;
}


// 583 MX80 rom 1
// 4fa   ok so cfa gets hit         1:611 call 4f6

void epson_mx80_device::port1_w(uint8_t data)
{
// p10 = online lamp
// p11 = PRAV'
// p12 = ERR'
// p13 = DIRAV'
// p14 = DIR'
// p15 = ACK
// p16 = BUSY
// p17 = autoLF

	m_8049_p1 = data;
	m_bitmap_printer->set_led_state(bitmap_printer_device::LED_ONLINE, BIT(data, 0));
//  m_bitmap_printer->set_led_state(bitmap_printer_device::LED_ONLINE, !BIT(data, 0));  yes! got the online switch working!  SHOWS that this is not inverted!
	update_busy();
	update_ack();
	logerror("P1 W %x %s\n", data, machine().describe_context());
}

void epson_mx80_device::port2_w(uint8_t data)
{
// p20-23 = select IO device  0x700 = 8155 0xb00 = 8041 0xd00 = dipsw1 0xe00 = switches
// p24 = bank switching
// p25 = IO / not M    or  A0
// p26 = head trigger
// p27 = err

	m_8049_p2 = data;

	m_bitmap_printer->set_led_state(bitmap_printer_device::LED_ERROR, !BIT(data, 7)); // CENTRONICS error

	// P26 = head trigger
	co0_w(BIT(data, 6));

	logerror("P2 W %x %s\n", data, machine().describe_context());
	//printf("P2W  p2 0-3=%x  p25=%x   p26(head)=%x  err=%x\n",BIT(m_8049_p2,0,4),BIT(m_8049_p2,5),BIT(m_8049_p2,6),BIT(m_8049_p2,7));

}


void epson_mx80dots_device::prog_w(int state)
{
	logerror ("PROG_W WRITE LINE  STATE = %x  %s\n", state, machine().describe_context());
	if ((m_prog_line == ASSERT_LINE) && (state == CLEAR_LINE)) // transition from high to low
	{
		logerror ("PROG EXISTING BANK %x  %s\n", m_dots_bank, machine().describe_context());
		m_dots_bank = BIT(m_8049_p2, 0, 2);
		logerror ("PROG SET BANK %x  %s\n", m_dots_bank, machine().describe_context());
	}
	m_prog_line = state;
}

void epson_mx80_device::bus_w(uint8_t data)
{
	logerror("BUS W %x %s\n", data, machine().describe_context());
}




// cases: been steady for a long time
// cases: starting up  been steady   steady for a long time, now it moved
// cases: moving quickly
// cases: slowing down
// going to have to track position and velocity, not just last time moved perhaps


void epson_mx80_device::trackcrpos()
{
	//int pos = m_bitmap_printer->m_xpos;
	//int pos = m_bitmap_printer->m_xpos;
	double posd = m_bitmap_printer->m_cr_stepper->get_absolute_position();
	if (lastmovecrposd != posd)  // if it's been moved
	{
		//double dist = pos - lastmovecrpos;
		double dist = posd - lastmovecrposd;
		double currenttime = machine().time().as_double();
		double elapsed = currenttime - lastmovecrtime;
		if (elapsed < .00001) elapsed = .00001;  // avoid divide by zero
		double speed = dist / elapsed;

		//lastmovecrpos = pos;
		lastmovecrposd = posd;
		lastmovecrtime = machine().time().as_double();
		lastmovecrspeed = speed;
		//if lastmovecrspeed  < mincrspeed) do something, we should have a minimum speed if there needs to be movement
	}
 }
 /*
double epson_mx80_device::calccrpos()
{
    double calcpos;
    if ((machine().time().as_double() - lastmovecrtime) < .2)
        //calcpos = lastmovecrpos + (machine().time().as_double() - lastmovecrtime) * lastmovecrspeed;
        calcpos = lastmovecrposd + (machine().time().as_double() - lastmovecrtime) * lastmovecrspeed;
    else
//      calcpos = m_xpos;
//      calcpos = m_bitmap_printer->m_xpos;
//      double posd = m_bitmap_printer->m_cr_stepper->get_absolute_position();
        calcpos = m_bitmap_printer->m_cr_stepper->get_absolute_position();
    return calcpos;
}
*/
// this version, let's set a minimum cr speed
// seconds per stepper unit of cr
// so if   minspeed = .02 = 20 ms per step
// so if the time is greater than that, we should be
// treat it as linear

double epson_mx80_device::calccrpos()
{
	double calcpos;
	if ((machine().time().as_double() - lastmovecrtime) < .2)  // value here is 200 milliseconds
		//calcpos = lastmovecrpos + (machine().time().as_double() - lastmovecrtime) * lastmovecrspeed;
		calcpos = lastmovecrposd + (machine().time().as_double() - lastmovecrtime) * lastmovecrspeed;
	else
//      calcpos = m_xpos;
//      calcpos = m_bitmap_printer->m_xpos;
//      double posd = m_bitmap_printer->m_cr_stepper->get_absolute_position();
		calcpos = m_bitmap_printer->m_cr_stepper->get_absolute_position();
	return calcpos;
}

/*
// converted to use tracckpos, not using the spring steppers, didn't work very well.

void  epson_mx80_device::co0_w (int state)
{

    if (!state)
    {
        [[maybe_unused]] static constexpr int pin_pattern[] = { 7, 6, 5, 4, 3, 2, 1, 0, 15 };

        s32 xpos = m_bitmap_printer->m_xpos;
        s32 ypos = m_bitmap_printer->m_ypos;

    //  s32 xpos2 = m_cr_stepper.m_position;
        s32 xpos2 = calccrpos();

        if (xpos < m_bitmap_printer->m_page_bitmap.width())
        {
            for (int i = 0; i < 9; i++)
            {

              if (!(m_printhead & (1 << pin_pattern[i])))  // pin pattern: 7 6 5 4 3 2 1 0 8
                    m_bitmap_printer->pix(
                        ypos + i * 1, // * 1 for no interleave at 72 vdpi
                        xpos2 ) = 0;
            }
        }
    }

}
*/



void  epson_mx80_device::co0_w (int state)
{
/*
u32 ribbon[] = {
                    0x000000, // black
                    0xff00ff, // magenta (invert g)
                    0x00ffff, // cyan    (invert r)
                    0xffff00, // yellow  (invert b)
                };

    s32 ribbon_index = (m_ribbon_stepper->get_absolute_position() / 60);
    ribbon_index = std::max(ribbon_index,0);
    ribbon_index = std::min(ribbon_index,3);
    u32 ribboncolor = ribbon[ribbon_index];
*/
	int rightward_offset = fix16(ioport("RIGHTOFFSET")->read());

	if (!state)
	{
		[[maybe_unused]] static constexpr int pin_pattern[] = { 7, 6, 5, 4, 3, 2, 1, 0, 15 };

		s32 xpos = m_bitmap_printer->m_xpos;
		s32 ypos = m_bitmap_printer->m_ypos;

		s32 xpos2 = m_cr_stepper.m_position;


	//  if (xpos2 < 0) xpos2 = 0;
	//  if (xpos2 > m_bitmap_printer->m_paper_width-1) xpos2 = m_bitmap_printer->m_paper_width - 1;


		if (xpos < m_bitmap_printer->m_page_bitmap.width())
		{
			for (int i = 0; i < 9; i++)
			{

			  if (!(m_printhead & (1 << pin_pattern[i])))  // pin pattern: 7 6 5 4 3 2 1 0 8
					m_bitmap_printer->pix(
						ypos + i * 1, // * 1 for no interleave at 72 vdpi
						xpos2  +
						(m_bitmap_printer->m_cr_direction > 0 ? rightward_offset : 0)  ) = 0;
//                      drawcmy (m_bitmap_printer->pix(ypos + i * 1, xpos), ribboncolor);


			}
		}
	}

}





u8 epson_mx80_device::home_sensor()
{
	return (!(m_bitmap_printer->m_xpos <= 50)) ^ ioport("INVERTHOME")->read();
//  return (m_bitmap_printer->m_xpos <= 0) ^ ioport("INVERTHOME")->read();
}

uint8_t epson_mx80dots_device::dots_perfect_prog_mem_r(offs_t offset)
{
	u8 *dots_ptr = memregion("dots_perfect")->base();
//  u8 retval = dots_ptr[offset | ((bitswap<2>(m_dots_bank,1,0)) * 0x1000)];
	u8 retval = dots_ptr[offset | ((m_dots_bank) * 0x1000)];
//  logerror("READ_DOTS_PERFECT %s with offset %x low=%x high=%x, memmap=%x retval=%x\n",  machine().describe_context(), offset, offset_low,offset_upper, dots_perfect_memory_map[offset_upper], retval);
	return retval;
}


//-------------------------------------------------
//  IO Sel
//-------------------------------------------------

/***************************************************************************
    Centronics
***************************************************************************/

void  epson_mx80_device::input_strobe (int state)
{
	logerror("CENTRONICS_INPUT_STROBE %s state = %x oldstate=%x\n", machine().describe_context(), state, m_centronics_strobe);
	if (m_centronics_strobe == true && state == false) {
		m_centronics_data_latch   = m_centronics_data;
		m_centronics_data_latched = true;
		update_busy();
		update_ack();
		//m_maincpu->set_input_line(UPD7810_INTF1, m_centronics_data_latched);  // it's rising edge
		m_maincpu->set_input_line(MCS48_INPUT_IRQ, m_centronics_data_latched);  // it's rising edge
		logerror("CENTRONICS_INPUT_STROBE R %s = %x\n", machine().describe_context(), m_centronics_data);
	}

	m_centronics_strobe = state;
}


void  epson_mx80_device::input_init (int state)
{
	if (( state == false ) && (m_centronics_init == true))
	{
		m_maincpu->reset();
		m_slavecpu->reset();
		device_reset();
//      m_real_cr_steps = 0;
//      m_cr_timer->adjust(attotime::never, m_bitmap_printer->m_cr_direction);
	}
	m_centronics_init = state;
}


uint8_t epson_mx80_device::pts_r() // print timing sensor
{
	double fracpart, intpart;
//  double multiplier = fix16(ioport("MULTIPLIER")->read()); // try 2.0
	double multiplier =  ioport("MULTIPLIER")->read(); // try 2.0
	double multiplier1 = ioport("MULTIPLIERDIV")->read(); // try 2.0
	double low  = ioport("PTS0")->read() / 100.0; // try 2.0
	double high = ioport("PTS1")->read() / 100.0; // try 2.0
//  fracpart = modf(m_cr_stepper.m_position * ((multiplier < 0) ? (multiplier1 / (-multiplier)) : (multiplier / multiplier1)), &intpart);
	fracpart = modf(m_cr_stepper.m_position * (multiplier / multiplier1), &intpart);
	return (fracpart > low && fracpart < high);
}

uint8_t epson_mx80_device::slave_r(offs_t offset)
{
		u8 const data = m_slavecpu->upi41_master_r(offset);
		logerror("slave_data_r (%x)=%02x (%s)\n", offset, data, machine().describe_context());
		return data;
}

void epson_mx80_device::slave_w(offs_t offset, uint8_t data)
{
	if (offset == 0)
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(epson_mx80_device::slave_write_data_sync), this), unsigned(data));
	else if (offset == 1)
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(epson_mx80_device::slave_write_command_sync), this), unsigned(data));
	logerror("SLAVE W (%x)=%x %s with offset %x %s\n", offset, data, machine().describe_context(), offset, offset ? "COMMAND" : "DATA");
}

TIMER_CALLBACK_MEMBER(epson_mx80_device::slave_write_data_sync)
{
	m_slavecpu->upi41_master_w(0U, u8(u32(param)));
}

TIMER_CALLBACK_MEMBER(epson_mx80_device::slave_write_command_sync)
{
	m_slavecpu->upi41_master_w(1U, u8(u32(param)));
}


} // anonymous namespace

// GLOBAL

DEFINE_DEVICE_TYPE_PRIVATE(EPSON_MX80, device_centronics_peripheral_interface, epson_mx80_device, "epson_mx80", "Epson MX-80 (with graftrax)")
DEFINE_DEVICE_TYPE_PRIVATE(EPSON_MX80_DOTS, device_centronics_peripheral_interface, epson_mx80dots_device, "epson_mx80_dots", "Epson MX-80 (with Dots Perfect Upgrade)")
DEFINE_DEVICE_TYPE_PRIVATE(EPSON_MX80_III, device_centronics_peripheral_interface, epson_mx80_iii_device, "epson_mx80_iii", "Epson MX-80 III")

