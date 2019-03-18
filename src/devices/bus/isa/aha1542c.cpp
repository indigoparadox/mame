// license:BSD-3-Clause
// copyright-holders:Darkstar
/**********************************************************************
 *
 *    Adaptec AHA-1542{C,CF,CP} SCSI Controller
 *
 **********************************************************************/

/*
 PCB layout
 ----------
           Floppy            SCSI
       +-----------+ +------------------+  +-
 +-----+-----------+-+------------------+--+
 |                                         |
 | DIPSW DS1                               |\
 |     +-----+                             | |
 |     |     | Y1   +---+   TRM     TRM    | |
 |     | U3  |      |U5 |                  |S|
 | U8  +-----+      +---+                  |C|
 |       +---+ +------+     +-------+      |S|
 |       |U13| |U15   |  Y2 |       |      |I|
 |       +---+ +------+     |       |      | |
 |             |U16   |     |U17    |      | |
 |             +------+     +-------+      |/
 +-----------------------------------------+
 |||||||||  |||||||||||||||||||||||||      |


 DIPSW  sw1 - sw8
 U3 Intel chip labelled "AHA-1542CF/552800-01 D/9346", probably FDC (82077)
 U5 Z84C0010VEC
 U8 EEPROM(?) labelled 545120A
 U13    CXK5864CM-10LL (64kbit SRAM)
 U15    M27C256B labelled "ADAPTEC INC/553801-00 C/MCODE 563D/(C) 1993"
 U16    M27C256B labelled "ADAPTEC INC/553601-00 C/BIOS C38D/(C) 1993"
 U17    AIC-7970Q
 Y1 XTAL SRX4054 93-38
 Y2 XTAL SRX4053 93-40
 TRM    Dallas DS2107AS (SCSI termination)
 DS1    LED

*/

/*
 * The PCB has a couple of DIP switches:
 *
 *  sw1 on  enable termination
 *      off software-controlled termination
 *
 *  sw2 sw3 sw4 I/O Port
 *  off off off 0x330 - 0x333 (default)
 *  on  off off 0x334 - 0x337
 *  off on  off 0x230 - 0x233
 *  on  on  off 0x234 - 0x237
 *  off off on  0x130 - 0x133
 *  on  off on  0x134 - 0x137
 *  off on  on  reserved
 *  on  on  on  reserved
 *
 *  sw5 on  disable floppy interface
 *      off enable floppy interface
 *
 *  sw6 sw7 sw8 BIOS base address
 *  off off off 0xdc000 (default)
 *  on  off off 0xd8000
 *  off on  off 0xd4000
 *  on  on  off 0xd0000
 *  off off on  0xcc000
 *  on  off on  0xc8000
 *  off on  on  reserved
 *  on  on  on  BIOS disabled
 *
 * source: http://download.adaptec.com/pdfs/installation_guides/1540cfig.pdf
 */

#include "emu.h"
#include "aha1542c.h"
#include "cpu/z80/z80.h"

// I/O Port interface
// READ  Port x+0: STATUS
// WRITE Port x+0: CONTROL
//
// READ  Port x+1: DATA
// WRITE Port x+1: COMMAND
//
// READ  Port x+2: INTERRUPT STATUS
// WRITE Port x+2: (undefined?)
//
// R/W   Port x+3: (undefined)

// READ STATUS flags
#define STAT_STST   0x80    // self-test in progress
#define STAT_DIAGF  0x40    // internal diagnostic failure
#define STAT_INIT   0x20    // mailbox initialization required
#define STAT_IDLE   0x10    // HBA is idle
#define STAT_CDFULL 0x08    // Command/Data output port is full
#define STAT_DFULL  0x04    // Data input port is full
#define STAT_INVCMD 0x01    // Invalid command

// READ INTERRUPT STATUS flags
#define INTR_ANY    0x80    // any interrupt
#define INTR_SRCD   0x08    // SCSI reset detected
#define INTR_HACC   0x04    // HA command complete
#define INTR_MBOA   0x02    // MBO empty
#define INTR_MBIF   0x01    // MBI full

// WRITE CONTROL commands
#define CTRL_HRST   0x80    // Hard reset
#define CTRL_SRST   0x40    // Soft reset
#define CTRL_IRST   0x20    // interrupt reset
#define CTRL_SCRST  0x10    // SCSI bus reset

// READ/WRITE DATA commands
#define CMD_NOP         0x00    // No operation
#define CMD_MBINIT      0x01    // mailbox initialization
#define CMD_START_SCSI  0x02    // Start SCSI command
#define CMD_BIOSCMD     0x03    // undocumented BIOS conmmand (shadow RAM etc.)
#define CMD_INQUIRY     0x04    // Adapter inquiry
#define CMD_EMBOI       0x05    // enable Mailbox Out Interrupt
#define CMD_SELTIMEOUT  0x06    // Set SEL timeout
#define CMD_BUSON_TIME  0x07    // set bus-On time
#define CMD_BUSOFF_TIME 0x08    // set bus-off time
#define CMD_DMASPEED    0x09    // set ISA DMA speed
#define CMD_RETDEVS     0x0a    // return installed devices
#define CMD_RETCONF     0x0b    // return configuration data
#define CMD_TARGET      0x0c    // set HBA to target mode
#define CMD_RETSETUP    0x0d    // return setup data
#define CMD_ECHO        0x1f    // ECHO command data (NetBSD says it is 0x1e)

// these are for 1541C only:
#define CMD_RETDEVSHI   0x23    // return devices 8-15 (from NetBSD)
#define CMD_EXTBIOS     0x28    // return extended BIOS information
#define CMD_MBENABLE    0x29    // set mailbox interface enable

DEFINE_DEVICE_TYPE(AHA1542C, aha1542c_device, "aha1542c", "AHA-1542C SCSI Controller")
DEFINE_DEVICE_TYPE(AHA1542CF, aha1542cf_device, "aha1542cf", "AHA-1542CF SCSI Controller")
DEFINE_DEVICE_TYPE(AHA1542CP, aha1542cp_device, "aha1542cp", "AHA-1542CP SCSI Controller")

#define Z84C0010_TAG "z84c0010"

READ8_MEMBER( aha1542c_device::aha1542_r )
{
	logerror("%s aha1542_r(): offset=%d\n", machine().describe_context(), offset);
	return 0xff;
}

WRITE8_MEMBER( aha1542c_device::aha1542_w )
{
	logerror("%s aha1542_w(): offset=%d data=0x%02x\n", machine().describe_context(), offset, data);
}


ROM_START( aha1542c )
	ROM_REGION( 0x8000, "aha1542", 0 )
	ROM_LOAD( "534201-00_d_bios_144c.u15", 0x0000, 0x8000, CRC(35178004) SHA1(2b38f2e40cd02a1b32966ead7b202b0fca130cb8) )

	ROM_REGION( 0x8000, Z84C0010_TAG, 0 )
	ROM_LOAD( "534001-00_d_mcode_a3c2.u5", 0x0000, 0x8000, CRC(220dd5a2) SHA1(4fc51c9dd63b45a50edcd56baa706d61decbef38) )
ROM_END

ROM_START( aha1542cf )
	ROM_REGION( 0x8000, "aha1542", 0 )
	ROM_SYSTEM_BIOS( 0, "v201", "Adaptec 1540CF/1542CF BIOS v2.01" )
	ROMX_LOAD( "553601-00_c_bios_c38d.u16", 0x0000, 0x8000, CRC(ab22fc02) SHA1(f9f783e0272fc14ba3de32316997f1f6cadc67d0), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v211", "Adaptec 1540CF/1542CF BIOS v2.11" )
	ROMX_LOAD( "aha1542cf-v2.11-lower-socket.bin", 0x0000, 0x8000, CRC(fddd0b83) SHA1(aabd227cb338d8812e0bb5c17c08ea06c5aedd36), ROM_BIOS(1) )

	ROM_REGION( 0x8000, Z84C0010_TAG, 0 )
	ROMX_LOAD( "553801-00_c_mcode_563d.u15", 0x0000, 0x8000, CRC(7824397e) SHA1(35bc2c8fab31aad3190a478f2dc8f3a72958cf04), ROM_BIOS(0) )
	ROMX_LOAD( "aha1542cf-v2.11-upper-socket.bin", 0x0000, 0x8000, CRC(896873cd) SHA1(6edbdd9b0b15ef31ca0741cac31556d2d5266b6e), ROM_BIOS(1) )
ROM_END

ROM_START( aha1542cp )
	ROM_REGION( 0x8000, "aha1542", 0 )
	ROM_LOAD( "908501-00_d_bios_a91e.u7", 0x0000, 0x8000, CRC(0646c35e) SHA1(3a7c2731abd8295438cfa1f2a525be53e9512b1a) )

	ROM_REGION( 0x8000, Z84C0010_TAG, 0 )
	ROM_LOAD( "908301-00_f_mcode_17c9.u12", 0x0000, 0x8000, CRC(04494022) SHA1(431dfc26312556ddd24fccc429b2b3e93bac5c2f) )
ROM_END


void aha1542c_device::z84c0010_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom().region(Z84C0010_TAG, 0);
	map(0x8000, 0x800f).noprw();        // something is mapped there
	map(0x9000, 0xafff).ram();        // 2kb RAM chip
	map(0xe000, 0xe0ff).ram();        // probably PC<->Z80 communication area
	map(0xb000, 0xb000).noprw();        // something?
}

const tiny_rom_entry *aha1542c_device::device_rom_region() const
{
	return ROM_NAME( aha1542c );
}

const tiny_rom_entry *aha1542cf_device::device_rom_region() const
{
	return ROM_NAME( aha1542cf );
}

const tiny_rom_entry *aha1542cp_device::device_rom_region() const
{
	return ROM_NAME( aha1542cp );
}

void aha1542c_device::device_add_mconfig(machine_config &config)
{
	z80_device &cpu(Z80(config, Z84C0010_TAG, 10'000'000));
	cpu.set_addrmap(AS_PROGRAM, &aha1542c_device::z84c0010_mem);
}

aha1542c_device::aha1542c_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_isa16_card_interface(mconfig, *this)
{
}

aha1542c_device::aha1542c_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	aha1542c_device(mconfig, AHA1542C, tag, owner, clock)
{
}

aha1542cf_device::aha1542cf_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	aha1542c_device(mconfig, AHA1542CF, tag, owner, clock)
{
}

aha1542cp_device::aha1542cp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	aha1542c_device(mconfig, AHA1542CP, tag, owner, clock)
{
}

void aha1542c_device::device_start()
{
	set_isa_device();
	m_isa->install_rom(this, 0xdc000, 0xdffff, "aha1542", "aha1542");
	m_isa->install_device(0x330, 0x333, read8_delegate(FUNC( aha1542cf_device::aha1542_r ), this),
	write8_delegate(FUNC( aha1542cf_device::aha1542_w ), this) );
}


void aha1542c_device::device_reset()
{
}