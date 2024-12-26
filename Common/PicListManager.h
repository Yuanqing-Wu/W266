#pragma once

#include <list>

struct Picture;
class Slice;
class SPS;
class PPS;
class VPS;
class ReferencePictureList;

typedef std::list<Picture*> PicList;

class PicListManager {
private:
    PicList                            m_cPicList;   //  Dynamic buffer

};