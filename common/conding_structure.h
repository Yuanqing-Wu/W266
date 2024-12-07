#pragma once

#include <memory>

#include "unit.h"
#include "picture.h"



class CodingStructure {
public:

  UnitArea         area;
  Picture         *picture;
  int              chromaQpAdj;
  std::shared_ptr<const VPS> vps;
  std::shared_ptr<const SPS> sps;
  std::shared_ptr<const PPS> pps;
  std::shared_ptr<PicHeader> picHeader;
  std::shared_ptr<const APS> alfApss[ALF_CTB_MAX_NUM_APS];
  std::shared_ptr<const APS> lmcsAps;
  const PreCalcValues*       pcv;

  // data for which memory is partially borrowed from DecLibRecon
  CtuData*          m_ctuData;
  size_t            m_ctuDataSize;

  Pel*              m_predBuf;
  Mv*               m_dmvrMvCache;
  // end of partially borrowed data
  
  CodingStructure( CUChunkCache* cuChunkCache, TUChunkCache* tuChunkCache );

  void create(const UnitArea &_unit);
  void create(const ChromaFormat &_chromaFormat, const Area& _area);
  void destroy();

  void resetForUse();
  void rebindPicBufs();

  // ---------------------------------------------------------------------------
  // global accessors
  // ---------------------------------------------------------------------------

#if _DEBUG
  const CodingUnit*     getCU(Position pos, ChannelType _chType) const
  {
    if( area.blocks[_chType].contains( pos ) )
    {
      int rsAddr = ctuRsAddr( pos, _chType );
      int inCtu  = inCtuPos ( pos, _chType );
      return getCtuData( rsAddr ).cuPtr[_chType][inCtu];
    }
    else return nullptr;
  }

  CodingUnit*     getCU(Position pos, ChannelType _chType)
  {
    if( area.blocks[_chType].contains( pos ) )
    {
      int rsAddr = ctuRsAddr( pos, _chType );
      int inCtu  = inCtuPos ( pos, _chType );
      return getCtuData( rsAddr ).cuPtr[_chType][inCtu];
    }
    else return nullptr;
  }
#else
  const CodingUnit*     getCU(Position pos, ChannelType _chType) const { if( area.blocks[_chType].contains( pos ) ) return getCtuData( ctuRsAddr( pos, _chType ) ).cuPtr[_chType][inCtuPos( pos, _chType )]; else return nullptr; }
        CodingUnit*     getCU(Position pos, ChannelType _chType)       { if( area.blocks[_chType].contains( pos ) ) return getCtuData( ctuRsAddr( pos, _chType ) ).cuPtr[_chType][inCtuPos( pos, _chType )]; else return nullptr; }
#endif

  const CodingUnit*     getCURestricted(const Position &pos, const Position curPos, const unsigned curSliceIdx, const unsigned curTileIdx, const ChannelType _chType) const;
  const CodingUnit*     getCURestricted(const Position &pos, const CodingUnit& curCu,                                                      const ChannelType _chType, const CodingUnit* guess = nullptr) const;

  CodingUnit&     addCU(const UnitArea &unit, const ChannelType _chType, const TreeType treeType, const ModeType modeType, const CodingUnit* cuLeft, const CodingUnit* cuAbove );
  TransformUnit&  addTU(const UnitArea &unit, const ChannelType _chType, CodingUnit &cu);
  void            addEmptyTUs(Partitioner &partitioner, CodingUnit& cu);
  CUTraverser     traverseCUs(const int ctuRsAddr);

  void initStructData();

  void allocTempInternals();
  void deallocTempInternals();

  void createInternals(const UnitArea& _unit);

  CUCache    m_cuCache;
  TUCache    m_tuCache;

  PelStorage m_reco;
  PelStorage m_rec_wrap;

  unsigned int         m_widthInCtus;
  PosType              m_ctuSizeMask[2];
  PosType              m_ctuWidthLog2[2];

  CodingUnit**         m_cuMap;
  ptrdiff_t            m_cuMapSize;
  ColocatedMotionInfo* m_colMiMap;
  ptrdiff_t            m_colMiMapSize;

public:

  // in CTU coordinates
  int ctuRsAddr( int col, int line ) const { return col + ( line * m_widthInCtus ); }
  // in sample coordinates
  int ctuRsAddr( Position pos, ChannelType chType ) const { Position posL = recalcPosition( area.chromaFormat, chType, CH_L, pos ); return ctuRsAddr( posL.x >> pcv->maxCUWidthLog2, posL.y >> pcv->maxCUHeightLog2 ); }
  // 4x4 luma block position within the CTU
  int inCtuPos ( Position pos, ChannelType chType ) const { return ( unitScale[chType].scaleHor( pos.x ) & m_ctuSizeMask[chType] ) + ( ( unitScale[chType].scaleVer( pos.y ) & m_ctuSizeMask[chType] ) << m_ctuWidthLog2[chType] ); }
  // 8x8 luma block position within the CTU
  int colMotPos( Position pos )                     const { return ( g_colMiScaling.scaleHor( pos.x ) & ( m_ctuSizeMask[CH_L] >> 1 ) ) + ( ( g_colMiScaling.scaleVer( pos.y ) & ( m_ctuSizeMask[CH_L] >> 1 ) ) << ( m_ctuWidthLog2[CH_L] - 1 ) ); }

        CtuData& getCtuData( int col, int line )       { return m_ctuData[ctuRsAddr( col, line )]; }
  const CtuData& getCtuData( int col, int line ) const { return m_ctuData[ctuRsAddr( col, line )]; }

        CtuData& getCtuData( int addr )       { return m_ctuData[addr]; }
  const CtuData& getCtuData( int addr ) const { return m_ctuData[addr]; }

  int m_IBCBufferWidth;
  std::vector<PelStorage> m_virtualIBCbuf;
  std::vector<char>       hasIbcBlock;
  void initVIbcBuf( int numCtuLines, ChromaFormat chromaFormatIDC, int ctuSize );
  void fillIBCbuffer( CodingUnit &cu, int lineIdx );

  MotionBuf getMotionBuf( const     Area& _area );
  MotionBuf getMotionBuf( const UnitArea& _area ) { return getMotionBuf( _area.Y() ); }

  const CMotionBuf getMotionBuf( const     Area& _area ) const;
  const CMotionBuf getMotionBuf( const UnitArea& _area ) const { return getMotionBuf( _area.Y() ); }

        MotionInfo& getMotionInfo( const Position& pos )       { return getCtuData( ctuRsAddr( pos, CH_L ) ).motion[inCtuPos( pos, CH_L )]; }
  const MotionInfo& getMotionInfo( const Position& pos ) const { return getCtuData( ctuRsAddr( pos, CH_L ) ).motion[inCtuPos( pos, CH_L )]; }
  

  const ColocatedMotionInfo& getColInfo( const Position &pos, const Slice*& pColSlice ) const;

  LoopFilterParam const* getLFPMapPtr   ( const DeblockEdgeDir edgeDir, ptrdiff_t _ctuRsAddr ) const { return m_ctuData[_ctuRsAddr].lfParam[edgeDir]; }
  LoopFilterParam      * getLFPMapPtr   ( const DeblockEdgeDir edgeDir, ptrdiff_t _ctuRsAddr )       { return m_ctuData[_ctuRsAddr].lfParam[edgeDir]; }
  ptrdiff_t              get4x4MapStride() const { return ( ptrdiff_t( 1 ) << m_ctuWidthLog2[CH_L] ); }

  UnitScale getScaling( const UnitScale::ScaliningType type, const ChannelType chType = CH_L ) const
  {
    return type == UnitScale::MI_MAP ? g_miScaling : unitScale[chType];
  }

public:

         PelBuf       getRecoBuf(const CompArea &blk)        { return m_reco.bufs[blk.compID()].subBuf( blk ); }
  const CPelBuf       getRecoBuf(const CompArea &blk) const  { return m_reco.bufs[blk.compID()].subBuf( blk ); }
         PelUnitBuf   getRecoBuf(const UnitArea &unit)       { return m_reco.subBuf( unit ); }
  const CPelUnitBuf   getRecoBuf(const UnitArea &unit) const { return m_reco.subBuf( unit ); }



  // reco buffer
         PelBuf       getRecoBuf(const ComponentID compID, bool wrap=false)         { return wrap ? m_rec_wrap.get(compID) : m_reco.get(compID); }
  const CPelBuf       getRecoBuf(const ComponentID compID, bool wrap=false)   const { return wrap ? m_rec_wrap.get(compID) : m_reco.get(compID); }
         PelUnitBuf   getRecoBuf(bool wrap=false)                                   { return wrap ? m_rec_wrap : m_reco; }
  const CPelUnitBuf   getRecoBuf(bool wrap=false)                             const { return wrap ? m_rec_wrap : m_reco; }

         PelUnitBuf   getPredBuf(const CodingUnit &cu);
  const CPelUnitBuf   getPredBuf(const CodingUnit &cu) const;
};