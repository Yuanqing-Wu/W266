#pragma once

#include "Common/PicListManager.h"
#include "DecLibParser.h"

class DecLib {
    PicListManager           m_picListManager;
    DecLibParser             m_decLibParser{ *this, m_picListManager };

public:
    void create();
    Picture* decode( InputNALUnit& nalu );
};