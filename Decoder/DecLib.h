#pragma once

#include "PicListManager.h"
#include "DecLibParser.h"

class DecLib {
    PicListManager           m_picListManager;
    DecLibParser             m_decLibParser{ *this, m_picListManager };

public:
    void create();
};