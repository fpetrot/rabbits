#ifndef _ETHERNET_H
#define _ETHERNET_H

#include <sys/types.h>
#include <inttypes.h>

class EthernetFrame {

public:
	EthernetFrame(size_t capacity);
	~EthernetFrame();

public:
	size_t capacity() const { return m_capacity; }
	size_t length  () const { return m_length; }

	uint8_t * get_buffer() const { return m_buffer; }

	void set_length(size_t length);

private:
	uint8_t *m_buffer;
	size_t   m_length;
	size_t   m_capacity;

private:
	EthernetFrame(const EthernetFrame&);
	EthernetFrame& operator=(const EthernetFrame&);
};

#endif
