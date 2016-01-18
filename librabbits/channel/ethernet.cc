#include "rabbits/channel/ethernet.h"
#include <cstdlib>
#include <inttypes.h>

#include <stdio.h>
#include <string.h>

EthernetFrame::EthernetFrame(size_t capacity)
:m_buffer  (NULL)
,m_length  (0)
,m_capacity(capacity)
{
    m_buffer = new uint8_t[capacity];
}

EthernetFrame::~EthernetFrame() {
    if (m_buffer) {
        delete[] m_buffer;
    }
}

void EthernetFrame::set_length(size_t length) {
    if (length > m_capacity) {
        printf("warning: ethernet frame is too large\n");
        return;
    }
    m_length = length;
}
