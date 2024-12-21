#pragma once

#include <memory>

#include "unit.h"
#include "picture.h"
#include "slice.h"

struct CtuData {
  // SAOBlkParam           saoParam;
  // CtuAlfData            alfParam;
  const Slice*          slice;
  const PPS*            pps;
  const SPS*            sps;
  const PicHeader*      ph;
  int                   lineIdx, colIdx, ctuIdx;

  CodingUnit           *firstCU, *lastCU;
  unsigned              numCUs, numTUs;

  ptrdiff_t             predBufOffset;

  CodingUnit**          cuPtr  [MAX_NUM_CHANNEL_TYPE];
};

class CodingStructure {
public:

  UnitArea         area;
  Picture         *picture;
  int              chromaQpAdj;
  std::shared_ptr<const SPS> sps;
  std::shared_ptr<const PPS> pps;
  std::shared_ptr<PicHeader> picHeader;

  // data for which memory is partially borrowed from DecLibRecon
  CtuData*          m_ctuData;
  size_t            m_ctuDataSize;

  Pel*              m_predBuf;
  // end of partially borrowed data

  PelStorage m_reco;
  PelStorage m_rec_wrap;

  unsigned int         m_widthInCtus;
  PosType              m_ctuSizeMask[2];
  PosType              m_ctuWidthLog2[2];

  CodingUnit**         m_cuMap;
  ptrdiff_t            m_cuMapSize;
};