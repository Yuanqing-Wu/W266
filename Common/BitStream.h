#pragma once

#include <stdint.h>
#include <vector>
#include "Def.h"

class InputBitstream {
private:
    AlignedByteVec        m_fifo;
    uint32_t m_fifo_idx = 0;   /// Read index into m_fifo

    uint32_t m_num_held_bits = 0;
    uint64_t m_held_bits     = 0;

    bool m_zeroByteAdded = false;

public:

    const AlignedByteVec& getFifo() const { return m_fifo; }
          AlignedByteVec& getFifo()       { return m_fifo; }

    inline uint8_t  getNumBitsUntilByteAligned() const { return m_num_held_bits & ( 0x7 ); }
    inline uint32_t getNumBitsLeft()             const { return ( m_fifo_idx < m_fifo.size() ? 8 * ( (uint32_t) m_fifo.size() - m_fifo_idx ) : 0 ) + m_num_held_bits; }

    uint32_t       peekBits( uint32_t uiNumberOfBits );
    uint32_t       read    ( uint32_t uiNumberOfBits );

private:
    inline void load_next_bits( int requiredBits ) {
        uint32_t num_bytes_to_load = 8;
        if UNLIKELY( m_fifo_idx + num_bytes_to_load > m_fifo.size()   // end of bitstream
                    || ( m_fifo_idx & 0x7 ) != 0 )                   // unaligned read position (m_fifo should be aligned)
        {
            const int required_bytes = ( requiredBits + 7 ) >> 3;
            CHECK( m_fifo_idx + required_bytes > m_fifo.size(), "Exceeded FIFO size" );

            num_bytes_to_load = (uint32_t)m_fifo.size() - m_fifo_idx;

            m_held_bits = 0;
            switch( num_bytes_to_load )
            {
            default: num_bytes_to_load = 8;   // in the unaligned case num_bytes_to_load could be >8
            case 8:  m_held_bits =  static_cast<uint64_t>( m_fifo[m_fifo_idx++] ) << ( 7 * 8 );
            case 7:  m_held_bits |= static_cast<uint64_t>( m_fifo[m_fifo_idx++] ) << ( 6 * 8 );
            case 6:  m_held_bits |= static_cast<uint64_t>( m_fifo[m_fifo_idx++] ) << ( 5 * 8 );
            case 5:  m_held_bits |= static_cast<uint64_t>( m_fifo[m_fifo_idx++] ) << ( 4 * 8 );
            case 4:  m_held_bits |= static_cast<uint64_t>( m_fifo[m_fifo_idx++] ) << ( 3 * 8 );
            case 3:  m_held_bits |= static_cast<uint64_t>( m_fifo[m_fifo_idx++] ) << ( 2 * 8 );
            case 2:  m_held_bits |= static_cast<uint64_t>( m_fifo[m_fifo_idx++] ) << ( 1 * 8 );
            case 1:  m_held_bits |= static_cast<uint64_t>( m_fifo[m_fifo_idx++] );
            }
        }
        else
        {
            CHECKD( reinterpret_cast<intptr_t>( &m_fifo[m_fifo_idx] ) & 0x7, "bistream read pos unaligned" );
            m_held_bits = __builtin_bswap64( *reinterpret_cast<uint64_t*>( &m_fifo[m_fifo_idx] ) );
            m_fifo_idx += num_bytes_to_load;
        }

        m_num_held_bits = num_bytes_to_load * 8;
    }
};