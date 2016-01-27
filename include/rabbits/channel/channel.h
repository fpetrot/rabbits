#ifndef _UTILS_CHANNEL_H
#define _UTILS_CHANNEL_H

template <class T> class channel_out_if : virtual public sc_core::sc_interface
{
    public:
        virtual void write(const T&) = 0;
    protected:
        channel_out_if() {
        };
    private:
        channel_out_if (const channel_out_if&); // disable copy
        channel_out_if& operator= (const channel_out_if&); // disable
};

template <class T> class channel_in_if : virtual public sc_core::sc_interface
{
    public:
        virtual void read(T&) = 0;
    protected:
        channel_in_if() {
        };
    private:
        channel_in_if(const channel_in_if&); // disable copy
        channel_in_if& operator= (const channel_in_if&); // disable
};



template <class T> class Channel : public sc_core::sc_prim_channel, public channel_out_if<T>, public channel_in_if<T>
{
};

class QemuConsoleChannel : public Channel<uint8_t>
{
    private:
        sc_core::sc_event ev1;

    public:
        QemuConsoleChannel() {
            // TODO: verify single instance
        }
    public:
        void write(const uint8_t &value) {
            putc(value, stdout);
            fflush(stdout);
        }

        void read(uint8_t &value) {
            wait(ev1); // TODO: notify ev1 when a character is available
        }
};

class ClockedByteChannel : public sc_core::sc_module, public Channel<uint8_t>
{   
    public:
        sc_core::sc_out<uint8_t> wdata;
        sc_core::sc_out<bool> wclk;

        sc_core::sc_in<uint8_t> rdata;
        sc_core::sc_in<bool> rclk;

        SC_HAS_PROCESS(ClockedByteChannel);
        ClockedByteChannel() : sc_core::sc_module(sc_core::sc_module_name(sc_core::sc_gen_unique_name("ClockedByteChannel"))) {
            SC_METHOD(r_thread); 
            sensitive << rclk;
        }

    public:
        void write(const uint8_t &value) {
            wdata = value;
            wclk = !wclk;
        }

        void read(uint8_t &value) {
            sc_core::sc_prim_channel::wait(ev1);
            value = rdata;
        }

    private:
        sc_core::sc_event ev1;
        
        void r_thread(void) {
            ev1.notify();
        }

        // SOLUTION 1: THREAD WAITING FOR EDGE ON 'rclk', which notify
        // SOLUTION 2: MatlabConnector to notify channel when value change ?
};

#include <components/i2c/dwapb/i2c.h>

class I2cMasterChannel : public Channel<i2c_packet> {
    public:
        I2cMasterChannel() {

        }

    public:
        void write(const i2c_packet &packet) {
            printf("I2cMasterChannel: write at 0x%03x, data=0x%02x\n", packet.addr, packet.data);
        }

        void read(i2c_packet &packet) {
            printf("I2cMasterChannel: read at 0x%03x\n", packet.addr);
            packet.data = 0xA0;
        }
};

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "rabbits/channel/ethernet.h"

class TapChannel : public Channel<EthernetFrame>
{
    private:
        int fd;

    public:
        TapChannel(std::string tap) {
            fd = -1;
            if(tap.empty()) {
                /*D*/printf("error, tun interface name cannot be empty\n");
                exit(1);
            }
            tap_open(tap);
        }

    public:
        void write(const EthernetFrame &frame) {
            if(fd < 0) {
                return;
            }

            ::write(fd, frame.get_buffer(), (int)frame.length());
        }

        void read(EthernetFrame &frame) {
            frame.set_length(0);

            if(fd < 0) {
                // TODO: show error
                return;
            }
#if 0
            static int prepared = 0;
            if(!prepared) {
                // set non-blocking
                int flags = fcntl(fd, F_GETFL, 0);
                fcntl(fd, F_SETFL, flags | O_NONBLOCK);
                prepared = 1;
            }
#endif
            int r = ::read(fd, frame.get_buffer(), (int)frame.capacity());
            
            // TODO: handle error
            
            if(r > 0) {
                if(r < 60) r = 60;
                frame.set_length(r);
            }
        }

    private:
        void tap_open(std::string tun) {
            struct ifreq ifr;

            if ((fd = open("/dev/net/tun", O_RDWR)) < 0) {
                perror("open");
                return;
            }

            memset(&ifr, 0, sizeof(ifr));
            ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
            strncpy(ifr.ifr_name, tun.c_str(), IFNAMSIZ);
            if(ioctl(fd, TUNSETIFF, (void *) &ifr) < 0 ) {
                /*E*/printf("%s: tun/tap device setup failed: %s\n", ifr.ifr_name, strerror(errno));
                close(fd);
                fd = -1;
                return;
            }

            /*D*/printf("TAP opened\n");
        }
};

#if 0
// TODO: TCPClientChannel and TCPServerChannel
class TCPChannel: public Channel<uint8_t>
{
    // ctor: connect
    // write: send
    // read: recv
}

class SerdesChannel: public Channel<uint8_t> {
    // SPISerdes, SerialSerdes, ... ?
    // params: baudrate ...
    // write/read from a second channel
    // write: serialize to signal
    // read: deserialize signal
}
#endif

// UART: send/recv bytes
// --> xterm
// --> qemu console
// --> serdes --> simulink
// --> TCP Server (SNPS Telnet) or Client (Another platform)

// GMAC: send/recv frames (pakets)
// --> tuntap

// TODO: SPI / I2C / GPIO / ADC / 

// TODO: different input / output types ? <input_type, output_type>

// 18/11/2015: Our models should be easily integrable in another project, so their output should be systemc compatible
// Si on sort les signaux physiques (TX, DTR; ...) if faut des sc_out<bool>. Sinon il faut monter en abstraction : sc_out<uint8_t> ou __sc_channel__ ?

#endif

