#include "BitStream.h"

uint32_t InputBitstream::peekBits( uint32_t uiNumberOfBits ) {
    auto saved_fifo_idx      = m_fifo_idx;
    auto saved_num_held_bits = m_num_held_bits;
    auto saved_held_bits     = m_held_bits;

    uint32_t num_bits_to_read = std::min(uiNumberOfBits, getNumBitsLeft());
    uint32_t uiBits           = read( num_bits_to_read );
    uiBits <<= (uiNumberOfBits - num_bits_to_read);

    m_fifo_idx      = saved_fifo_idx;
    m_num_held_bits = saved_num_held_bits;
    m_held_bits     = saved_held_bits;

    return uiBits;
}

uint32_t InputBitstream::read (uint32_t uiNumberOfBits) {
    constexpr static uint64_t ONES = ~static_cast<uint64_t>( 0 );   // need to ensure 64 bits for the mask, because shift by 32 is UB for uin32_t

    /* NB, bits are extracted from the MSB of each byte. */
    uint32_t retval = 0;
    if (uiNumberOfBits <= m_num_held_bits)
    {
        /* n=1, len(H)=7:   -VHH HHHH, shift_down=6, mask=0xfe
        * n=3, len(H)=7:   -VVV HHHH, shift_down=4, mask=0xf8
        */
        retval = static_cast<uint32_t>( m_held_bits >> ( m_num_held_bits - uiNumberOfBits ) );
        retval &= ~( ONES << uiNumberOfBits );
        m_num_held_bits -= uiNumberOfBits;

        return retval;
    }

    CHECK( uiNumberOfBits > 32, "Too many bits read" );

    if( m_num_held_bits )
    {
        /* all num_held_bits will go into retval
        *   => need to mask leftover bits from previous extractions
        *   => align retval with top of extracted word */
        /* n=5, len(H)=3: ---- -VVV, mask=0x07, shift_up=5-3=2,
        * n=9, len(H)=3: ---- -VVV, mask=0x07, shift_up=9-3=6 */
        uiNumberOfBits -= m_num_held_bits;
        retval = static_cast<uint32_t>( m_held_bits ) & ~( ONES << m_num_held_bits );   // we can cast to 32 bits, because the held bits are the rightmost bits
        retval <<= uiNumberOfBits;
    }

    /* number of whole bytes that need to be loaded to form retval */
    /* n=32, len(H)=0, load 4bytes, shift_down=0
    * n=32, len(H)=1, load 4bytes, shift_down=1
    * n=31, len(H)=1, load 4bytes, shift_down=1+1
    * n=8,  len(H)=0, load 1byte,  shift_down=0
    * n=8,  len(H)=3, load 1byte,  shift_down=3
    * n=5,  len(H)=1, load 1byte,  shift_down=1+3
    */
    load_next_bits( uiNumberOfBits );

    /* resolve remainder bits */
    m_num_held_bits -= uiNumberOfBits;

    /* copy required part of m_held_bits into retval */
    retval |= static_cast<uint32_t>( m_held_bits >> m_num_held_bits );

    return retval;
}