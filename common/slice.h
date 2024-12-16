#pragma once

#include <memory>
#include <vector>
#include <array>

#include "def.h"
#include "picture.h"
#include "rom.h"

using PartitionConstraints = std::array<unsigned, 3>;


struct Picture;
template<class T>
struct BasePS: public std::enable_shared_from_this<T> {
    std::shared_ptr<T> getSharedPtr() {
        return (static_cast<T*>(this))->shared_from_this();
    }
    std::shared_ptr<const T> getSharedPtr() const {
        return (static_cast<const T*>(this))->shared_from_this();
    }

    void clearChangedFlag() {
        m_changedFlag = false;
    }

    bool getChangedFlag() const {
        return m_changedFlag;
    }

    bool             m_changedFlag = false;
    template<class Tf, int MAX_ID> friend class ParameterSetMap;
};

class Window {
private:
    bool m_enabledFlag     = false;
    int  m_winLeftOffset   = 0;
    int  m_winRightOffset  = 0;
    int  m_winTopOffset    = 0;
    int  m_winBottomOffset = 0;

public:
    bool getWindowEnabledFlag() const   { return m_enabledFlag;                          }
    int  getWindowLeftOffset() const    { return m_enabledFlag ? m_winLeftOffset : 0;    }
    void setWindowLeftOffset(int val)   { m_winLeftOffset = val; m_enabledFlag = true;   }
    int  getWindowRightOffset() const   { return m_enabledFlag ? m_winRightOffset : 0;   }
    void setWindowRightOffset(int val)  { m_winRightOffset = val; m_enabledFlag = true;  }
    int  getWindowTopOffset() const     { return m_enabledFlag ? m_winTopOffset : 0;     }
    void setWindowTopOffset(int val)    { m_winTopOffset = val; m_enabledFlag = true;    }
    int  getWindowBottomOffset() const  { return m_enabledFlag ? m_winBottomOffset: 0;   }
    void setWindowBottomOffset(int val) { m_winBottomOffset = val; m_enabledFlag = true; }

    void setWindow(int offsetLeft, int offsetLRight, int offsetLTop, int offsetLBottom) {
        m_enabledFlag     = true;
        m_winLeftOffset   = offsetLeft;
        m_winRightOffset  = offsetLRight;
        m_winTopOffset    = offsetLTop;
        m_winBottomOffset = offsetLBottom;
    }
};

class ReferencePictureList {
private:
    int   m_numberOfShorttermPictures                = 0;
    int   m_numberOfLongtermPictures                 = 0;
    int   m_isLongtermRefPic      [MAX_NUM_REF_PICS] = { 0 };
    int   m_refPicIdentifier      [MAX_NUM_REF_PICS] = { 0 };   // This can be delta POC for STRP or POC LSB for LTRP
    int   m_POC                   [MAX_NUM_REF_PICS] = { 0 };
    int   m_deltaPOCMSBCycleLT    [MAX_NUM_REF_PICS] = { 0 };
    bool  m_deltaPocMSBPresentFlag[MAX_NUM_REF_PICS] = { 0 };
    bool  m_ltrp_in_slice_header_flag                = false;
    bool  m_interLayerPresentFlag                    = false;
    bool  m_isInterLayerRefPic [MAX_NUM_REF_PICS]    = { 0 };
    int   m_interLayerRefPicIdx[MAX_NUM_REF_PICS]    = { 0 };
    int   m_numberOfInterLayerPictures               = 0;

public:
    ReferencePictureList();
    void clear();

    void setRefPicIdentifier( int idx, int identifier, bool isLongterm, bool isInterLayerRefPic, int interLayerIdx );
    int  getRefPicIdentifier( int idx ) const;
    bool isRefPicLongterm( int idx ) const;

    void setRefPicLongterm( int idx, bool isLongterm );

    void setNumberOfShorttermPictures( int numberOfStrp );
    int  getNumberOfShorttermPictures() const;

    void setNumberOfLongtermPictures( int numberOfLtrp );
    int  getNumberOfLongtermPictures() const;

    void setLtrpInSliceHeaderFlag( bool flag ) { m_ltrp_in_slice_header_flag = flag; }
    bool getLtrpInSliceHeaderFlag() const { return m_ltrp_in_slice_header_flag; }

    void setNumberOfInterLayerPictures( const int numberOfIlrp ) { m_numberOfInterLayerPictures = numberOfIlrp; }
    int  getNumberOfInterLayerPictures() const { return m_numberOfInterLayerPictures; }

    int getNumRefEntries() const { return m_numberOfShorttermPictures + m_numberOfLongtermPictures + m_numberOfInterLayerPictures; }

    void setPOC( int idx, int POC );
    int  getPOC( int idx ) const;

    int  getDeltaPocMSBCycleLT( int i ) const { return m_deltaPOCMSBCycleLT[i]; }
    void setDeltaPocMSBCycleLT( int i, int x );
    bool getDeltaPocMSBPresentFlag( int i ) const { return m_deltaPocMSBPresentFlag[i]; }
    void setDeltaPocMSBPresentFlag( int i, bool x );

    bool getInterLayerPresentFlag() const { return m_interLayerPresentFlag; }
    void setInterLayerPresentFlag( bool b ) { m_interLayerPresentFlag = b; }
    bool isInterLayerRefPic( int idx ) const { return m_isInterLayerRefPic[idx]; }
    int  getInterLayerRefPicIdx( int idx ) const { return m_interLayerRefPicIdx[idx]; }
    void setInterLayerRefPicIdx( int idx, int layerIdc );

    bool findInRefPicList( const Picture* checkRefPic, int currPicPoc, int layerId ) const;

    static int calcLTRefPOC( int currPoc, int bitsForPoc, int refPicIdentifier, bool pocMSBPresent, int deltaPocMSBCycle );
    int        calcLTRefPOC( int currPoc, int bitsForPoc, int refPicIdx ) const;
};

typedef std::vector<ReferencePictureList> RPLList;

class ScalingList {
public:
    ScalingList();
    ~ScalingList() = default;
    CLASS_COPY_MOVE_DEFAULT( ScalingList )

    void       reset();

    static inline int  matrixSize( uint32_t scalingListId )   { return scalingListId < SCALING_LIST_1D_START_4x4 ? 2 : scalingListId < SCALING_LIST_1D_START_8x8 ? 4 : 8; }
    static inline bool isLumaScalingList( int scalingListId ) { return scalingListId % MAX_NUM_COMPONENT == SCALING_LIST_1D_START_4x4 || scalingListId == SCALING_LIST_1D_START_64x64 + 1;}

    void              setScalingListDC(uint32_t scalingListId, uint32_t u)             { m_scalingListDC[scalingListId] = u;                       } //!< set DC value
    int               getScalingListDC(uint32_t scalingListId) const                   { return m_scalingListDC[scalingListId];                    } //!< get DC value

    int*              getScalingListAddress(uint32_t scalingListId)                    { return m_scalingListCoef[scalingListId].data();           } //!< get matrix coefficient
    const int*        getScalingListAddress(uint32_t scalingListId) const              { return m_scalingListCoef[scalingListId].data();           } //!< get matrix coefficient
    std::vector<int>& getScalingListVec( uint32_t scalingListId )                      { return m_scalingListCoef[scalingListId]; }

private:
    int              m_scalingListDC  [28] = { 0 }; //!< the DC value of the matrix coefficient for 16x16
    std::vector<int> m_scalingListCoef[28];         //!< quantization matrix
};


class SPS : public BasePS<SPS> {
private:
    int               m_SPSId                              = 0;
    int               m_decodingParameterSetId             = 0;
    int               m_VPSId                              = 0;
    int               m_layerId                            = 0;
    bool              m_affineAmvrEnabledFlag              = false;
    bool              m_fpelMmvdEnabledFlag                = false;
    bool              m_DMVR                               = false;
    bool              m_MMVD                               = false;
    bool              m_SBT                                = false;
    bool              m_ISP                                = false;
    ChromaFormat      m_chromaFormatIdc                    = CHROMA_420;
    uint32_t          m_uiMaxTLayers                       = 1;            // maximum number of temporal layers
    bool              m_BdofControlPresentInPhFlag         = false;
    bool              m_DmvrControlPresentInPhFlag         = false;
    bool              m_ProfControlPresentInPhFlag         = false;
    bool              m_ptlDpbHrdParamsPresentFlag         = false;
    bool              m_SubLayerDpbParamsFlag              = false;

    // Structure
    uint32_t          m_maxWidthInLumaSamples              = 0;
    uint32_t          m_maxHeightInLumaSamples             = 0;

    bool              m_subPicInfoPresentFlag              = false;                // indicates the presence of sub-picture info
    uint8_t           m_numSubPics                         = 1;                        //!< number of sub-pictures used
    uint32_t          m_subPicCtuTopLeftX                  [MAX_NUM_SUB_PICS] = { 0 };
    uint32_t          m_subPicCtuTopLeftY                  [MAX_NUM_SUB_PICS] = { 0 };
    uint32_t          m_SubPicWidth                        [MAX_NUM_SUB_PICS] = { 0 };
    uint32_t          m_SubPicHeight                       [MAX_NUM_SUB_PICS] = { 0 };
    bool              m_subPicTreatedAsPicFlag             [MAX_NUM_SUB_PICS] = { 0 };
    bool              m_loopFilterAcrossSubpicEnabledFlag  [MAX_NUM_SUB_PICS] = { 0 };
    bool              m_subPicIdMappingExplicitlySignalledFlag = false;
    bool              m_subPicIdMappingPresentFlag         = false;
    uint32_t          m_subPicIdLen                        = 0;                               //!< sub-picture ID length in bits
    uint16_t          m_subPicId                           [MAX_NUM_SUB_PICS] = { 0 };        //!< sub-picture ID for each sub-picture in the sequence
    int               m_log2MinCodingBlockSize             = 0;
    unsigned          m_CTUSize                            = 0;
    unsigned          m_partitionOverrideEnalbed           = 0;            // enable partition constraints override function
    PartitionConstraints m_minQT                           = PartitionConstraints{ 0, 0, 0 };    // 0: I slice luma; 1: P/B slice luma; 2: I slice chroma
    PartitionConstraints m_maxMTTHierarchyDepth            = PartitionConstraints{ 0, 0, 0 };
    PartitionConstraints m_maxBTSize                       = PartitionConstraints{ 0, 0, 0 };
    PartitionConstraints m_maxTTSize                       = PartitionConstraints{ 0, 0, 0 };
    bool              m_idrRefParamList                    = false;
    unsigned          m_dualITree                          = 0;
    uint32_t          m_uiMaxCUWidth                       = 32;
    uint32_t          m_uiMaxCUHeight                      = 32;
    bool              m_conformanceWindowPresentFlag       = false;
    Window            m_conformanceWindow;
    bool              m_independentSubPicsFlag             = true;
    bool              m_subPicSameSizeFlag                 = false;

    RPLList           m_RPLList[2];
    uint32_t          m_numRPL[2]                          = {0,0};

    bool              m_rpl1CopyFromRpl0Flag               = false;
    bool              m_rpl1IdxPresentFlag                 = false;
    bool              m_allRplEntriesHasSameSignFlag       = true;
    bool              m_bLongTermRefsPresent               = false;
    bool              m_SPSTemporalMVPEnabledFlag          = false;
    int               m_numReorderPics[MAX_TLAYER]         = { 0 };

    // Tool list
    uint32_t          m_uiQuadtreeTULog2MaxSize            = 0;
    uint32_t          m_uiQuadtreeTULog2MinSize            = 0;
    uint32_t          m_uiQuadtreeTUMaxDepthInter          = 0;
    uint32_t          m_uiQuadtreeTUMaxDepthIntra          = 0;
    bool              m_useAMP                             = false;

    bool              m_transformSkipEnabledFlag           = false;
    int               m_log2MaxTransformSkipBlockSize      = 2;
    bool              m_BDPCMEnabledFlag                   = false;
    bool              m_JointCbCrEnabledFlag               = false;
    // Parameter
    int               m_bitDepths;
    bool              m_entropyCodingSyncEnabledFlag       = false;              //!< Flag for enabling WPP
    bool              m_entryPointPresentFlag              = false;              //!< Flag for indicating the presence of entry points
    int               m_qpBDOffset                         = 0;
    int               m_internalMinusInputBitDepth         = 0;                  //  max(0, internal bitdepth - input bitdepth);                                          }
    bool              m_sbtmvpEnabledFlag                  = false;
    bool              m_disFracMmvdEnabledFlag             = false;

    uint32_t          m_uiBitsForPOC                       =  8;
    bool              m_pocMsbFlag                         = false;
    uint32_t          m_pocMsbLen                          = 0;
    uint32_t          m_numLongTermRefPicSPS               = 0;
    uint32_t          m_ltRefPicPocLsbSps     [MAX_NUM_LONG_TERM_REF_PICS] = { 0 };
    bool              m_usedByCurrPicLtSPSFlag[MAX_NUM_LONG_TERM_REF_PICS] = { 0 };
    int               m_numExtraPHBitsBytes                = 0;
    int               m_numExtraSHBitsBytes                = 0;
    std::vector<bool> m_extraPHBitPresentFlag;
    std::vector<bool> m_extraSHBitPresentFlag;
    uint32_t          m_log2MaxTbSize                      = 6;

    bool             m_useWeightPred                       = false; //!< Use of Weighting Prediction (P_SLICE)
    bool             m_useWeightedBiPred                   = false; //!< Use of Weighting Bi-Prediction (B_SLICE)
    // Max physical transform size
    bool              m_bUseSAO                            = false;

    bool              m_bTemporalIdNestingFlag             = false;      // temporal_id_nesting_flag

    bool              m_scalingListEnabledFlag             = false;
    bool              m_scalingListPresentFlag             = false;
    ScalingList       m_scalingList;
    bool              m_depQuantEnabledFlag                = false;      //!< dependent quantization enabled flag
    bool              m_signDataHidingEnabledFlag          = false;      //!< sign data hiding enabled flag
    bool              m_virtualBoundariesEnabledFlag       = false;      //!< Enable virtual boundaries tool
    bool              m_virtualBoundariesPresentFlag       = false;      //!< disable loop filtering across virtual boundaries
    unsigned          m_numVerVirtualBoundaries            = 0;          //!< number of vertical virtual boundaries
    unsigned          m_numHorVirtualBoundaries            = 0;          //!< number of horizontal virtual boundaries
    unsigned          m_virtualBoundariesPosX[3]           = { 0 };      //!< horizontal position of each vertical virtual boundary
    unsigned          m_virtualBoundariesPosY[3]           = { 0 };      //!< vertical position of each horizontal virtual boundary
    uint32_t          m_uiMaxDecPicBuffering     [MAX_TLAYER];  // init in constructor to 1
    uint32_t          m_uiMaxLatencyIncreasePlus1[MAX_TLAYER] = { 0 };

    uint32_t          m_maxNumMergeCand                    = 0;
    uint32_t          m_maxNumAffineMergeCand              = 0;
    uint32_t          m_maxNumIBCMergeCand                 = 0;
    uint32_t          m_maxNumGeoCand                      = 0;

    bool              m_generalHrdParametersPresentFlag    = false;

    bool              m_fieldSeqFlag                       = false;
    bool              m_vuiParametersPresentFlag           = false;
    unsigned          m_vuiPayloadSize                     = 0;

    //SPSRExt           m_spsRangeExtension;
    static const      int m_winUnitX[NUM_CHROMA_FORMAT];
    static const      int m_winUnitY[NUM_CHROMA_FORMAT];

    bool              m_useALF                             = false;
    bool              m_useCCALF                           = false;

    bool              m_useWrapAround                      = false;
    bool              m_IBCFlag                            = false;
    bool              m_useColorTrans                      = false;
    bool              m_lumaReshapeEnable                  = false;
    bool              m_AMVREnabledFlag                    = false;
    bool              m_LMChroma                           = false;
    bool              m_horCollocatedChromaFlag            = true;
    bool              m_verCollocatedChromaFlag            = true;   // standard says to be inferred 1, but setting it to 0 correctly decodes some 422 sequences. Seems to be due to getCclmCollocatedChromaFlag()
    bool              m_MTS                                = false;
    bool              m_IntraMTS                           = false;
    bool              m_InterMTS                           = false;
    bool              m_LFNST                              = false;
    bool              m_SMVD                               = false;
    bool              m_Affine                             = false;
    bool              m_AffineType                         = false;
    bool              m_PROF                               = false;
    bool              m_BIO                                = false;
    bool              m_bcw                                = false;                        //
    bool              m_ciip                               = false;
    bool              m_MRL                                = false;
    bool              m_Geo                                = false;

    bool              m_LadfEnabled                        = false;
    int               m_LadfNumIntervals                   = 0;
    int               m_LadfQpOffset          [MAX_LADF_INTERVALS] = { 0 };
    int               m_LadfIntervalLowerBound[MAX_LADF_INTERVALS] = { 0 };
    bool              m_MIP                                = false;
    // ChromaQpMappingTable m_chromaQpMappingTable;
    bool              m_GDREnabledFlag                     = false;
    bool              m_SubLayerCbpParametersPresentFlag   = false;
    bool              m_rprEnabledFlag                     = false;
    bool              m_resChangeInClvsEnabledFlag         = false;
    bool              m_interLayerPresentFlag              = false;
    uint32_t          m_log2ParallelMergeLevelMinus2       = 0;
    bool              m_scalingMatrixAlternativeColourSpaceDisabledFlag = false;
    bool              m_scalingMatrixDesignatedColourSpaceFlag          = true;
    bool              m_disableScalingMatrixForLfnstBlks                = true;
public:

    SPS() {
        for ( int i = 0; i < MAX_TLAYER; i++ ) {
            m_uiMaxDecPicBuffering[i]      = 1;
        }
    }
    ~SPS() = default;

    int                     getSPSId() const                                                                { return m_SPSId;                                                      }
    void                    setSPSId(int i)                                                                 { m_SPSId = i;                                                         }
    void                    setDecodingParameterSetId(int val)                                              { m_decodingParameterSetId = val;                                      }
    int                     getDecodingParameterSetId() const                                               { return m_decodingParameterSetId;                                     }
    int                     getVPSId() const                                                                { return m_VPSId; }
    void                    setVPSId(int i)                                                                 { m_VPSId = i; }
    void                    setLayerId( int i )                                                             { m_layerId = i;                                                       }
    int                     getLayerId() const                                                              { return m_layerId;                                                    }
    ChromaFormat            getChromaFormatIdc () const                                                     { return m_chromaFormatIdc;                                            }
    void                    setChromaFormatIdc (ChromaFormat i)                                             { m_chromaFormatIdc = i;                                               }

    static int              getWinUnitX (int chromaFormatIdc)                                               { CHECK(chromaFormatIdc < 0 || chromaFormatIdc >= NUM_CHROMA_FORMAT, "Invalid chroma format parameter"); return m_winUnitX[chromaFormatIdc]; }
    static int              getWinUnitY (int chromaFormatIdc)                                               { CHECK(chromaFormatIdc < 0 || chromaFormatIdc >= NUM_CHROMA_FORMAT, "Invalid chroma format parameter"); return m_winUnitY[chromaFormatIdc]; }

    // structure
    void                    setMaxPicWidthInLumaSamples( uint32_t u )                                       { m_maxWidthInLumaSamples = u; }
    uint32_t                getMaxPicWidthInLumaSamples() const                                             { return  m_maxWidthInLumaSamples; }
    void                    setMaxPicHeightInLumaSamples( uint32_t u )                                      { m_maxHeightInLumaSamples = u; }
    uint32_t                getMaxPicHeightInLumaSamples() const                                            { return  m_maxHeightInLumaSamples; }
    void                    setConformanceWindowPresentFlag(bool b)                                         { m_conformanceWindowPresentFlag = b;           }
    bool                    getConformanceWindowPresentFlag() const                                         { return m_conformanceWindowPresentFlag;        }
    Window&                 getConformanceWindow()                                                          { return  m_conformanceWindow;                                         }
    const Window&           getConformanceWindow() const                                                    { return  m_conformanceWindow;                                         }
    void                    setConformanceWindow(Window& conformanceWindow )                                { m_conformanceWindow = conformanceWindow;                             }

    void                    setSubPicInfoPresentFlag(bool b)                                                { m_subPicInfoPresentFlag = b;            }
    bool                    getSubPicInfoPresentFlag() const                                                { return m_subPicInfoPresentFlag;         }

    void                    setNumSubPics( uint8_t u )                                                      { m_numSubPics = u;                           }
    void                    setIndependentSubPicsFlag(bool b)                                               { m_independentSubPicsFlag = b;                    }
    bool                    getIndependentSubPicsFlag() const                                               { return m_independentSubPicsFlag;                 }
    void                    setSubPicSameSizeFlag(bool b)                                                   { m_subPicSameSizeFlag = b;                       }
    bool                    getSubPicSameSizeFlag() const                                                   { return m_subPicSameSizeFlag;                    }
    uint8_t                 getNumSubPics( ) const                                                          { return  m_numSubPics;            }
    void                    setSubPicCtuTopLeftX( int i, uint32_t u )                                       { CHECK( i >= MAX_NUM_SUB_PICS, "Sub-picture index exceeds valid range" ); m_subPicCtuTopLeftX[i] = u;                     }
    uint32_t                getSubPicCtuTopLeftX( int i ) const                                             { CHECK( i >= MAX_NUM_SUB_PICS, "Sub-picture index exceeds valid range" ); return  m_subPicCtuTopLeftX[i];                 }
    void                    setSubPicCtuTopLeftY( int i, uint32_t u )                                       { CHECK( i >= MAX_NUM_SUB_PICS, "Sub-picture index exceeds valid range" ); m_subPicCtuTopLeftY[i] = u;                     }
    uint32_t                getSubPicCtuTopLeftY( int i ) const                                             { CHECK( i >= MAX_NUM_SUB_PICS, "Sub-picture index exceeds valid range" ); return  m_subPicCtuTopLeftY[i];                 }
    void                    setSubPicWidth( int i, uint32_t u )                                             { CHECK( i >= MAX_NUM_SUB_PICS, "Sub-picture index exceeds valid range" ); m_SubPicWidth[i] = u;                           }
    uint32_t                getSubPicWidth( int i ) const                                                   { CHECK( i >= MAX_NUM_SUB_PICS, "Sub-picture index exceeds valid range" ); return  m_SubPicWidth[i];                       }
    void                    setSubPicHeight( int i, uint32_t u )                                            { CHECK( i >= MAX_NUM_SUB_PICS, "Sub-picture index exceeds valid range" ); m_SubPicHeight[i] = u;                          }
    uint32_t                getSubPicHeight( int i ) const                                                  { CHECK( i >= MAX_NUM_SUB_PICS, "Sub-picture index exceeds valid range" ); return  m_SubPicHeight[i];                      }
    void                    setSubPicTreatedAsPicFlag( int i, bool u )                                      { CHECK( i >= MAX_NUM_SUB_PICS, "Sub-picture index exceeds valid range" ); m_subPicTreatedAsPicFlag[i] = u;                }
    bool                    getSubPicTreatedAsPicFlag( int i ) const                                        { CHECK( i >= MAX_NUM_SUB_PICS, "Sub-picture index exceeds valid range" ); return  m_subPicTreatedAsPicFlag[i];            }
    void                    setLoopFilterAcrossSubpicEnabledFlag( int i, bool u )                           { CHECK( i >= MAX_NUM_SUB_PICS, "Sub-picture index exceeds valid range" ); m_loopFilterAcrossSubpicEnabledFlag[i] = u;     }
    bool                    getLoopFilterAcrossSubpicEnabledFlag( int i ) const                             { CHECK( i >= MAX_NUM_SUB_PICS, "Sub-picture index exceeds valid range" ); return  m_loopFilterAcrossSubpicEnabledFlag[i]; }
    void                    setSubPicIdMappingExplicitlySignalledFlag( bool b )                             { m_subPicIdMappingExplicitlySignalledFlag = b;    }
    bool                    getSubPicIdMappingExplicitlySignalledFlag() const                               { return m_subPicIdMappingExplicitlySignalledFlag; }
    void                    setSubPicIdMappingPresentFlag( bool b )                                         { m_subPicIdMappingPresentFlag = b;                }
    bool                    getSubPicIdMappingPresentFlag() const                                           { return  m_subPicIdMappingPresentFlag;            }
    void                    setSubPicIdLen( uint32_t u )                                                    { CHECK( u > 16, "Sub-Picture id length exeeds valid range" ); m_subPicIdLen = u;                 }
    uint32_t                getSubPicIdLen() const                                                          { return  m_subPicIdLen;                                                                          }
    void                    setSubPicId( int i, uint16_t u )                                                { CHECK( i >= MAX_NUM_SUB_PICS, "Sub-picture index exceeds valid range" ); m_subPicId[i] = u;     }
    uint16_t                getSubPicId( int i ) const                                                      { CHECK( i >= MAX_NUM_SUB_PICS, "Sub-picture index exceeds valid range" ); return  m_subPicId[i]; }

    uint32_t                getNumLongTermRefPicSPS() const                                                 { return m_numLongTermRefPicSPS;                                       }
    void                    setNumLongTermRefPicSPS(uint32_t val)                                           { m_numLongTermRefPicSPS = val;                                        }

    uint32_t                getLtRefPicPocLsbSps(uint32_t index) const                                      { CHECK( index >= MAX_NUM_LONG_TERM_REF_PICS, "Index exceeds boundary" ); return m_ltRefPicPocLsbSps[index]; }
    void                    setLtRefPicPocLsbSps(uint32_t index, uint32_t val)                              { CHECK( index >= MAX_NUM_LONG_TERM_REF_PICS, "Index exceeds boundary" ); m_ltRefPicPocLsbSps[index] = val;  }

    bool                    getUsedByCurrPicLtSPSFlag(int i) const                                          { CHECK( i >= MAX_NUM_LONG_TERM_REF_PICS, "Index exceeds boundary" ); return m_usedByCurrPicLtSPSFlag[i];    }
    void                    setUsedByCurrPicLtSPSFlag(int i, bool x)                                        { CHECK( i >= MAX_NUM_LONG_TERM_REF_PICS, "Index exceeds boundary" ); m_usedByCurrPicLtSPSFlag[i] = x;       }

    int                     getLog2MinCodingBlockSize() const                                               { return m_log2MinCodingBlockSize;                                     }
    void                    setLog2MinCodingBlockSize(int val)                                              { m_log2MinCodingBlockSize = val;                                      }
    void                    setCTUSize(unsigned    ctuSize)                                                 { m_CTUSize = ctuSize; }
    unsigned                getCTUSize()                                                              const { return  m_CTUSize; }
    void                    setSplitConsOverrideEnabledFlag(bool b)                                         { m_partitionOverrideEnalbed = b; }
    bool                    getSplitConsOverrideEnabledFlag()                                         const { return m_partitionOverrideEnalbed; }

    const PartitionConstraints& getMinQTSizes()                                                       const { return m_minQT;                                                      }
    const PartitionConstraints& getMaxMTTHierarchyDepths()                                            const { return m_maxMTTHierarchyDepth;                                       }
    const PartitionConstraints& getMaxBTSizes()                                                       const { return m_maxBTSize;                                                  }
    const PartitionConstraints& getMaxTTSizes()                                                       const { return m_maxTTSize;                                                  }

    void                    setMinQTSize           ( unsigned idx, unsigned minQT  )                        { m_minQT[idx] = minQT;                                                }
    void                    setMaxMTTHierarchyDepth( unsigned idx, unsigned maxMTT )                        { m_maxMTTHierarchyDepth[idx] = maxMTT;                                }
    void                    setMaxBTSize           ( unsigned idx, unsigned maxBT  )                        { m_maxBTSize[idx] = maxBT;                                            }
    void                    setMaxTTSize           ( unsigned idx, unsigned maxTT  )                        { m_maxTTSize[idx] = maxTT;                                            }

    void                    setMinQTSizes           ( const PartitionConstraints& minQT )                   { m_minQT = minQT;                                                     }
    void                    setMaxMTTHierarchyDepths( const PartitionConstraints& maxMTT )                  { m_maxMTTHierarchyDepth = maxMTT;                                     }
    void                    setMaxBTSizes           ( const PartitionConstraints& maxBT )                   { m_maxBTSize = maxBT;                                                 }
    void                    setMaxTTSizes           ( const PartitionConstraints& maxTT )                   { m_maxTTSize = maxTT;                                                 }

    unsigned                getMinQTSize           ( SliceType slicetype, ChannelType chType = CHANNEL_TYPE_LUMA ) const { return m_minQT               [chType == CHANNEL_TYPE_LUMA ? slicetype == I_SLICE ?  0 : 1 : 2]; }
    unsigned                getMaxMTTHierarchyDepth( SliceType slicetype, ChannelType chType = CHANNEL_TYPE_LUMA ) const { return m_maxMTTHierarchyDepth[chType == CHANNEL_TYPE_LUMA ? slicetype == I_SLICE ?  0 : 1 : 2]; }
    unsigned                getMaxBTSize           ( SliceType slicetype, ChannelType chType = CHANNEL_TYPE_LUMA ) const { return m_maxBTSize           [chType == CHANNEL_TYPE_LUMA ? slicetype == I_SLICE ?  0 : 1 : 2]; }
    unsigned                getMaxTTSize           ( SliceType slicetype, ChannelType chType = CHANNEL_TYPE_LUMA ) const { return m_maxTTSize           [chType == CHANNEL_TYPE_LUMA ? slicetype == I_SLICE ?  0 : 1 : 2]; }

    unsigned                getMaxMTTHierarchyDepth()                                                 const { return m_maxMTTHierarchyDepth[1]; }
    unsigned                getMaxMTTHierarchyDepthI()                                                const { return m_maxMTTHierarchyDepth[0]; }
    unsigned                getMaxMTTHierarchyDepthIChroma()                                          const { return m_maxMTTHierarchyDepth[2]; }

    unsigned                getMaxBTSize()                                                            const { return m_maxBTSize[1]; }
    unsigned                getMaxBTSizeI()                                                           const { return m_maxBTSize[0]; }
    unsigned                getMaxBTSizeIChroma()                                                     const { return m_maxBTSize[2]; }

    unsigned                getMaxTTSize()                                                            const { return m_maxTTSize[1]; }
    unsigned                getMaxTTSizeI()                                                           const { return m_maxTTSize[0]; }
    unsigned                getMaxTTSizeIChroma()                                                     const { return m_maxTTSize[2]; }

    void                    setIDRRefParamListPresent(bool b)                                               { m_idrRefParamList = b; }
    bool                    getIDRRefParamListPresent()                                               const { return m_idrRefParamList; }
    void                    setUseDualITree(bool b) { m_dualITree = b; }
    bool                    getUseDualITree()                                                         const { return m_dualITree; }

    void                    setMaxCUWidth( uint32_t u )                                                     { m_uiMaxCUWidth = u;                                                  }
    uint32_t                getMaxCUWidth() const                                                           { return  m_uiMaxCUWidth;                                              }
    void                    setMaxCUHeight( uint32_t u )                                                    { m_uiMaxCUHeight = u;                                                 }
    uint32_t                getMaxCUHeight() const                                                          { return  m_uiMaxCUHeight;                                             }
    bool                    getTransformSkipEnabledFlag() const                                             { return m_transformSkipEnabledFlag;                                   }
    void                    setTransformSkipEnabledFlag( bool b )                                           { m_transformSkipEnabledFlag = b;                                      }
    uint32_t                getLog2MaxTransformSkipBlockSize() const                                        { return m_log2MaxTransformSkipBlockSize;                              }
    void                    setLog2MaxTransformSkipBlockSize(uint32_t u)                                    { m_log2MaxTransformSkipBlockSize = u;                                 }
    bool                    getBDPCMEnabledFlag() const                                                     { return m_BDPCMEnabledFlag;                                           }
    void                    setBDPCMEnabledFlag( bool b )                                                   { m_BDPCMEnabledFlag = b;                                              }
    void                    setBitsForPOC( uint32_t u )                                                     { m_uiBitsForPOC = u;                                                  }
    uint32_t                getBitsForPOC() const                                                           { return m_uiBitsForPOC;                                               }
    void                    setPocMsbFlag(bool b)                                                           { m_pocMsbFlag = b;                                                    }
    bool                    getPocMsbFlag() const                                                           { return m_pocMsbFlag;                                                 }
    void                    setPocMsbLen(uint32_t u)                                                        { m_pocMsbLen = u;                                                     }
    uint32_t                getPocMsbLen() const                                                            { return m_pocMsbLen;                                                  }
    void                    setNumExtraPHBitsBytes(int i)                                                   { m_numExtraPHBitsBytes = i;                                           }
    int                     getNumExtraPHBitsBytes() const                                                  { return m_numExtraPHBitsBytes;                                        }
    void                    setNumExtraSHBitsBytes(int i)                                                   { m_numExtraSHBitsBytes = i;                                           }
    int                     getNumExtraSHBitsBytes() const                                                  { return m_numExtraSHBitsBytes;                                        }
    void                     setExtraPHBitPresentFlags( const std::vector<bool>&& b )                       { m_extraPHBitPresentFlag = std::move( b );                            }
    const std::vector<bool>& getExtraPHBitPresentFlags() const                                              { return m_extraPHBitPresentFlag;                                      }
    void                     setExtraSHBitPresentFlags( const std::vector<bool>&& b )                       { m_extraSHBitPresentFlag = std::move( b );                            }
    const std::vector<bool>& getExtraSHBitPresentFlags() const                                              { return m_extraSHBitPresentFlag;                                      }
    bool                    getUseAMP() const                                                               { return m_useAMP;                                                     }
    void                    setUseAMP( bool b )                                                             { m_useAMP = b;                                                        }
    void                    setQuadtreeTULog2MaxSize( uint32_t u )                                          { m_uiQuadtreeTULog2MaxSize = u;                                       }
    uint32_t                getQuadtreeTULog2MaxSize() const                                                { return m_uiQuadtreeTULog2MaxSize;                                    }
    void                    setQuadtreeTULog2MinSize( uint32_t u )                                          { m_uiQuadtreeTULog2MinSize = u;                                       }
    uint32_t                getQuadtreeTULog2MinSize() const                                                { return m_uiQuadtreeTULog2MinSize;                                    }
    void                    setQuadtreeTUMaxDepthInter( uint32_t u )                                        { m_uiQuadtreeTUMaxDepthInter = u;                                     }
    void                    setQuadtreeTUMaxDepthIntra( uint32_t u )                                        { m_uiQuadtreeTUMaxDepthIntra = u;                                     }
    uint32_t                getQuadtreeTUMaxDepthInter() const                                              { return m_uiQuadtreeTUMaxDepthInter;                                  }
    uint32_t                getQuadtreeTUMaxDepthIntra() const                                              { return m_uiQuadtreeTUMaxDepthIntra;                                  }
    void                    setNumReorderPics(int i, uint32_t tlayer)                                       { m_numReorderPics[tlayer] = i;                                        }
    int                     getNumReorderPics(uint32_t tlayer) const                                        { return m_numReorderPics[tlayer];                                     }
    RPLList&                createRPLList( int l, int numRPL );
    const RPLList&          getRPLList( int l ) const                                                       { return m_RPLList[l];                                                 }
    uint32_t                getNumRPL( int l ) const                                                        { return m_numRPL[l];                                                  }
    void                    setRPL1CopyFromRPL0Flag(bool isCopy)                                            { m_rpl1CopyFromRpl0Flag = isCopy;                                     }
    bool                    getRPL1CopyFromRPL0Flag() const                                                 { return m_rpl1CopyFromRpl0Flag;                                       }
    bool                    getRPL1IdxPresentFlag() const                                                   { return m_rpl1IdxPresentFlag;                                         }
    void                    setAllActiveRplEntriesHasSameSignFlag(bool isAllSame)                           { m_allRplEntriesHasSameSignFlag = isAllSame;                          }
    bool                    getAllActiveRplEntriesHasSameSignFlag() const                                   { return m_allRplEntriesHasSameSignFlag;                               }
    bool                    getLongTermRefsPresent() const                                                  { return m_bLongTermRefsPresent;                                       }
    void                    setLongTermRefsPresent(bool b)                                                  { m_bLongTermRefsPresent=b;                                            }
    bool                    getSPSTemporalMVPEnabledFlag() const                                            { return m_SPSTemporalMVPEnabledFlag;                                  }
    void                    setSPSTemporalMVPEnabledFlag(bool b)                                            { m_SPSTemporalMVPEnabledFlag=b;                                       }
    void                    setLog2MaxTbSize( uint32_t u )                                                  { m_log2MaxTbSize = u;                                                 }
    uint32_t                getLog2MaxTbSize() const                                                        { return  m_log2MaxTbSize;                                             }
    uint32_t                getMaxTbSize() const                                                            { return  1 << m_log2MaxTbSize;                                        }
    // Bit-depth
    int                     getBitDepth() const                                                             { return m_bitDepths;                                                  }
    void                    setBitDepth(int u )                                                             { m_bitDepths = u;                                                     }


    bool                    getEntropyCodingSyncEnabledFlag() const                                         { return m_entropyCodingSyncEnabledFlag;                               }
    void                    setEntropyCodingSyncEnabledFlag(bool val)                                       { m_entropyCodingSyncEnabledFlag = val;                                }
    bool                    getEntryPointsPresentFlag() const                                               { return m_entryPointPresentFlag;                                      }
    void                    setEntryPointsPresentFlag(bool val)                                             { m_entryPointPresentFlag = val;                                       }

    static constexpr int    getMaxLog2TrDynamicRange( ChannelType )            			                        { return 15; }

    int                     getQpBDOffset() const                                                           { return m_qpBDOffset;                                                 }
    void                    setQpBDOffset(int i)                                                            { m_qpBDOffset = i;                                                    }
    int                     getInternalMinusInputBitDepth() const                                           { return m_internalMinusInputBitDepth;                                 }
    void                    setInternalMinusInputBitDepth(int i)                                            { m_internalMinusInputBitDepth = i;                                    }
    void                    setUseSAO(bool bVal)                                                            { m_bUseSAO = bVal;                                                    }
    bool                    getUseSAO() const                                                               { return m_bUseSAO;                                                    }

    void                    setJointCbCrEnabledFlag(bool bVal)                                              { m_JointCbCrEnabledFlag = bVal; }
    bool                    getJointCbCrEnabledFlag() const                                                 { return m_JointCbCrEnabledFlag; }

    bool                    getSBTMVPEnabledFlag() const                                                    { return m_sbtmvpEnabledFlag; }
    void                    setSBTMVPEnabledFlag(bool b)                                                    { m_sbtmvpEnabledFlag = b; }

    bool                    getDisFracMmvdEnabledFlag() const                                               { return m_disFracMmvdEnabledFlag; }
    void                    setDisFracMmvdEnabledFlag( bool b )                                             { m_disFracMmvdEnabledFlag = b;    }

    bool                    getFpelMmvdEnabledFlag() const                                                  { return m_fpelMmvdEnabledFlag; }
    void                    setFpelMmvdEnabledFlag( bool b )                                                { m_fpelMmvdEnabledFlag = b;    }

    bool                    getUseDMVR()const                                                               { return m_DMVR; }
    void                    setUseDMVR(bool b)                                                              { m_DMVR = b;    }

    bool                    getUseMMVD()const                                                               { return m_MMVD; }
    void                    setUseMMVD(bool b)                                                              { m_MMVD = b;    }

    bool                    getBdofControlPresentInPhFlag()const                                            { return m_BdofControlPresentInPhFlag; }
    void                    setBdofControlPresentInPhFlag(bool b)                                           { m_BdofControlPresentInPhFlag = b;    }

    bool                    getDmvrControlPresentInPhFlag()const                                            { return m_DmvrControlPresentInPhFlag; }
    void                    setDmvrControlPresentInPhFlag(bool b)                                           { m_DmvrControlPresentInPhFlag = b;    }

    bool                    getProfControlPresentInPhFlag()const                                            { return m_ProfControlPresentInPhFlag; }
    void                    setProfControlPresentInPhFlag(bool b)                                           { m_ProfControlPresentInPhFlag = b;    }

    uint32_t                getMaxTLayers() const                                                           { return m_uiMaxTLayers; }
    void                    setMaxTLayers( uint32_t uiMaxTLayers )                                          { CHECK( uiMaxTLayers > MAX_TLAYER, "Invalid number T-layers" ); m_uiMaxTLayers = uiMaxTLayers; }

    bool                    getPtlDpbHrdParamsPresentFlag()  const                                          { return m_ptlDpbHrdParamsPresentFlag;     }
    void                    setPtlDpbHrdParamsPresentFlag(bool b)                                           {        m_ptlDpbHrdParamsPresentFlag = b; }
    bool                    getSubLayerDpbParamsFlag()  const                                               { return m_SubLayerDpbParamsFlag;          }
    void                    setSubLayerDpbParamsFlag(bool b)                                                {        m_SubLayerDpbParamsFlag = b;      }
    bool                    getTemporalIdNestingFlag() const                                                { return m_bTemporalIdNestingFlag;                                     }
    void                    setTemporalIdNestingFlag( bool bValue )                                         { m_bTemporalIdNestingFlag = bValue;                                   }

    bool                    getScalingListFlag() const                                                      { return m_scalingListEnabledFlag;                                     }
    void                    setScalingListFlag( bool b )                                                    { m_scalingListEnabledFlag  = b;                                       }
    bool                    getScalingListPresentFlag() const                                               { return m_scalingListPresentFlag;                                     }
    void                    setScalingListPresentFlag( bool b )                                             { m_scalingListPresentFlag  = b;                                       }
    ScalingList&            getScalingList()                                                                { return m_scalingList; }
    const ScalingList&      getScalingList() const                                                          { return m_scalingList; }
    void                    setDepQuantEnabledFlag(bool b)                                                  { m_depQuantEnabledFlag = b; }
    bool                    getDepQuantEnabledFlag() const                                                  { return m_depQuantEnabledFlag; }
    void                    setSignDataHidingEnabledFlag(bool b)                                            { m_signDataHidingEnabledFlag = b; }
    bool                    getSignDataHidingEnabledFlag() const                                            { return m_signDataHidingEnabledFlag; }
    void                    setVirtualBoundariesEnabledFlag( bool b )                                       { m_virtualBoundariesEnabledFlag = b;                                  }
    bool                    getVirtualBoundariesEnabledFlag() const                                         { return m_virtualBoundariesEnabledFlag;                               }
    void                    setVirtualBoundariesPresentFlag( bool b )                                       { m_virtualBoundariesPresentFlag = b; }
    bool                    getVirtualBoundariesPresentFlag() const                                         { return m_virtualBoundariesPresentFlag; }
    void                    setNumVerVirtualBoundaries(unsigned u)                                          { m_numVerVirtualBoundaries = u;                                       }
    unsigned                getNumVerVirtualBoundaries() const                                              { return m_numVerVirtualBoundaries;                                    }
    void                    setNumHorVirtualBoundaries(unsigned u)                                          { m_numHorVirtualBoundaries = u;                                       }
    unsigned                getNumHorVirtualBoundaries() const                                              { return m_numHorVirtualBoundaries;                                    }
    void                    setVirtualBoundariesPosX(unsigned u, unsigned idx)                              { CHECK( idx >= 3, "vitrual boundary index exceeds valid range" ); m_virtualBoundariesPosX[idx] = u;    }
    unsigned                getVirtualBoundariesPosX(unsigned idx) const                                    { CHECK( idx >= 3, "vitrual boundary index exceeds valid range" ); return m_virtualBoundariesPosX[idx]; }
    void                    setVirtualBoundariesPosY(unsigned u, unsigned idx)                              { CHECK( idx >= 3, "vitrual boundary index exceeds valid range" ); m_virtualBoundariesPosY[idx] = u;    }
    unsigned                getVirtualBoundariesPosY(unsigned idx) const                                    { CHECK( idx >= 3, "vitrual boundary index exceeds valid range" ); return m_virtualBoundariesPosY[idx]; }

    uint32_t                getMaxDecPicBuffering(uint32_t tlayer) const                                    { return m_uiMaxDecPicBuffering[tlayer];                               }
    void                    setMaxDecPicBuffering( uint32_t ui, uint32_t tlayer )                           { CHECK(tlayer >= MAX_TLAYER, "Invalid T-layer"); m_uiMaxDecPicBuffering[tlayer] = ui;    }
    uint32_t                getMaxLatencyIncreasePlus1(uint32_t tlayer) const                               { return m_uiMaxLatencyIncreasePlus1[tlayer];                          }
    void                    setMaxLatencyIncreasePlus1( uint32_t ui , uint32_t tlayer)                      { m_uiMaxLatencyIncreasePlus1[tlayer] = ui;                            }

    uint32_t                getMaxNumMergeCand() const                                                      { return m_maxNumMergeCand; }
    void                    setMaxNumMergeCand(uint32_t u)                                                  { m_maxNumMergeCand = u; }
    uint32_t                getMaxNumAffineMergeCand() const                                                { return m_maxNumAffineMergeCand; }
    void                    setMaxNumAffineMergeCand(uint32_t u)                                            { m_maxNumAffineMergeCand = u; }
    uint32_t                getMaxNumIBCMergeCand() const                                                   { return m_maxNumIBCMergeCand; }
    void                    setMaxNumIBCMergeCand(uint32_t u)                                               { m_maxNumIBCMergeCand = u; }
    uint32_t                getMaxNumGeoCand() const                                                        { return m_maxNumGeoCand; }
    void                    setMaxNumGeoCand(uint32_t u)                                                    { m_maxNumGeoCand = u; }
    
    void                    setAffineAmvrEnabledFlag( bool val )                                            { m_affineAmvrEnabledFlag = val;                                       }
    bool                    getAffineAmvrEnabledFlag() const                                                { return m_affineAmvrEnabledFlag;                                      }
    bool                    getGeneralHrdParametersPresentFlag() const { return m_generalHrdParametersPresentFlag; }
    void                    setGeneralHrdParametersPresentFlag(bool b) { m_generalHrdParametersPresentFlag = b; }

    bool                    getFieldSeqFlag() const                                                         { return m_fieldSeqFlag;                         }
    void                    setFieldSeqFlag(bool i)                                                         { m_fieldSeqFlag = i;                            }
    bool                    getVuiParametersPresentFlag() const                                             { return m_vuiParametersPresentFlag;                                   }
    void                    setVuiParametersPresentFlag(bool b)                                             { m_vuiParametersPresentFlag = b;                                      }
    unsigned                getVuiPayloadSize() const                                                       { return m_vuiPayloadSize; }
    void                    setVuiPayloadSize(unsigned i)                                                   { m_vuiPayloadSize = i; }

    //const SPSRExt&          getSpsRangeExtension() const                                                    { return m_spsRangeExtension;                                          }
    //SPSRExt&                getSpsRangeExtension()                                                          { return m_spsRangeExtension;                                          }


    bool                    getUseALF() const                                                               { return m_useALF;                                                     }
    void                    setUseALF( bool b )                                                             { m_useALF = b;                                                        }
    bool                    getUseCCALF() const                                                             { return m_useCCALF; }
    void                    setUseCCALF( bool b )                                                           { m_useCCALF = b; }

    void                    setUseWrapAround(bool b)                                                        { m_useWrapAround = b;                                                 }
    bool                    getUseWrapAround() const                                                        { return m_useWrapAround;                                              }
    void                    setUseReshaper(bool b)                                                          { m_lumaReshapeEnable = b;                                                   }
    bool                    getUseReshaper() const                                                          { return m_lumaReshapeEnable;                                                }
    void                    setIBCFlag(bool IBCFlag)                                                    { m_IBCFlag = IBCFlag; }
    bool                    getIBCFlag() const                                                              { return m_IBCFlag; }
    void                    setUseColorTrans(bool value)                                                    { m_useColorTrans = value; }
    bool                    getUseColorTrans() const                                                        { return m_useColorTrans; }
    void                    setUseSBT( bool b )                                                             { m_SBT = b; }
    bool                    getUseSBT() const                                                               { return m_SBT; }
    void                    setUseISP( bool b )                                                             { m_ISP = b; }
    bool                    getUseISP() const                                                               { return m_ISP; }

    void      setAMVREnabledFlag    ( bool b )                                        { m_AMVREnabledFlag = b; }
    bool      getAMVREnabledFlag    ()                                      const     { return m_AMVREnabledFlag; }

    void      setUseAffine          ( bool b )                                        { m_Affine = b; }
    bool      getUseAffine          ()                                      const     { return m_Affine; }
    void      setUseAffineType      ( bool b )                                        { m_AffineType = b; }
    bool      getUseAffineType      ()                                      const     { return m_AffineType; }
    void      setUsePROF            ( bool b )                                        { m_PROF = b; }
    bool      getUsePROF            ()                                      const     { return m_PROF; }
    void      setUseBcw             ( bool b )                                        { m_bcw = b; }
    bool      getUseBcw             ()                                      const     { return m_bcw; }
    void      setUseCiip            ( bool b )                                        { m_ciip = b; }
    bool      getUseCiip            ()                                      const     { return m_ciip; }
    void      setUseBIO(bool b)                                                       { m_BIO = b; }
    bool      getUseBIO()                                                   const     { return m_BIO; }

    void      setUseLMChroma        ( bool b )                                        { m_LMChroma = b; }
    bool      getUseLMChroma        ()                                      const     { return m_LMChroma; }
    void      setHorCollocatedChromaFlag( bool b )                                    { m_horCollocatedChromaFlag = b;    }
    bool      getHorCollocatedChromaFlag()                                  const     { return m_horCollocatedChromaFlag; }
    void      setVerCollocatedChromaFlag( bool b )                                    { m_verCollocatedChromaFlag = b;    }
    bool      getVerCollocatedChromaFlag()                                  const     { return m_verCollocatedChromaFlag; }
    bool      getCclmCollocatedChromaFlag()                                 const     { return m_verCollocatedChromaFlag; }
    void      setUseMTS             ( bool b )                                        { m_MTS = b; }
    bool      getUseMTS             ()                                      const     { return m_MTS; }
    bool      getUseImplicitMTS     ()                                      const     { return m_MTS && !m_IntraMTS; }
    void      setUseIntraMTS        ( bool b )                                        { m_IntraMTS = b; }
    bool      getUseIntraMTS        ()                                      const     { return m_IntraMTS; }
    void      setUseInterMTS        ( bool b )                                        { m_InterMTS = b; }
    bool      getUseInterMTS        ()                                      const     { return m_InterMTS; }
    void      setUseLFNST           ( bool b )                                        { m_LFNST = b; }
    bool      getUseLFNST           ()                                      const     { return m_LFNST; }
    void      setUseSMVD(bool b)                                                      { m_SMVD = b; }
    bool      getUseSMVD()                                                  const     { return m_SMVD; }

    void      setLadfEnabled        ( bool b )                                        { m_LadfEnabled = b; }
    bool      getLadfEnabled        ()                                      const     { return m_LadfEnabled; }
    void      setLadfNumIntervals   ( int i )                                         { m_LadfNumIntervals = i; }
    int       getLadfNumIntervals   ()                                      const     { return m_LadfNumIntervals; }
    void      setLadfQpOffset       ( int value, int idx )                            { m_LadfQpOffset[ idx ] = value; }
    int       getLadfQpOffset       ( int idx )                             const     { return m_LadfQpOffset[ idx ]; }
    void      setLadfIntervalLowerBound( int value, int idx )                         { m_LadfIntervalLowerBound[ idx ] = value; }
    int       getLadfIntervalLowerBound( int idx )                          const     { return m_LadfIntervalLowerBound[ idx ]; }
    void      setUseMRL             ( bool b )                                        { m_MRL = b; }
    bool      getUseMRL             ()                                      const     { return m_MRL; }
    void      setUseGeo             ( bool b )                                        { m_Geo = b; }
    bool      getUseGeo             ()                                      const     { return m_Geo; }
    void      setUseMIP             ( bool b )                                        { m_MIP = b; }
    bool      getUseMIP             ()                                      const     { return m_MIP; }

    bool      getUseWP              ()                                      const     { return m_useWeightPred; }
    bool      getUseWPBiPred        ()                                      const     { return m_useWeightedBiPred; }
    void      setUseWP              ( bool b )                                        { m_useWeightPred = b; }
    void      setUseWPBiPred        ( bool b )                                        { m_useWeightedBiPred = b; }
    // void      setChromaQpMappingTableFromParams( const ChromaQpMappingTableParams& params )  { m_chromaQpMappingTable = ChromaQpMappingTable( params ); }
    // void      setChromaQpMappingTableFromParams( const ChromaQpMappingTableParams&& params ) { m_chromaQpMappingTable = ChromaQpMappingTable( std::move( params ) ); }
    // void      deriveChromaQPMappingTables()                                           { m_chromaQpMappingTable.deriveChromaQPMappingTables(); }
    // const ChromaQpMappingTable& getChromaQpMappingTable()                   const     { return m_chromaQpMappingTable;}
    // int       getMappedChromaQpValue(ComponentID compID, int qpVal)         const     { return m_chromaQpMappingTable.getMappedChromaQpValue(compID, qpVal); }
    void      setGDREnabledFlag     ( bool b )                                        { m_GDREnabledFlag = b;    }
    bool      getGDREnabledFlag()                                           const     { return m_GDREnabledFlag; }
    void      setSubLayerParametersPresentFlag(bool flag)                             { m_SubLayerCbpParametersPresentFlag = flag; }
    bool      getSubLayerParametersPresentFlag()                            const     { return m_SubLayerCbpParametersPresentFlag;  }
    bool      getRprEnabledFlag()                                           const     { return m_rprEnabledFlag; }
    void      setRprEnabledFlag( bool flag )                                          { m_rprEnabledFlag = flag; }
    bool      getInterLayerPresentFlag()                                    const     { return m_interLayerPresentFlag; }
    void      setInterLayerPresentFlag( bool b )                                      { m_interLayerPresentFlag = b; }
    bool      getResChangeInClvsEnabledFlag()                               const     { return m_resChangeInClvsEnabledFlag; }
    void      setResChangeInClvsEnabledFlag(bool flag)                                { m_resChangeInClvsEnabledFlag = flag; }

    uint32_t   getLog2ParallelMergeLevelMinus2()                            const     { return m_log2ParallelMergeLevelMinus2; }
    void       setLog2ParallelMergeLevelMinus2(uint32_t mrgLevel)                     { m_log2ParallelMergeLevelMinus2 = mrgLevel; }
    bool       getScalingMatrixForAlternativeColourSpaceDisabledFlag()      const     { return m_scalingMatrixAlternativeColourSpaceDisabledFlag; }
    void       setScalingMatrixForAlternativeColourSpaceDisabledFlag(bool b)          { m_scalingMatrixAlternativeColourSpaceDisabledFlag = b; }
    bool       getScalingMatrixDesignatedColourSpaceFlag()                  const     { return m_scalingMatrixDesignatedColourSpaceFlag; }
    void       setScalingMatrixDesignatedColourSpaceFlag(bool b)                      { m_scalingMatrixDesignatedColourSpaceFlag = b; }
    bool       getDisableScalingMatrixForLfnstBlks()                        const     { return m_disableScalingMatrixForLfnstBlks; }
    void       setDisableScalingMatrixForLfnstBlks(bool flag)                         { m_disableScalingMatrixForLfnstBlks = flag; }
};

class PPS : public BasePS<PPS> {
private:
    int              m_PPSId                             = 0;       // pic_parameter_set_id
    int              m_SPSId                             = 0;       // seq_parameter_set_id
    int              m_layerId                           = 0;
    int              m_picInitQPMinus26                  = 0;
    bool             m_useDQP                            = false;
    bool             m_usePPSChromaTool                  = false;
    bool             m_bConstrainedIntraPred             = false;   // constrained_intra_pred_flag
    bool             m_bSliceChromaQpFlag                = false;   // slicelevel_chroma_qp_flag

    // access channel

    // ChromaQpOffset   m_chromaQpOffset;
    // bool             m_chromaJointCbCrQpOffsetPresentFlag= false;

    // Chroma QP Adjustments
    // int              m_chromaQpOffsetListLen             = 0; // size (excludes the null entry used in the following array).
    // ChromaQpOffset   m_ChromaQpAdjTableIncludingNullEntry[1+MAX_QP_OFFSET_LIST_SIZE]; //!< Array includes entry [0] for the null offset used when cu_chroma_qp_offset_flag=0, and entries [cu_chroma_qp_offset_idx+1...] otherwise

    uint32_t         m_numRefIdxL0DefaultActive          = 1;
    uint32_t         m_numRefIdxL1DefaultActive          = 1;

    bool             m_rpl1IdxPresentFlag                = false;

    bool             m_bUseWeightPred                    = false;   //!< Use of Weighting Prediction (P_SLICE)
    bool             m_useWeightedBiPred                 = false;   //!< Use of Weighting Bi-Prediction (B_SLICE)
    bool             m_OutputFlagPresentFlag             = false;   //!< Indicates the presence of output_flag in slice header
    // uint8_t          m_numSubPics                        = 1;       //!< number of sub-pictures used - must match SPS
    // bool             m_subPicIdMappingPresentFlag        = false;
    // uint32_t         m_subPicIdLen                       = 0;       //!< sub-picture ID length in bits
    // uint16_t         m_subPicId[MAX_NUM_SUB_PICS]        = { 0 };   //!< sub-picture ID for each sub-picture in the sequence
    // bool             m_noPicPartitionFlag                = false;   //!< no picture partitioning flag - single slice, single tile
    uint8_t          m_log2CtuSize                       = 0;       //!< log2 of the CTU size - required to match corresponding value in SPS
    uint8_t          m_ctuSize                           = 0;       //!< CTU size
    uint32_t         m_picWidthInCtu                     = 0;       //!< picture width in units of CTUs
    uint32_t         m_picHeightInCtu                    = 0;       //!< picture height in units of CTUs
    // uint32_t         m_numExpTileCols                    = 0;       //!< number of explicitly specified tile columns
    // uint32_t         m_numExpTileRows                    = 0;       //!< number of explicitly specified tile rows
    // uint32_t         m_numTileCols                       = 0;       //!< number of tile columns
    // uint32_t         m_numTileRows                       = 0;       //!< number of tile rows
    // std::vector<int> m_tileColumnWidth;
    // std::vector<int> m_tileRowHeight;

    // bool                   m_rectSliceFlag            = true;
    // bool                   m_singleSlicePerSubPicFlag = false;   //!< single slice per sub-picture flag
    // uint32_t               m_numSlicesInPic           = 1;       //!< number of rectangular slices in the picture (raster-scan slice specified at slice level)
    // bool                   m_tileIdxDeltaPresentFlag  = false;   //!< tile index delta present flag
    // std::vector<uint32_t>  m_tileColBd;                          //!< tile column left-boundaries in units of CTUs
    // std::vector<uint32_t>  m_tileRowBd;                          //!< tile row top-boundaries in units of CTUs
    // std::vector<uint32_t>  m_ctuToTileCol;                       //!< mapping between CTU horizontal address and tile column index
    // std::vector<uint32_t>  m_ctuToTileRow;                       //!< mapping between CTU vertical address and tile row index
    // std::vector<RectSlice> m_rectSlices;                         //!< list of rectangular slice signalling parameters
    // std::vector<SliceMap>  m_sliceMap;                           //!< list of CTU maps for each slice in the picture
    //std::vector<SubPic>    m_subPics;                            //!< list of subpictures in the picture

    bool             m_cabacInitPresentFlag                = false;

    bool             m_pictureHeaderExtensionPresentFlag   = false;   //< picture header extension flags present in picture headers or not
    bool             m_sliceHeaderExtensionPresentFlag     = false;
    bool             m_loopFilterAcrossTilesEnabledFlag    = false;   //!< loop filtering applied across tiles flag
    bool             m_loopFilterAcrossSlicesEnabledFlag   = false;
    bool             m_deblockingFilterControlPresentFlag  = false;
    bool             m_deblockingFilterOverrideEnabledFlag = false;
    bool             m_ppsDeblockingFilterDisabledFlag     = false;
    int              m_deblockingFilterBetaOffsetDiv2      = 0;       //< beta offset for deblocking filter
    int              m_deblockingFilterTcOffsetDiv2        = 0;       //< tc offset for deblocking filter
    int              m_deblockingFilterCbBetaOffsetDiv2    = 0;       //< beta offset for Cb deblocking filter
    int              m_deblockingFilterCbTcOffsetDiv2      = 0;       //< tc offset for Cb deblocking filter
    int              m_deblockingFilterCrBetaOffsetDiv2    = 0;       //< beta offset for Cr deblocking filter
    int              m_deblockingFilterCrTcOffsetDiv2      = 0;       //< tc offset for Cr deblocking filter
    bool             m_listsModificationPresentFlag        = false;
    int              m_numExtraSliceHeaderBits             = 0;

    bool             m_rplInfoInPhFlag                     = false;
    bool             m_dbfInfoInPhFlag                     = false;
    bool             m_saoInfoInPhFlag                     = false;
    bool             m_alfInfoInPhFlag                     = false;
    bool             m_wpInfoInPhFlag                      = false;
    bool             m_qpDeltaInfoInPhFlag                 = false;
    bool             m_mixedNaluTypesInPicFlag             = false;

    bool             m_scalingListPresentFlag              = false;
    ScalingList      m_scalingList;                       //!< ScalingList class
    uint32_t         m_picWidthInLumaSamples               = 0;
    uint32_t         m_picHeightInLumaSamples              = 0;
    bool             m_conformanceWindowPresentFlag        = false;
    Window           m_conformanceWindow;
    Window           m_scalingWindow;

    bool             m_useWrapAround                       = false;   //< reference wrap around enabled or not
    unsigned         m_picWidthMinusWrapAroundOffset       = 0;       //< pic_width_in_minCbSizeY - wraparound_offset_in_minCbSizeY
    unsigned         m_wrapAroundOffset                    = 0;       //< reference wrap around offset in luma samples

public:
    PPS()  = default;
    ~PPS() = default;

    int                    getPPSId() const                                                 { return m_PPSId;                               }
    void                   setPPSId(int i)                                                  { m_PPSId = i;                                  }
    int                    getSPSId() const                                                 { return m_SPSId;                               }
    void                   setSPSId(int i)                                                  { m_SPSId = i;                                  }
    void                   setLayerId( int i )                                              { m_layerId = i;                                }
    int                    getLayerId()                                               const { return m_layerId;                             }
    int                    getPicInitQPMinus26() const                                      { return  m_picInitQPMinus26;                   }
    void                   setPicInitQPMinus26( int i )                                     { m_picInitQPMinus26 = i;                       }
    bool                   getUseDQP() const                                                { return m_useDQP;                              }
    void                   setUseDQP( bool b )                                              { m_useDQP   = b;                               }
    bool                   getPPSChromaToolFlag()                                     const { return  m_usePPSChromaTool;                   }
    void                   setPPSChromaToolFlag(bool b)                                     { m_usePPSChromaTool = b;                       }
    bool                   getConstrainedIntraPred() const                                  { return  m_bConstrainedIntraPred;              }
    void                   setConstrainedIntraPred( bool b )                                { m_bConstrainedIntraPred = b;                  }
    bool                   getSliceChromaQpFlag() const                                     { return  m_bSliceChromaQpFlag;                 }
    void                   setSliceChromaQpFlag( bool b )                                   { m_bSliceChromaQpFlag = b;                     }


    // bool                   getJointCbCrQpOffsetPresentFlag() const                          { return m_chromaJointCbCrQpOffsetPresentFlag;   }
    // void                   setJointCbCrQpOffsetPresentFlag(bool b)                          { m_chromaJointCbCrQpOffsetPresentFlag = b;      }

    // void                   setQpOffset( ComponentID compID, int val )                       { m_chromaQpOffset.set( compID, val ); }
    // int                    getQpOffset( ComponentID compID ) const                          { return m_chromaQpOffset.get( compID ); }

    // bool                   getCuChromaQpOffsetEnabledFlag() const                           { return getChromaQpOffsetListLen()>0;            }
    // int                    getChromaQpOffsetListLen() const                                 { return m_chromaQpOffsetListLen;                 }
    // void                   clearChromaQpOffsetList()                                        { m_chromaQpOffsetListLen = 0;                    }

    // const ChromaQpOffset&  getChromaQpOffsetListEntry( int cuChromaQpOffsetIdxPlus1 ) const
    // {
    //     CHECK(cuChromaQpOffsetIdxPlus1 >= m_chromaQpOffsetListLen+1, "Invalid chroma QP offset");
    //     return m_ChromaQpAdjTableIncludingNullEntry[cuChromaQpOffsetIdxPlus1]; // Array includes entry [0] for the null offset used when cu_chroma_qp_offset_flag=0, and entries [cu_chroma_qp_offset_idx+1...] otherwise
    // }

    // void                   setChromaQpOffsetListEntry( int cuChromaQpOffsetIdxPlus1, int cbOffset, int crOffset, int jointCbCrOffset )
    // {
    //     CHECK(cuChromaQpOffsetIdxPlus1 == 0 || cuChromaQpOffsetIdxPlus1 > MAX_QP_OFFSET_LIST_SIZE, "Invalid chroma QP offset");
    //     // Array includes entry [0] for the null offset used when cu_chroma_qp_offset_flag=0, and entries [cu_chroma_qp_offset_idx+1...] otherwise
    //     m_ChromaQpAdjTableIncludingNullEntry[cuChromaQpOffsetIdxPlus1].CbOffset        = cbOffset;
    //     m_ChromaQpAdjTableIncludingNullEntry[cuChromaQpOffsetIdxPlus1].CrOffset        = crOffset;
    //     m_ChromaQpAdjTableIncludingNullEntry[cuChromaQpOffsetIdxPlus1].JointCbCrOffset = jointCbCrOffset;
    //     m_chromaQpOffsetListLen                                                        = std::max( m_chromaQpOffsetListLen, cuChromaQpOffsetIdxPlus1 );
    // }
    
    void                   setNumRefIdxL0DefaultActive(uint32_t ui)                         { m_numRefIdxL0DefaultActive=ui;                }
    uint32_t               getNumRefIdxL0DefaultActive() const                              { return m_numRefIdxL0DefaultActive;            }
    void                   setNumRefIdxL1DefaultActive(uint32_t ui)                         { m_numRefIdxL1DefaultActive=ui;                }
    uint32_t               getNumRefIdxL1DefaultActive() const                              { return m_numRefIdxL1DefaultActive;            }

    void                   setRpl1IdxPresentFlag(bool isPresent)                            { m_rpl1IdxPresentFlag = isPresent;             }
    uint32_t               getRpl1IdxPresentFlag() const                                    { return m_rpl1IdxPresentFlag;                  }

    bool                   getUseWP() const                                                 { return m_bUseWeightPred;                      }
    bool                   getWPBiPred() const                                              { return m_useWeightedBiPred;                   }
    void                   setUseWP( bool b )                                               { m_bUseWeightPred = b;                         }
    void                   setWPBiPred( bool b )                                            { m_useWeightedBiPred = b;                      }

    void                   setUseWrapAround(bool b)                                         { m_useWrapAround = b;                          }
    bool                   getUseWrapAround() const                                         { return m_useWrapAround;                       }
    void                   setPicWidthMinusWrapAroundOffset(unsigned offset)                { m_picWidthMinusWrapAroundOffset = offset;     }
    unsigned               getPicWidthMinusWrapAroundOffset() const                         { return m_picWidthMinusWrapAroundOffset;       }
    void                   setWrapAroundOffset(unsigned offset)                             { m_wrapAroundOffset = offset;                  }
    unsigned               getWrapAroundOffset() const                                      { return m_wrapAroundOffset;                    }
    void                   setOutputFlagPresentFlag( bool b )                               { m_OutputFlagPresentFlag = b;                  }
    bool                   getOutputFlagPresentFlag() const                                 { return m_OutputFlagPresentFlag;               }
    // void                   setNumSubPics( uint8_t u )                                       { m_numSubPics = u;                             }
    // uint8_t                getNumSubPics( ) const                                           { return  m_numSubPics;                         }
    // void                   setSubPicIdMappingPresentFlag( bool b )                          { m_subPicIdMappingPresentFlag = b;             }
    // bool                   getSubPicIdMappingPresentFlag() const                            { return m_subPicIdMappingPresentFlag;          }
    // void                   setSubPicIdLen( uint32_t u )                                     { CHECK( u > 16, "Sub-picture id len exceeds valid range" ); m_subPicIdLen = u;                   }
    // uint32_t               getSubPicIdLen() const                                           { return  m_subPicIdLen;                                                                          }
    // void                   setSubPicId( int i, uint16_t u )                                 { CHECK( i >= MAX_NUM_SUB_PICS, "Sub-picture index exceeds valid range" ); m_subPicId[i] = u;     }
    // uint16_t               getSubPicId( int i ) const                                       { CHECK( i >= MAX_NUM_SUB_PICS, "Sub-picture index exceeds valid range" ); return  m_subPicId[i]; }
    // uint32_t               getSubPicIdxFromSubPicId( uint32_t subPicId ) const;
    // void                   setNoPicPartitionFlag( bool b )                                  { m_noPicPartitionFlag = b;                     }
    // bool                   getNoPicPartitionFlag( ) const                                   { return  m_noPicPartitionFlag;                 }
    void                   setLog2CtuSize( uint8_t u )                                      { m_log2CtuSize = u; m_ctuSize = 1 << m_log2CtuSize;
                                                                                                m_picWidthInCtu = (m_picWidthInLumaSamples  + m_ctuSize - 1) / m_ctuSize;
                                                                                                m_picHeightInCtu = (m_picHeightInLumaSamples  + m_ctuSize - 1) / m_ctuSize; }
    uint8_t                getLog2CtuSize( ) const                                          { return  m_log2CtuSize;                        }
    uint8_t                getCtuSize( ) const                                              { return  m_ctuSize;                            }
    uint8_t                getPicWidthInCtu( ) const                                        { return  m_picWidthInCtu;                      }
    uint8_t                getPicHeightInCtu( ) const                                       { return  m_picHeightInCtu;                     }
    // void                   setNumExpTileColumns( uint32_t u )                               { m_numExpTileCols = u;                         }
    // uint32_t               getNumExpTileColumns( ) const                                    { return  m_numExpTileCols;                     }
    // void                   setNumExpTileRows( uint32_t u )                                  { m_numExpTileRows = u;                         }
    // uint32_t               getNumExpTileRows( ) const                                       { return  m_numExpTileRows;                     }
    // void                   setNumTileColumns( uint32_t u )                                  { m_numTileCols = u;                            }
    // uint32_t               getNumTileColumns( ) const                                       { return  m_numTileCols;                        }
    // void                   setNumTileRows( uint32_t u )                                     { m_numTileRows = u;                            }
    // uint32_t               getNumTileRows( ) const                                          { return  m_numTileRows;                        }
    // void                   addTileColumnWidth( uint32_t u )                                 { CHECK( m_tileColumnWidth.size()  >= MAX_TILE_COLS, "Number of tile columns exceeds valid range" ); m_tileColumnWidth.push_back(u);    }
    // void                   addTileRowHeight( uint32_t u )                                   { m_tileRowHeight.push_back(u);   }

    // void                   setTileColumnWidth(const std::vector<int>& columnWidth )         { m_tileColumnWidth = columnWidth;              }
    // uint32_t               getTileColumnWidth(uint32_t columnIdx) const                     { return  m_tileColumnWidth[columnIdx];         }
    // void                   setTileRowHeight(const std::vector<int>& rowHeight)              { m_tileRowHeight = rowHeight;                  }
    // uint32_t               getTileRowHeight(uint32_t rowIdx) const                          { return m_tileRowHeight[rowIdx];               }
    // uint32_t               getNumTiles() const                                              { return m_numTileCols * m_numTileRows;        }
    // uint32_t               ctuToTileCol( int ctuX ) const                                   { CHECK( ctuX >= m_ctuToTileCol.size(), "CTU address index exceeds valid range" ); return  m_ctuToTileCol[ctuX];                  }
    // uint32_t               ctuToTileRow( int ctuY ) const                                   { CHECK( ctuY >= m_ctuToTileRow.size(), "CTU address index exceeds valid range" ); return  m_ctuToTileRow[ctuY];                  }
    // uint32_t               ctuToTileColBd( int ctuX ) const                                 { return  getTileColumnBd(ctuToTileCol( ctuX ));                                                                                  }
    // uint32_t               ctuToTileRowBd( int ctuY ) const                                 { return  getTileRowBd(ctuToTileRow( ctuY ));                                                                                     }
    // bool                   ctuIsTileColBd( int ctuX ) const                                 { return  ctuX == ctuToTileColBd( ctuX );                                                                                         }
    // bool                   ctuIsTileRowBd( int ctuY ) const                                 { return  ctuY == ctuToTileRowBd( ctuY );                                                                                         }
    // uint32_t               getTileIdx( uint32_t ctuX, uint32_t ctuY ) const                 { return (ctuToTileRow( ctuY ) * getNumTileColumns()) + ctuToTileCol( ctuX );                                                     }
    // uint32_t               getTileIdx( uint32_t ctuRsAddr) const                            { return getTileIdx( ctuRsAddr % m_picWidthInCtu,  ctuRsAddr / m_picWidthInCtu );                                                 }
    // uint32_t               getTileIdx( const Position& pos ) const                          { return getTileIdx( pos.x / m_ctuSize, pos.y / m_ctuSize );                                                                      }
    // bool                   getRectSliceFlag() const                                         { return m_rectSliceFlag;                       }
    // void                   setRectSliceFlag(bool val)                                       { m_rectSliceFlag = val;                        }
    // void                   setSingleSlicePerSubPicFlag( bool b )                            { m_singleSlicePerSubPicFlag = b;                                                                                                 }
    // bool                   getSingleSlicePerSubPicFlag( ) const                             { return  m_singleSlicePerSubPicFlag;                                                                                             }
    // void                   setNumSlicesInPic( uint32_t u )                                  { CHECK( u > MAX_SLICES, "Number of slices in picture exceeds valid range" ); m_numSlicesInPic = u;                               }
    // uint32_t               getNumSlicesInPic( ) const                                       { return  m_numSlicesInPic;                                                                                                       }
    // void                   setTileIdxDeltaPresentFlag( bool b )                             { m_tileIdxDeltaPresentFlag = b;                                                                                                  }
    // bool                   getTileIdxDeltaPresentFlag( ) const                              { return  m_tileIdxDeltaPresentFlag;                                                                                              }
    // uint32_t               getTileColumnBd( int idx ) const                                 { CHECK( idx >= m_tileColBd.size(), "Tile column index exceeds valid range" );                    return  m_tileColBd[idx];       }
    // uint32_t               getTileRowBd( int idx ) const                                    { CHECK( idx >= m_tileRowBd.size(), "Tile row index exceeds valid range" );                       return  m_tileRowBd[idx];       }
    // void                   setSliceWidthInTiles( int idx, uint32_t u )                      { CHECK( idx >= m_numSlicesInPic, "Slice index exceeds valid range" );    m_rectSlices[idx].setSliceWidthInTiles( u );            }
    // uint32_t               getSliceWidthInTiles( int idx ) const                            { CHECK( idx >= m_numSlicesInPic, "Slice index exceeds valid range" );    return  m_rectSlices[idx].getSliceWidthInTiles( );      }
    // void                   setSliceHeightInTiles( int idx, uint32_t u )                     { CHECK( idx >= m_numSlicesInPic, "Slice index exceeds valid range" );    m_rectSlices[idx].setSliceHeightInTiles( u );           }
    // uint32_t               getSliceHeightInTiles( int idx ) const                           { CHECK( idx >= m_numSlicesInPic, "Slice index exceeds valid range" );    return  m_rectSlices[idx].getSliceHeightInTiles( );     }
    // void                   setNumSlicesInTile( int idx, uint32_t u )                        { CHECK( idx >= m_numSlicesInPic, "Slice index exceeds valid range" );    m_rectSlices[idx].setNumSlicesInTile( u );              }
    // uint32_t               getNumSlicesInTile( int idx ) const                              { CHECK( idx >= m_numSlicesInPic, "Slice index exceeds valid range" );    return  m_rectSlices[idx].getNumSlicesInTile( );        }
    // void                   setSliceHeightInCtu( int idx, uint32_t u )                       { CHECK( idx >= m_numSlicesInPic, "Slice index exceeds valid range" );    m_rectSlices[idx].setSliceHeightInCtu( u );             }
    // uint32_t               getSliceHeightInCtu( int idx ) const                             { CHECK( idx >= m_numSlicesInPic, "Slice index exceeds valid range" );    return  m_rectSlices[idx].getSliceHeightInCtu( );       }
    // void                   setSliceTileIdx(  int idx, uint32_t u )                          { CHECK( idx >= m_numSlicesInPic, "Slice index exceeds valid range" );    m_rectSlices[idx].setTileIdx( u );                      }
    // uint32_t               getSliceTileIdx( int idx ) const                                 { CHECK( idx >= m_numSlicesInPic, "Slice index exceeds valid range" );    return  m_rectSlices[idx].getTileIdx( );                }
    // void                   resetTileSliceInfo();
    // void                   initTiles();
    // void                   initRectSlices();
    // void                   initRectSliceMap(const SPS *sps);
    // const SubPic&          getSubPic(uint32_t idx) const                                    { return m_subPics[idx]; }
    // void                   initSubPic(const SPS &sps);
    // const SubPic&          getSubPicFromPos(const Position& pos)  const;
    // const SubPic&          getSubPicFromCU (const CodingUnit& cu) const;
    // void                   checkSliceMap();

    // SliceMap               getSliceMap( int idx ) const                                     { CHECK( idx >= m_numSlicesInPic, "Slice index exceeds valid range" );    return m_sliceMap[idx];                             }

    void                   setCabacInitPresentFlag( bool flag )                             { m_cabacInitPresentFlag = flag;                }
    bool                   getCabacInitPresentFlag() const                                  { return m_cabacInitPresentFlag;                }
    void                   setDeblockingFilterControlPresentFlag( bool val )                { m_deblockingFilterControlPresentFlag = val;   }
    bool                   getDeblockingFilterControlPresentFlag() const                    { return m_deblockingFilterControlPresentFlag;  }
    void                   setDeblockingFilterOverrideEnabledFlag( bool val )               { m_deblockingFilterOverrideEnabledFlag = val;  }
    bool                   getDeblockingFilterOverrideEnabledFlag() const                   { return m_deblockingFilterOverrideEnabledFlag; }
    void                   setPPSDeblockingFilterDisabledFlag(bool val)                     { m_ppsDeblockingFilterDisabledFlag = val;      } //!< set offset for deblocking filter disabled
    bool                   getPPSDeblockingFilterDisabledFlag() const                       { return m_ppsDeblockingFilterDisabledFlag;     } //!< get offset for deblocking filter disabled
    void                   setDeblockingFilterBetaOffsetDiv2(int val)                       { m_deblockingFilterBetaOffsetDiv2 = val;       } //!< set beta offset for deblocking filter
    int                    getDeblockingFilterBetaOffsetDiv2() const                        { return m_deblockingFilterBetaOffsetDiv2;      } //!< get beta offset for deblocking filter
    void                   setDeblockingFilterTcOffsetDiv2(int val)                         { m_deblockingFilterTcOffsetDiv2 = val;         } //!< set tc offset for deblocking filter
    int                    getDeblockingFilterTcOffsetDiv2() const                          { return m_deblockingFilterTcOffsetDiv2;        } //!< get tc offset for deblocking filter
    void                   setDeblockingFilterCbBetaOffsetDiv2(int val)                     { m_deblockingFilterCbBetaOffsetDiv2 = val;     } //!< set beta offset for Cb deblocking filter
    int                    getDeblockingFilterCbBetaOffsetDiv2() const                      { return m_deblockingFilterCbBetaOffsetDiv2;    } //!< get beta offset for Cb deblocking filter
    void                   setDeblockingFilterCbTcOffsetDiv2(int val)                       { m_deblockingFilterCbTcOffsetDiv2 = val;       } //!< set tc offset for Cb deblocking filter
    int                    getDeblockingFilterCbTcOffsetDiv2() const                        { return m_deblockingFilterCbTcOffsetDiv2;      } //!< get tc offset for Cb deblocking filter
    void                   setDeblockingFilterCrBetaOffsetDiv2(int val)                     { m_deblockingFilterCrBetaOffsetDiv2 = val;     } //!< set beta offset for Cr deblocking filter
    int                    getDeblockingFilterCrBetaOffsetDiv2() const                      { return m_deblockingFilterCrBetaOffsetDiv2;    } //!< get beta offset for Cr deblocking filter
    void                   setDeblockingFilterCrTcOffsetDiv2(int val)                       { m_deblockingFilterCrTcOffsetDiv2 = val;       } //!< set tc offset for Cr deblocking filter
    int                    getDeblockingFilterCrTcOffsetDiv2() const                        { return m_deblockingFilterCrTcOffsetDiv2;      } //!< get tc offset for Cr deblocking filter
    bool                   getListsModificationPresentFlag() const                          { return m_listsModificationPresentFlag;        }
    void                   setListsModificationPresentFlag( bool b )                        { m_listsModificationPresentFlag = b;           }
    int                    getNumExtraSliceHeaderBits() const                               { return m_numExtraSliceHeaderBits;             }
    void                   setNumExtraSliceHeaderBits(int i)                                { m_numExtraSliceHeaderBits = i;                }
    void                   setLoopFilterAcrossTilesEnabledFlag( bool b )                    { m_loopFilterAcrossTilesEnabledFlag = b;                                                                                         }
    bool                   getLoopFilterAcrossTilesEnabledFlag( ) const                     { return  m_loopFilterAcrossTilesEnabledFlag;                                                                                     }
    void                   setLoopFilterAcrossSlicesEnabledFlag( bool bValue )              { m_loopFilterAcrossSlicesEnabledFlag = bValue; }
    bool                   getLoopFilterAcrossSlicesEnabledFlag() const                     { return m_loopFilterAcrossSlicesEnabledFlag;   }
    bool                   getPictureHeaderExtensionPresentFlag() const                     { return m_pictureHeaderExtensionPresentFlag;     }
    void                   setPictureHeaderExtensionPresentFlag(bool val)                   { m_pictureHeaderExtensionPresentFlag = val;      }
    bool                   getSliceHeaderExtensionPresentFlag() const                       { return m_sliceHeaderExtensionPresentFlag;     }
    void                   setSliceHeaderExtensionPresentFlag(bool val)                     { m_sliceHeaderExtensionPresentFlag = val;      }

    void                   setRplInfoInPhFlag(bool flag)                                    { m_rplInfoInPhFlag = flag;                     }
    bool                   getRplInfoInPhFlag() const                                       { return m_rplInfoInPhFlag;                     }
    void                   setDbfInfoInPhFlag(bool flag)                                    { m_dbfInfoInPhFlag = flag;                     }
    bool                   getDbfInfoInPhFlag() const                                       { return m_dbfInfoInPhFlag;                     }
    void                   setSaoInfoInPhFlag(bool flag)                                    { m_saoInfoInPhFlag = flag;                     }
    bool                   getSaoInfoInPhFlag() const                                       { return m_saoInfoInPhFlag;                     }
    void                   setAlfInfoInPhFlag(bool flag)                                    { m_alfInfoInPhFlag = flag;                     }
    bool                   getAlfInfoInPhFlag() const                                       { return m_alfInfoInPhFlag;                     }
    void                   setWpInfoInPhFlag(bool flag)                                     { m_wpInfoInPhFlag = flag;                      }
    bool                   getWpInfoInPhFlag() const                                        { return m_wpInfoInPhFlag;                      }
    void                   setQpDeltaInfoInPhFlag(bool flag)                                { m_qpDeltaInfoInPhFlag = flag;                 }
    bool                   getQpDeltaInfoInPhFlag() const                                   { return m_qpDeltaInfoInPhFlag; }

    bool                   getScalingListPresentFlag() const                                { return m_scalingListPresentFlag;              }
    void                   setScalingListPresentFlag( bool b )                              { m_scalingListPresentFlag  = b;                }
    ScalingList&           getScalingList()                                                 { return m_scalingList;                         }
    const ScalingList&     getScalingList() const                                           { return m_scalingList;                         }

    void                    setPicWidthInLumaSamples( uint32_t u )                          { m_picWidthInLumaSamples = u; }
    uint32_t                getPicWidthInLumaSamples() const                                { return  m_picWidthInLumaSamples; }
    void                    setPicHeightInLumaSamples( uint32_t u )                         { m_picHeightInLumaSamples = u; }
    uint32_t                getPicHeightInLumaSamples() const                               { return  m_picHeightInLumaSamples; }

    void                    setConformanceWindowPresentFlag(bool b)                         { m_conformanceWindowPresentFlag = b;           }
    bool                    getConformanceWindowPresentFlag() const                         { return m_conformanceWindowPresentFlag;        }
    Window&                 getConformanceWindow()                                          { return  m_conformanceWindow; }
    const Window&           getConformanceWindow() const                                    { return  m_conformanceWindow; }
    void                    setConformanceWindow( const Window& conformanceWindow )         { m_conformanceWindow = conformanceWindow; }
    Window&                 getScalingWindow()                                              { return  m_scalingWindow; }
    const Window&           getScalingWindow()                                        const { return  m_scalingWindow; }
    void                    setScalingWindow( const Window& scalingWindow )                 { m_scalingWindow = scalingWindow; }
    int                     getMixedNaluTypesInPicFlag() const                              { return m_mixedNaluTypesInPicFlag; }
    void                    setMixedNaluTypesInPicFlag( const bool flag )                   { m_mixedNaluTypesInPicFlag = flag; }
};

class PicHeader {
private:
    bool                        m_valid                                         = false;   //!< picture header is valid yet or not
    Picture*                    m_pcPic                                         = nullptr; //!< pointer to picture structure
    int                         m_pocLsb                                        = -1;      //!< least significant bits of picture order count
    bool                        m_nonReferencePictureFlag                       = false;   //!< non-reference picture flag
    bool                        m_gdrOrIrapPicFlag                              = false;   //!< gdr or irap picture flag
    bool                        m_pocMsbPresentFlag                             = false;  //!< ph_poc_msb_present_flag
    int                         m_pocMsbVal                                     = 0;      //!< poc_msb_val
    bool                        m_gdrPicFlag                                    = false;   //!< gradual decoding refresh picture flag
    bool                        m_handleCraAsCvsStartFlag                       = false;                                //!< HandleCraAsCvsStartFlag
    bool                        m_handleGdrAsCvsStartFlag                       = false;                                //!< HandleGdrAsCvsStartFlag
    uint32_t                    m_recoveryPocCnt                                = 0;       //!< recovery POC count
    bool                        m_noOutputBeforeRecoveryFlag                    = false;                             //!< NoOutputBeforeRecoveryFlag
    int                         m_spsId                                         = -1;      //!< sequence parameter set ID
    int                         m_ppsId                                         = -1;      //!< picture parameter set ID
    bool                        m_virtualBoundariesPresentFlag                  = false;   //!< disable loop filtering across virtual boundaries
    unsigned                    m_numVerVirtualBoundaries                       = 0;       //!< number of vertical virtual boundaries
    unsigned                    m_numHorVirtualBoundaries                       = 0;       //!< number of horizontal virtual boundaries
    unsigned                    m_virtualBoundariesPosX[3]                      = { 0, 0, 0 }; //!< horizontal virtual boundary positions
    unsigned                    m_virtualBoundariesPosY[3]                      = { 0, 0, 0 }; //!< vertical virtual boundary positions
    bool                        m_picOutputFlag                                 = true;    //!< picture output flag
    ReferencePictureList        m_RPL[2];                                                  //!< RPL for L0/L1 when present in picture header
    int                         m_RPLIdx[2]                                     = { 0, 0 };//!< index of used RPL L0/L1 in the SPS or -1 for local RPL in the picture header
    bool                        m_picInterSliceAllowedFlag                      = false;  //!< inter slice allowed flag in PH
    bool                        m_picIntraSliceAllowedFlag                      = true;   //!< intra slice allowed flag in PH
    bool                        m_splitConsOverrideFlag                         = false;  //!< partitioning constraint override flag
    uint32_t                    m_cuQpDeltaSubdivIntra                          = 0;      //!< CU QP delta maximum subdivision for intra slices
    uint32_t                    m_cuQpDeltaSubdivInter                          = 0;      //!< CU QP delta maximum subdivision for inter slices
    uint32_t                    m_cuChromaQpOffsetSubdivIntra                   = 0;      //!< CU chroma QP offset maximum subdivision for intra slices
    uint32_t                    m_cuChromaQpOffsetSubdivInter                   = 0;      //!< CU chroma QP offset maximum subdivision for inter slices
    bool                        m_enableTMVPFlag                                = false;  //!< enable temporal motion vector prediction
    bool                        m_picColFromL0Flag                              = false;  //!< syntax element collocated_from_l0_flag
    uint32_t                    m_colRefIdx                                     = 0;
    bool                        m_mvdL1ZeroFlag                                 = true;   //!< L1 MVD set to zero flag
    //uint32_t                    m_maxNumAffineMergeCand                         = AFFINE_MRG_MAX_NUM_CANDS; //!< max number of sub-block merge candidates
    bool                        m_disFracMMVD                                   = false;  //!< fractional MMVD offsets disabled flag
    bool                        m_disBdofFlag                                   = false;  //!< picture level BDOF disable flag
    bool                        m_disDmvrFlag                                   = false;  //!< picture level DMVR disable flag
    bool                        m_disProfFlag                                   = false;  //!< picture level PROF disable flag
    uint32_t                    m_maxNumGeoCand                                 = 0;      //!< max number of geometric merge candidates
    //uint32_t                    m_maxNumIBCMergeCand                            = IBC_MRG_MAX_NUM_CANDS; //!< max number of IBC merge candidates
    bool                        m_jointCbCrSignFlag                             = false;  //!< joint Cb/Cr residual sign flag
    int                         m_qpDelta                                       = 0;      //!< value of Qp delta
    bool                        m_saoEnabledFlag[MAX_NUM_CHANNEL_TYPE]          = { false, false }; //!< sao enabled flags for each channel
    bool                        m_alfEnabledFlag[MAX_NUM_COMPONENT]             = { false, false, false }; //!< alf enabled flags for each component
    int                         m_numAlfAps                                     = 0;      //!< number of alf aps active for the picture
    //AlfApsIdVec                 m_alfApsId;                                               //!< list of alf aps for the picture
    bool                        m_ccalfEnabledFlag[MAX_NUM_COMPONENT]           = { false, false, false };
    int                         m_ccalfCbApsId                                  = -1;
    int                         m_ccalfCrApsId                                  = -1;
    int                         m_alfChromaApsId                                = 0;      //!< chroma alf aps ID
    bool                        m_deblockingFilterOverrideFlag                  = false;  //!< deblocking filter override controls enabled
    bool                        m_deblockingFilterDisable                       = false;  //!< deblocking filter disabled flag
    int                         m_deblockingFilterBetaOffsetDiv2                = 0;      //!< beta offset for deblocking filter
    int                         m_deblockingFilterTcOffsetDiv2                  = 0;      //!< tc offset for deblocking filter
    int                         m_deblockingFilterCbBetaOffsetDiv2              = 0;                       //!< beta offset for deblocking filter
    int                         m_deblockingFilterCbTcOffsetDiv2                = 0;                         //!< tc offset for deblocking filter
    int                         m_deblockingFilterCrBetaOffsetDiv2              = 0;                       //!< beta offset for deblocking filter
    int                         m_deblockingFilterCrTcOffsetDiv2                = 0;                         //!< tc offset for deblocking filter
    bool                        m_lmcsEnabledFlag                               = false;  //!< lmcs enabled flag
    int                         m_lmcsApsId                                     = -1;     //!< lmcs APS ID
    //std::shared_ptr<const APS>  m_lmcsAps                                       = nullptr; //!< lmcs APS
    bool                        m_lmcsChromaResidualScaleFlag                   = false;  //!< lmcs chroma residual scale flag
    bool                        m_explicitScalingListEnabledFlag                = false;  //!< explicit quantization scaling list enabled
    int                         m_scalingListApsId                              = -1;     //!< quantization scaling list APS ID
    //std::shared_ptr<const APS>  m_scalingListAps                                = nullptr; //!< quantization scaling list APS
    PartitionConstraints        m_minQT                                         = PartitionConstraints{ 0, 0, 0 }; //!< minimum quad-tree size  0: I slice luma; 1: P/B slice luma; 2: I slice chroma
    PartitionConstraints        m_maxMTTHierarchyDepth                          = PartitionConstraints{ 0, 0, 0 }; //!< maximum MTT depth
    PartitionConstraints        m_maxBTSize                                     = PartitionConstraints{ 0, 0, 0 }; //!< maximum BT size
    PartitionConstraints        m_maxTTSize                                     = PartitionConstraints{ 0, 0, 0 }; //!< maximum TT size

    //WPScalingParam              m_weightPredTable[NUM_REF_PIC_LIST_01][MAX_NUM_REF][MAX_NUM_COMPONENT];   // [REF_PIC_LIST_0 or REF_PIC_LIST_1][refIdx][0:Y, 1:U, 2:V]
    int                         m_numL0Weights                                  = 0;  //!< number of weights for L0 list
    int                         m_numL1Weights                                  = 0;  //!< number of weights for L1 list

public:
                                PicHeader() = default;
                                ~PicHeader() { /*m_alfApsId.resize(0); */}
    //  void                        initPicHeader();
    bool                        isValid() const                                           { return m_valid;                                                                              }
    void                        setValid()                                                { m_valid = true;                                                                              }
    void                        setPic( Picture* p )                                      { m_pcPic = p;                                                                                 }
    Picture*                    getPic()                                                  { return m_pcPic;                                                                              }
    const Picture*              getPic() const                                            { return m_pcPic;                                                                              }
    void                        setPocLsb(int i)                                          { m_pocLsb = i;                                                                                }
    int                         getPocLsb() const                                         { return m_pocLsb;                                                                             }
    void                        setNonReferencePictureFlag( bool b )                      { m_nonReferencePictureFlag = b;                                                               }
    bool                        getNonReferencePictureFlag() const                        { return m_nonReferencePictureFlag;                                                            }
    void                        setGdrOrIrapPicFlag( bool b )                             { m_gdrOrIrapPicFlag = b;                                                                      }
    bool                        getGdrOrIrapPicFlag() const                               { return m_gdrOrIrapPicFlag;                                                                   }
    void                        setGdrPicFlag( bool b )                                   { m_gdrPicFlag = b;                                                                            }
    bool                        getGdrPicFlag() const                                     { return m_gdrPicFlag;                                                                         }
    void                        setRecoveryPocCnt( uint32_t u )                           { m_recoveryPocCnt = u;                                                                        }
    uint32_t                    getRecoveryPocCnt() const                                 { return m_recoveryPocCnt;                                                                     }
    void                        setSPSId( uint32_t u )                                    { m_spsId = u;                                                                                 }
    uint32_t                    getSPSId() const                                          { return m_spsId;                                                                              }
    void                        setPPSId( uint32_t u )                                    { m_ppsId = u;                                                                                 }
    uint32_t                    getPPSId() const                                          { return m_ppsId;                                                                              }
    void                        setPocMsbPresentFlag(bool b)                              { m_pocMsbPresentFlag = b;                                                                     }
    bool                        getPocMsbPresentFlag() const                              { return m_pocMsbPresentFlag;                                                                  }
    void                        setPocMsbVal(int i)                                       { m_pocMsbVal = i;                                                                             }
    int                         getPocMsbVal() const                                      { return m_pocMsbVal;                                                                          }
    void                        setVirtualBoundariesPresentFlag( bool b )                 { m_virtualBoundariesPresentFlag = b;                                                          }
    bool                        getVirtualBoundariesPresentFlag() const                   { return m_virtualBoundariesPresentFlag;                                                       }
    void                        setNumVerVirtualBoundaries(unsigned u)                    { m_numVerVirtualBoundaries = u;                                                               }
    unsigned                    getNumVerVirtualBoundaries() const                        { return m_numVerVirtualBoundaries;                                                            }
    void                        setNumHorVirtualBoundaries(unsigned u)                    { m_numHorVirtualBoundaries = u;                                                               }
    unsigned                    getNumHorVirtualBoundaries() const                        { return m_numHorVirtualBoundaries;                                                            }
    void                        setVirtualBoundariesPosX(unsigned u, unsigned idx)        { CHECK( idx >= 3, "boundary index exceeds valid range" ); m_virtualBoundariesPosX[idx] = u;   }
    unsigned                    getVirtualBoundariesPosX(unsigned idx) const              { CHECK( idx >= 3, "boundary index exceeds valid range" ); return m_virtualBoundariesPosX[idx];}
    void                        setVirtualBoundariesPosY(unsigned u, unsigned idx)        { CHECK( idx >= 3, "boundary index exceeds valid range" ); m_virtualBoundariesPosY[idx] = u;   }
    unsigned                    getVirtualBoundariesPosY(unsigned idx) const              { CHECK( idx >= 3, "boundary index exceeds valid range" ); return m_virtualBoundariesPosY[idx];}
    void                        setPicOutputFlag( bool b )                                { m_picOutputFlag = b;                                                                         }
    bool                        getPicOutputFlag() const                                  { return m_picOutputFlag;                                                                      }
    void                        clearRPL( RefPicList l )                                  { m_RPL[l].clear();                                                                            }
    void                        setRPL( RefPicList l, const ReferencePictureList& rpl )   { m_RPL[l] = rpl;                                                                              }
    ReferencePictureList*       getRPL( RefPicList l )                                    { return &m_RPL[l];                                                                            }
    const ReferencePictureList* getRPL( RefPicList l ) const                              { return &m_RPL[l];                                                                            }
    void                        setRPLIdx( RefPicList l, int RPLIdx)                      { m_RPLIdx[l] = RPLIdx;                                                                        }
    int                         getRPLIdx( RefPicList l ) const                           { return m_RPLIdx[l];                                                                          }
    void                        setPicInterSliceAllowedFlag(bool b)                       { m_picInterSliceAllowedFlag = b; }
    bool                        getPicInterSliceAllowedFlag() const                       { return m_picInterSliceAllowedFlag; }
    void                        setPicIntraSliceAllowedFlag(bool b)                       { m_picIntraSliceAllowedFlag = b; }
    bool                        getPicIntraSliceAllowedFlag() const                       { return m_picIntraSliceAllowedFlag; }
    void                        setSplitConsOverrideFlag( bool b )                        { m_splitConsOverrideFlag = b;                                                                 }
    bool                        getSplitConsOverrideFlag() const                          { return m_splitConsOverrideFlag;                                                              }  
    void                        setCuQpDeltaSubdivIntra( uint32_t u )                     { m_cuQpDeltaSubdivIntra = u;                                                                  }
    uint32_t                    getCuQpDeltaSubdivIntra() const                           { return m_cuQpDeltaSubdivIntra;                                                               }
    void                        setCuQpDeltaSubdivInter( uint32_t u )                     { m_cuQpDeltaSubdivInter = u;                                                                  }
    uint32_t                    getCuQpDeltaSubdivInter() const                           { return m_cuQpDeltaSubdivInter;                                                               }
    void                        setCuChromaQpOffsetSubdivIntra( uint32_t u )              { m_cuChromaQpOffsetSubdivIntra = u;                                                           }
    uint32_t                    getCuChromaQpOffsetSubdivIntra() const                    { return m_cuChromaQpOffsetSubdivIntra;                                                        }
    void                        setCuChromaQpOffsetSubdivInter( uint32_t u )              { m_cuChromaQpOffsetSubdivInter = u;                                                           }
    uint32_t                    getCuChromaQpOffsetSubdivInter() const                    { return m_cuChromaQpOffsetSubdivInter;                                                        }
    void                        setEnableTMVPFlag( bool b )                               { m_enableTMVPFlag = b;                                                                        }
    bool                        getEnableTMVPFlag() const                                 { return m_enableTMVPFlag;                                                                     }
    void                        setPicColFromL0Flag(bool val)                             { m_picColFromL0Flag = val;                                                                    }
    bool                        getPicColFromL0Flag() const                               { return m_picColFromL0Flag;                                                                   }
    void                        setColRefIdx( uint32_t refIdx)                            { m_colRefIdx = refIdx;                                                                        }
    uint32_t                    getColRefIdx() const                                      { return m_colRefIdx;                                                                          }
    void                        setMvdL1ZeroFlag( bool b )                                { m_mvdL1ZeroFlag = b;                                                                         }
    bool                        getMvdL1ZeroFlag() const                                  { return m_mvdL1ZeroFlag;                                                                      }  
    // void                        setMaxNumAffineMergeCand( uint32_t val )                  { m_maxNumAffineMergeCand = val;                                                               }
    // uint32_t                    getMaxNumAffineMergeCand() const                          { return m_maxNumAffineMergeCand;                                                              }
    void                        setDisFracMMVD( bool val )                                { m_disFracMMVD = val;                                                                         }
    bool                        getDisFracMMVD() const                                    { return m_disFracMMVD;                                                                        }  
    void                        setDisBdofFlag( bool val )                                { m_disBdofFlag = val;                                                                         }
    bool                        getDisBdofFlag() const                                    { return m_disBdofFlag;                                                                        }
    void                        setDisDmvrFlag( bool val )                                { m_disDmvrFlag = val;                                                                         }
    bool                        getDisDmvrFlag() const                                    { return m_disDmvrFlag;                                                                        }
    void                        setDisProfFlag( bool val )                                { m_disProfFlag = val;                                                                         }
    bool                        getDisProfFlag() const                                    { return m_disProfFlag;                                                                        }
    void                        setMaxNumGeoCand(uint32_t b)                              { m_maxNumGeoCand = b; }
    uint32_t                    getMaxNumGeoCand() const                                  { return m_maxNumGeoCand; }
    // void                        setMaxNumIBCMergeCand( uint32_t b )                       { m_maxNumIBCMergeCand = b;                                                                    }
    // uint32_t                    getMaxNumIBCMergeCand() const                             { return m_maxNumIBCMergeCand;                                                                 } 
    void                        setJointCbCrSignFlag( bool b )                            { m_jointCbCrSignFlag = b;                                                                     }
    bool                        getJointCbCrSignFlag() const                              { return m_jointCbCrSignFlag;                                                                  }
    void                        setQpDelta(int b)                                         { m_qpDelta = b;                                                                               }
    int                         getQpDelta() const                                        { return m_qpDelta;                                                                            }
    void                        setSaoEnabledFlag(ChannelType chType, bool b)             { m_saoEnabledFlag[chType] = b;                                                                }
    bool                        getSaoEnabledFlag(ChannelType chType) const               { return m_saoEnabledFlag[chType];                                                             }  
    void                        setAlfEnabledFlag(ComponentID compId, bool b)             { m_alfEnabledFlag[compId] = b;                                                                }
    bool                        getAlfEnabledFlag(ComponentID compId) const               { return m_alfEnabledFlag[compId];                                                             }
    void                        setNumAlfAps(int i)                                       { m_numAlfAps = i;                                                                             }
    int                         getNumAlfAps() const                                      { return m_numAlfAps;                                                                          }
    void                        setAlfApsIdChroma(int i)                                  { m_alfChromaApsId = i;                                                                        }
    int                         getAlfApsIdChroma() const                                 { return m_alfChromaApsId;                                                                     }  
    void                        setCcAlfEnabledFlag(ComponentID compId, bool b)           { m_ccalfEnabledFlag[compId] = b; }
    bool                        getCcAlfEnabledFlag(ComponentID compId) const             { return m_ccalfEnabledFlag[compId]; }

    void                        setCcAlfCbApsId(int i)                                    { m_ccalfCbApsId = i; }
    int                         getCcAlfCbApsId() const                                   { return m_ccalfCbApsId; }
    void                        setCcAlfCrApsId(int i)                                    { m_ccalfCrApsId = i; }
    int                         getCcAlfCrApsId() const                                   { return m_ccalfCrApsId; }
    void                        setDeblockingFilterOverrideFlag( bool b )                 { m_deblockingFilterOverrideFlag = b;                                                          }
    bool                        getDeblockingFilterOverrideFlag() const                   { return m_deblockingFilterOverrideFlag;                                                       }    
    void                        setDeblockingFilterDisable( bool b )                      { m_deblockingFilterDisable= b;                                                                }  
    bool                        getDeblockingFilterDisable() const                        { return m_deblockingFilterDisable;                                                            }
    void                        setDeblockingFilterBetaOffsetDiv2( int i )                { m_deblockingFilterBetaOffsetDiv2 = i;                                                        }  
    int                         getDeblockingFilterBetaOffsetDiv2()const                  { return m_deblockingFilterBetaOffsetDiv2;                                                     }
    void                        setDeblockingFilterTcOffsetDiv2( int i )                  { m_deblockingFilterTcOffsetDiv2 = i;                                                          }  
    int                         getDeblockingFilterTcOffsetDiv2() const                   { return m_deblockingFilterTcOffsetDiv2;                                                       }    
    void                        setDeblockingFilterCbBetaOffsetDiv2( int i )              { m_deblockingFilterCbBetaOffsetDiv2 = i;                                                      }
    int                         getDeblockingFilterCbBetaOffsetDiv2()const                { return m_deblockingFilterCbBetaOffsetDiv2;                                                   }
    void                        setDeblockingFilterCbTcOffsetDiv2( int i )                { m_deblockingFilterCbTcOffsetDiv2 = i;                                                        }
    int                         getDeblockingFilterCbTcOffsetDiv2() const                 { return m_deblockingFilterCbTcOffsetDiv2;                                                     }
    void                        setDeblockingFilterCrBetaOffsetDiv2( int i )              { m_deblockingFilterCrBetaOffsetDiv2 = i;                                                      }
    int                         getDeblockingFilterCrBetaOffsetDiv2()const                { return m_deblockingFilterCrBetaOffsetDiv2;                                                   }
    void                        setDeblockingFilterCrTcOffsetDiv2( int i )                { m_deblockingFilterCrTcOffsetDiv2 = i;                                                        }
    int                         getDeblockingFilterCrTcOffsetDiv2() const                 { return m_deblockingFilterCrTcOffsetDiv2;                                                     }
    void                        setLmcsEnabledFlag(bool b)                                { m_lmcsEnabledFlag = b;                                                                       }
    bool                        getLmcsEnabledFlag()                                      { return m_lmcsEnabledFlag;                                                                    }
    const bool                  getLmcsEnabledFlag() const                                { return m_lmcsEnabledFlag;                                                                    }
    // void                        setLmcsAPS(std::shared_ptr<const APS> aps)                { m_lmcsAps = aps; m_lmcsApsId = (aps) ? aps->getAPSId() : -1;                                 }
    // std::shared_ptr<const APS>  getLmcsAPS() const                                        { return m_lmcsAps;                                                                            }
    void                        setLmcsAPSId(int id)                                      { m_lmcsApsId = id;                                                                            }
    int                         getLmcsAPSId() const                                      { return m_lmcsApsId;                                                                          }
    void                        setLmcsChromaResidualScaleFlag(bool b)                    { m_lmcsChromaResidualScaleFlag = b;                                                           }
    bool                        getLmcsChromaResidualScaleFlag()                          { return m_lmcsChromaResidualScaleFlag;                                                        }
    const bool                  getLmcsChromaResidualScaleFlag() const                    { return m_lmcsChromaResidualScaleFlag;                                                        }
    // void                        setScalingListAPS(std::shared_ptr<const APS> aps)         { m_scalingListAps = aps; m_scalingListApsId = ( aps ) ? aps->getAPSId() : -1;                 }
    // std::shared_ptr<const APS>  getScalingListAPS() const                                 { return m_scalingListAps;                                                                     }
    void                        setScalingListAPSId( int id )                             { m_scalingListApsId = id;                                                                     }
    int                         getScalingListAPSId() const                               { return m_scalingListApsId;                                                                   }
    void                        setExplicitScalingListEnabledFlag( bool b )               { m_explicitScalingListEnabledFlag = b;                                                        }
    bool                        getExplicitScalingListEnabledFlag()                       { return m_explicitScalingListEnabledFlag;                                                     }
    bool                        getExplicitScalingListEnabledFlag() const                 { return m_explicitScalingListEnabledFlag;                                                     }

    const PartitionConstraints& getMinQTSizes() const                                     { return m_minQT;                                                                              }
    const PartitionConstraints& getMaxBTSizes() const                                     { return m_maxBTSize;                                                                          }
    const PartitionConstraints& getMaxTTSizes() const                                     { return m_maxTTSize;                                                                          }

    void                        setMinQTSize           ( unsigned idx, unsigned minQT  )  { m_minQT[idx] = minQT;                                                                        }
    void                        setMaxMTTHierarchyDepth( unsigned idx, unsigned maxMTT )  { m_maxMTTHierarchyDepth[idx] = maxMTT;                                                        }
    void                        setMaxBTSize           ( unsigned idx, unsigned maxBT  )  { m_maxBTSize[idx] = maxBT;                                                                    }
    void                        setMaxTTSize           ( unsigned idx, unsigned maxTT  )  { m_maxTTSize[idx] = maxTT;                                                                    }

    void                        setMinQTSizes           ( const PartitionConstraints& minQT )  { m_minQT = minQT;                                                                        }
    void                        setMaxMTTHierarchyDepths( const PartitionConstraints& maxMTT ) { m_maxMTTHierarchyDepth = maxMTT;                                                        }
    void                        setMaxBTSizes           ( const PartitionConstraints& maxBT )  { m_maxBTSize = maxBT;                                                                    }
    void                        setMaxTTSizes           ( const PartitionConstraints& maxTT )  { m_maxTTSize = maxTT;                                                                    }

    unsigned                    getMinQTSize           ( SliceType slicetype, ChannelType chType = CHANNEL_TYPE_LUMA ) const { return m_minQT               [chType == CHANNEL_TYPE_LUMA ? slicetype == I_SLICE ?  0 : 1 : 2]; }
    unsigned                    getMaxMTTHierarchyDepth( SliceType slicetype, ChannelType chType = CHANNEL_TYPE_LUMA ) const { return m_maxMTTHierarchyDepth[chType == CHANNEL_TYPE_LUMA ? slicetype == I_SLICE ?  0 : 1 : 2]; }
    unsigned                    getMaxBTSize           ( SliceType slicetype, ChannelType chType = CHANNEL_TYPE_LUMA ) const { return m_maxBTSize           [chType == CHANNEL_TYPE_LUMA ? slicetype == I_SLICE ?  0 : 1 : 2]; }
    unsigned                    getMaxTTSize           ( SliceType slicetype, ChannelType chType = CHANNEL_TYPE_LUMA ) const { return m_maxTTSize           [chType == CHANNEL_TYPE_LUMA ? slicetype == I_SLICE ?  0 : 1 : 2]; }


    // void                        setAlfAPSIds( const AlfApsIdVec& apsIDs )                { m_alfApsId = std::move( apsIDs ); }
    // const AlfApsIdVec&          getAlfAPSIds() const                                     { return m_alfApsId;                }

    // void                        setWpScaling(WPScalingParam *wp) {
    //     memcpy(m_weightPredTable, wp, sizeof(WPScalingParam) * NUM_REF_PIC_LIST_01 * MAX_NUM_REF * MAX_NUM_COMPONENT);
    // }
    // void                        getWpScaling(RefPicList e, int iRefIdx, WPScalingParam *&wp) const;
    // WPScalingParam*             getWpScalingAll()                                        { return (WPScalingParam *) m_weightPredTable; }
    void                        setNumL0Weights(int b)                                   { m_numL0Weights = b;                          }
    int                         getNumL0Weights()                                        { return m_numL0Weights;                       }
    void                        setNumL1Weights(int b)                                   { m_numL1Weights = b;                          }
    int                         getNumL1Weights()                                        { return m_numL1Weights;                       }

    void                        setNoOutputBeforeRecoveryFlag( bool val )                { m_noOutputBeforeRecoveryFlag = val;  }
    bool                        getNoOutputBeforeRecoveryFlag() const                    { return m_noOutputBeforeRecoveryFlag; }
    void                        setHandleCraAsCvsStartFlag( bool val )                   { m_handleCraAsCvsStartFlag = val;     }
    bool                        getHandleCraAsCvsStartFlag() const                       { return m_handleCraAsCvsStartFlag;    }
    void                        setHandleGdrAsCvsStartFlag( bool val )                   { m_handleGdrAsCvsStartFlag = val;     }
    bool                        getHandleGdrAsCvsStartFlag() const                       { return m_handleGdrAsCvsStartFlag;    }
};

class Slice {
private:
  //  Bitstream writing
  bool                       m_saoEnabledFlag[MAX_NUM_CHANNEL_TYPE] = { false, false };
  int                        m_iPOC                             = 0;
  int                        m_iLastIDR                         = 0;
  int                        m_iAssociatedIRAP                  = 0;
  NalUnitType                m_iAssociatedIRAPType              = NAL_UNIT_INVALID;
  ReferencePictureList       m_RPL[2];                                                               //< RPL for L0/L1 when present in slice header
  int                        m_RPLIdx[2]                        = { -1, -1 };                        //< index of used RPL in the SPS or -1 for local RPL in the slice header
  NalUnitType                m_eNalUnitType                     = NAL_UNIT_CODED_SLICE_IDR_W_RADL;   ///< Nal unit type for the slice
  bool                       m_pictureHeaderInSliceHeader       = false;
  uint32_t                   m_nuhLayerId                       = 0;   ///< Nal unit layer id
  SliceType                  m_eSliceType                       = I_SLICE;
  bool                       m_noOutputOfPriorPicsFlag          = false;           //!< no output of prior pictures flag
  int                        m_iSliceQp                         = 0;

  bool                       m_ChromaQpAdjEnabled               = false;
  bool                       m_deblockingFilterDisable          = false;
  bool                       m_deblockingFilterOverrideFlag     = false;   //< offsets for deblocking filter inherit from PPS
  int                        m_deblockingFilterBetaOffsetDiv2   = 0;       //< beta offset for deblocking filter
  int                        m_deblockingFilterTcOffsetDiv2     = 0;       //< tc offset for deblocking filter
  int                        m_deblockingFilterCbBetaOffsetDiv2 = 0;       //< beta offset for deblocking filter
  int                        m_deblockingFilterCbTcOffsetDiv2   = 0;       //< tc offset for deblocking filter
  int                        m_deblockingFilterCrBetaOffsetDiv2 = 0;       //< beta offset for deblocking filter
  int                        m_deblockingFilterCrTcOffsetDiv2   = 0;       //< tc offset for deblocking filter
  bool                       m_depQuantEnabledFlag              = false;   //!< dependent quantization enabled flag
  bool                       m_signDataHidingEnabledFlag        = false;   //!< sign data hiding enabled flag
  bool                       m_tsResidualCodingDisabledFlag     = false;
  bool                       m_lmcsEnabledFlag                  = false;
  bool                       m_explicitScalingListUsed          = false;
  int                        m_aiNumRefIdx[NUM_REF_PIC_LIST_01] = { 0, 0 }; //  for multiple reference of current slice

  bool                       m_bCheckLDC                        = false;

  bool                       m_biDirPred                        = false;
  int                        m_symRefIdx[2]                     = { -1, -1 };

  //  Data
  int                        m_iSliceChromaQpDelta[MAX_NUM_COMPONENT + 1]                = { 0 };
  Picture*                   m_apcRefPicList      [NUM_REF_PIC_LIST_01][MAX_NUM_REF + 1] = { { nullptr } };   // entry 0 in m_apcRefPicList is nullptr!
  int                        m_aiRefPOCList       [NUM_REF_PIC_LIST_01][MAX_NUM_REF + 1] = { { 0 } };         // this is needed to get the POC of the a reference picture, when this slice is used as a collocated reference and the pictures referenced by this one have already been reused. (needed for TMVP)
  bool                       m_bIsUsedAsLongTerm  [NUM_REF_PIC_LIST_01][MAX_NUM_REF + 1] = { { false } };
  std::pair<int, int>        m_scalingRatio       [NUM_REF_PIC_LIST_01][MAX_NUM_REF_PICS];

  // access channel
  const SPS*                 m_pcSPS                         = nullptr;
  const PPS*                 m_pcPPS                         = nullptr;
  Picture*                   m_pcPic                         = nullptr;
  PicHeader*                 m_pcPicHeader                   = nullptr;    //!< pointer to picture header structure
  bool                       m_colFromL0Flag                 = true;   // collocated picture from List0 flag

  uint32_t                   m_colRefIdx                     = 0;
  uint32_t                   m_maxNumIBCMergeCand            = 0;
  bool                       m_disBdofDmvrFlag               = false;

  uint32_t                   m_uiTLayer                      = false;
  bool                       m_bTLayerSwitchingFlag          = false;

  uint32_t                   m_independentSliceIdx           = 0;

  ClpRng                     m_clpRng;
  std::vector<uint32_t>      m_substreamSizes;
  uint32_t                   m_numEntryPoints                = 0;
  bool                       m_cabacInitFlag                 = false;

  uint32_t                   m_sliceSubPicId                 = 0;

public:

};