#include <cstring>

#include "slice.h"

ReferencePictureList::ReferencePictureList() {
    ::memset(this, 0, sizeof(*this));
}

void ReferencePictureList::clear() {
    ::memset(this, 0, sizeof(*this));
}

void ReferencePictureList::setRefPicIdentifier(int idx, int identifier, bool isLongterm, bool isInterLayerRefPic, int interLayerIdx) {
    CHECK(idx > MAX_NUM_REF_PICS, "RPL setRefPicIdentifier out of range (0-15)");
    m_refPicIdentifier[idx] = identifier;
    m_isLongtermRefPic[idx] = isLongterm;

    m_deltaPocMSBPresentFlag[idx] = false;
    m_deltaPOCMSBCycleLT[idx] = 0;

    m_isInterLayerRefPic[idx] = isInterLayerRefPic;
    m_interLayerRefPicIdx[idx] = interLayerIdx;
}

int ReferencePictureList::getRefPicIdentifier(int idx) const {
    return m_refPicIdentifier[idx];
}

bool ReferencePictureList::isRefPicLongterm(int idx) const {
    return m_isLongtermRefPic[idx];
}

void ReferencePictureList::setRefPicLongterm(int idx,bool isLongterm) {
    CHECK(idx > MAX_NUM_REF_PICS, "RPL setRefPicLongterm out of range (0-15)");
    m_isLongtermRefPic[idx] = isLongterm;
}

void ReferencePictureList::setNumberOfShorttermPictures(int numberOfStrp) {
    m_numberOfShorttermPictures = numberOfStrp;
}

int ReferencePictureList::getNumberOfShorttermPictures() const {
    return m_numberOfShorttermPictures;
}

void ReferencePictureList::setNumberOfLongtermPictures(int numberOfLtrp) {
    m_numberOfLongtermPictures = numberOfLtrp;
}

int ReferencePictureList::getNumberOfLongtermPictures() const {
    return m_numberOfLongtermPictures;
}

void ReferencePictureList::setPOC(int idx, int POC) {
    CHECK(idx > MAX_NUM_REF_PICS, "RPL setPOC out of range (0-15)");
    m_POC[idx] = POC;
}

int ReferencePictureList::getPOC(int idx) const {
    return m_POC[idx];
}

void ReferencePictureList::setDeltaPocMSBCycleLT(int idx, int x) {
    CHECK(idx > MAX_NUM_REF_PICS, "RPL setDeltaPocMSBCycleLT out of range (0-15)");
    m_deltaPOCMSBCycleLT[idx] = x;
}

void ReferencePictureList::setDeltaPocMSBPresentFlag(int idx, bool x) {
    CHECK(idx > MAX_NUM_REF_PICS, "RPL setDeltaPocMSBPresentFlag out of range (0-15)");
    m_deltaPocMSBPresentFlag[idx] = x;
}

void ReferencePictureList::setInterLayerRefPicIdx(int idx, int layerIdc) {
    CHECK(idx > MAX_NUM_REF_PICS, "RPL setInterLayerRefPicIdx out of range (0-15)");
    m_interLayerRefPicIdx[idx] = layerIdc;
}

bool ReferencePictureList::findInRefPicList(const Picture* checkRefPic, int currPicPoc, int layerId) const {
    return 0;
}

int ReferencePictureList::calcLTRefPOC(int currPoc, int bitsForPoc, int refPicIdentifier, bool pocMSBPresent, int deltaPocMSBCycle) {
    return 0;
}

int ReferencePictureList::calcLTRefPOC(int currPoc, int bitsForPoc, int refPicIdx) const {
    return 0;
}

ScalingList::ScalingList() {

}

void ScalingList::reset() {
    
}