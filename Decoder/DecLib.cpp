#include "DecLib.h"
#include "Common/Picture.h"

void DecLib::create() {
    // run constructor again to ensure all variables, especially in DecLibParser have been reset
    this->~DecLib();
    new(this) DecLib;
}

Picture* DecLib::decode( InputNALUnit& nalu ) {
    bool newPic = m_decLibParser.parse( nalu );
    Picture *pic = new Picture;
    return pic;
}