#include <boost/tuple/tuple.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "b101e1ngu.hpp"
#include "udp_bus.hpp"
#include "boot_loader.hpp"


namespace insys { namespace board {

boost::tuple< std::string , unsigned short > 
parse_address(const std::string& netaddr) {
    std::size_t pos = netaddr.find(':');
    std::string host = netaddr.substr(0, pos);
    unsigned short port = (pos == std::string::npos ? 0 : 
                           boost::lexical_cast<unsigned short>(netaddr.substr(pos+1)));
    return boost::make_tuple(host, port);
}

struct b101e1ngu::board_impl {
    enum bus_type {
        HOST_BUS = 0x2,
        PROC_BUS = 0x3,
    
    };

    enum processor_addrs {
        PROCESSOR_BASE = 0x2000000,
        SYSCON	= PROCESSOR_BASE + 0x00180480,
        SDRCON	= PROCESSOR_BASE + 0x00180484,      // SDRAM CONFIG REG
        VIRPT   = PROCESSOR_BASE + 0x00180730,

    };

    static const bus::ibus::value_type rstsyscon = 0x000279E7;
    static const bus::ibus::value_type defsyscon = 0x0019E623;
    static const bus::ibus::value_type defsdrcon = 0x00002513;


    enum hostpld_addrs {
        HMODE   = 0x00000000,
        HSTATUS = 0x00000004,
        HMASK   = 0x0000000C,
        SEM0    = 0x0000001C,
        MSG_ADR = 0x00000080,
    };

    enum hmode_flags {
        HMODE_DMA0EN	= 0x01000000,		// DMA0 request enable (FIFO2 - read)
        HMODE_DMA1EN	= 0x02000000,		// DMA1 request enable (FIFO1 - write)
        HMODE_RESFIFO2	= 0x04000000,		// FIFO2 reset
        HMODE_RESFIFO1	= 0x08000000,		// FIFO1 reset
        HMODE_RESET		= 0x10000000,		// BOARD RESET
        HMODE_FLASH		= 0x20000000,		// BOARD FLASH LOAD 
        HMODE_ERR_CLR 	= 0x80000000		// reset for host error bit
    };

    enum hmask_flags {
        HMASK_MMSG8    = 0x00000001,		// MSG[8] interrupt enab,e
        HMASK_MMSG9    = 0x00000002,		// MSG[9] interrupt enab,e
        HMASK_MMSG10   = 0x00000004,		// MSG[10] interrupt enab,e
        HMASK_MMSG11   = 0x00000008,		// MSG[11] interrupt enab,e
        HMASK_MMSG12   = 0x00000010,		// MSG[12] interrupt enab,e
        HMASK_MMSG13   = 0x00000020,		// MSG[13] interrupt enab,e
        HMASK_MMSG14   = 0x00000040,		// MSG[14] interrupt enab,e
        HMASK_MMSG15   = 0x00000080,		// MSG[15] interrupt enab,e
        HMASK_M2EF     = 0x00000100,		// FIFO 2 (DSP->PC) empty interrupt enab,e
        HMASK_M2HF     = 0x00000200,		// FIFO 2 (DSP->PC) ha,f interrupt enab,e
        HMASK_M2FF     = 0x00000400,		// FIFO 2 (DSP->PC) fu,, interrupt enab,e
        HMASK_M1EF     = 0x00000800,		// FIFO 1 (PC->DSP) empty interrupt enab,e
        HMASK_M1HF     = 0x00001000,		// FIFO 1 (PC->DSP) ha,f interrupt enab,e
        HMASK_M1FF     = 0x00002000,		// FIFO 1 (PC->DSP) fu,, interrupt enab,e
        HMASK_M2ERROR  = 0x00004000,		// interrupt for error FIFO 2
        HMASK_M1ERROR  = 0x00008000,		// interrupt for error FIFO 1
        HMASK_MSEM0    = 0x00010000,		// interrupt for SEM0
        HMASK_MSEM1    = 0x00020000,		// interrupt for SEM1
        HMASK_MSEM2    = 0x00040000,		// interrupt for SEM2
        HMASK_MSEM3    = 0x00080000,		// interrupt for SEM3
        HMASK_MSEM4    = 0x00100000,		// interrupt for SEM4
        HMASK_MSEM5    = 0x00200000,		// interrupt for SEM5
        HMASK_MSEM6    = 0x00400000,		// interrupt for SEM6
        HMASK_MSEM7    = 0x00800000,		// interrupt for SEM7
        HMASK_M2PEF    = 0x01000000,		//
        HMASK_M2PFF    = 0x02000000,		//
        HMASK_M1PEF    = 0x04000000,		//
        HMASK_M1PFF    = 0x08000000,		//
        HMASK_MCNT_ERR = 0x80000000,		//
    };

    board_impl(const std::string& netaddr) {
        std::string host;
        unsigned short port;
        boost::tie(host, port) = parse_address(netaddr);
        
        pbus_.reset((port == 0 ?
                     new insys::bus::udp_bus(host) :
                     new insys::bus::udp_bus(host, port)));

    }

    void reset(bool flash_boot) {
        const bus::address hmode(HOST_BUS, HMODE);
        pbus_->write(hmode, 0);
        boost::this_thread::sleep(boost::posix_time::seconds(1));
        if (flash_boot)
            pbus_->write(hmode, HMODE_FLASH | HMODE_ERR_CLR | HMODE_RESET | HMODE_RESFIFO1 | HMODE_RESFIFO2);
        else
            pbus_->write(hmode, HMODE_ERR_CLR | HMODE_RESET | HMODE_RESFIFO1 | HMODE_RESFIFO2);
        pbus_->read(hmode);
        pbus_->read(bus::address(HOST_BUS, HSTATUS));

        const bus::address hmask(HOST_BUS, HMASK);
        pbus_->write(hmask, HMASK_MCNT_ERR);
        pbus_->write(hmask, 0);

        pbus_->write(bus::address(HOST_BUS, SEM0), 0);
        pbus_->write(hmode, flash_boot ? HMODE_FLASH : 0);

        const bus::address syscon(PROC_BUS, SYSCON);
        if (pbus_->read(syscon) != rstsyscon)
            throw board_error("b101e1ngu", "bad reset syscon register");
        pbus_->write(syscon, defsyscon);
        if(pbus_->read(syscon) != defsyscon)
            throw board_error("b101e1ngu", "bad new value of syscon register");

        pbus_->write(hmask, 0);
        
        pbus_->write(bus::address(PROC_BUS, SDRCON), defsdrcon);
        
        std::vector<bus::ibus::value_type> zero(64);
        pbus_->write(bus::address(HOST_BUS, MSG_ADR), zero.begin(), zero.end());

        pbus_->write(hmask, 1);
    }

    void start() {
        pbus_->write(bus::address(PROC_BUS, VIRPT), 0);
    }

    void load(const std::string& path, int argc, char* argv[]) {
        boot::load(bus::address(PROC_BUS, PROCESSOR_BASE), pbus_, path, argc, argv);
    }
private:
    boost::shared_ptr<bus::ibus> pbus_;
    
};

b101e1ngu::b101e1ngu(const std::string& netaddr) : pimpl_(new board_impl(netaddr)) {}
void b101e1ngu::reset(bool flash_boot){ pimpl_->reset(flash_boot); }
void b101e1ngu::start(){ pimpl_->start(); }
void b101e1ngu::load(const std::string& path, int argc, char* argv[]) { pimpl_->load(path, argc, argv); }

}} //namespace insys::board
