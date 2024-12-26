#pragma once

#include "Common/Def.h"
#include "Common/PicListManager.h"

class DecLib;

class DecLibParser {
private:
    NalUnitType m_associatedIRAPType;   ///< NAL unit type of the associated IRAP picture
    int         m_pocCRA;   ///< POC number of the latest CRA picture
    int         m_pocRandomAccess      = MAX_INT;       ///< POC number of the random access point (the first IDR or CRA picture)
    int         m_decodingOrderCounter = 0;

    int m_prevPOC                   = MAX_INT;
    int m_prevTid0POC               = 0;

    DecLib&                   m_decLib;
    PicListManager&           m_picListManager;

    public:
    DecLibParser( DecLib& decLib, PicListManager& picListManager ) : m_decLib( decLib ), m_picListManager( picListManager ) {}
};