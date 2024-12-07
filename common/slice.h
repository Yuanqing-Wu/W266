#pragma once

#include <memory>
#include <vector>
#include <array>

#include "def.h"
#include "picture.h"

using PartitionConstraints = std::array<unsigned, 3>;

template<class T>
struct BasePS: public std::enable_shared_from_this<T> {
    std::shared_ptr<T> get_shared_ptr() {
        return (static_cast<T*>(this))->shared_from_this();
    }
    std::shared_ptr<const T> get_shared_ptr() const {
        return (static_cast<const T*>(this))->shared_from_this();
    }

    void clear_changed_flag() {
        m_changed_flag = false;
    }

    bool get_changed_flag() const {
        return m_changed_flag;
    }

    bool             m_changed_flag = false;
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

    void printRefPicInfo() const;
    bool getInterLayerPresentFlag() const { return m_interLayerPresentFlag; }
    void setInterLayerPresentFlag( bool b ) { m_interLayerPresentFlag = b; }
    bool isInterLayerRefPic( int idx ) const { return m_isInterLayerRefPic[idx]; }
    int  getInterLayerRefPicIdx( int idx ) const { return m_interLayerRefPicIdx[idx]; }
    void setInterLayerRefPicIdx( int idx, int layerIdc );

    bool findInRefPicList( const Picture* checkRefPic, int currPicPoc, int layerId ) const;

    static int calcLTRefPOC( int currPoc, int bitsForPoc, int refPicIdentifier, bool pocMSBPresent, int deltaPocMSBCycle );
    int        calcLTRefPOC( int currPoc, int bitsForPoc, int refPicIdx ) const;
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
    BitDepths         m_bitDepths;
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
    GeneralHrdParams  m_generalHrdParams;
    std::vector<OlsHrdParams> m_olsHrdParams;

    bool              m_fieldSeqFlag                       = false;
    bool              m_vuiParametersPresentFlag           = false;
    unsigned          m_vuiPayloadSize                     = 0;
    VUI               m_vuiParameters;

    //SPSRExt           m_spsRangeExtension;
    static const      int m_winUnitX[NUM_CHROMA_FORMAT];
    static const      int m_winUnitY[NUM_CHROMA_FORMAT];
    ProfileTierLevel  m_profileTierLevel;

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
    ChromaQpMappingTable m_chromaQpMappingTable;
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

    SPS()
    {
        for ( int i = 0; i < MAX_TLAYER; i++ )
        {
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
    int                     getBitDepth() const                                                             { return m_bitDepths.recon;                                            }
    void                    setBitDepth(int u )                                                             { m_bitDepths.recon = u;                                               }
    const BitDepths&        getBitDepths() const                                                            { return m_bitDepths;                                                  }

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
    std::vector<OlsHrdParams>&       getOlsHrdParameters()             { return m_olsHrdParams; }
    const std::vector<OlsHrdParams>& getOlsHrdParameters() const       { return m_olsHrdParams; }

    GeneralHrdParams*       getGeneralHrdParameters() { return &m_generalHrdParams; }
    const GeneralHrdParams* getGeneralHrdParameters() const { return &m_generalHrdParams; }
    bool                    getFieldSeqFlag() const                                                         { return m_fieldSeqFlag;                         }
    void                    setFieldSeqFlag(bool i)                                                         { m_fieldSeqFlag = i;                            }
    bool                    getVuiParametersPresentFlag() const                                             { return m_vuiParametersPresentFlag;                                   }
    void                    setVuiParametersPresentFlag(bool b)                                             { m_vuiParametersPresentFlag = b;                                      }
    unsigned                getVuiPayloadSize() const                                                       { return m_vuiPayloadSize; }
    void                    setVuiPayloadSize(unsigned i)                                                   { m_vuiPayloadSize = i; }
    VUI*                    getVuiParameters()                                                              { return &m_vuiParameters;                                             }
    const VUI*              getVuiParameters() const                                                        { return &m_vuiParameters;                                             }
    const ProfileTierLevel* getProfileTierLevel() const                                                     { return &m_profileTierLevel; }
    ProfileTierLevel*       getProfileTierLevel()                                                           { return &m_profileTierLevel; }

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
    void      setChromaQpMappingTableFromParams( const ChromaQpMappingTableParams& params )  { m_chromaQpMappingTable = ChromaQpMappingTable( params ); }
    void      setChromaQpMappingTableFromParams( const ChromaQpMappingTableParams&& params ) { m_chromaQpMappingTable = ChromaQpMappingTable( std::move( params ) ); }
    void      deriveChromaQPMappingTables()                                           { m_chromaQpMappingTable.deriveChromaQPMappingTables(); }
    const ChromaQpMappingTable& getChromaQpMappingTable()                   const     { return m_chromaQpMappingTable;}
    int       getMappedChromaQpValue(ComponentID compID, int qpVal)         const     { return m_chromaQpMappingTable.getMappedChromaQpValue(compID, qpVal); }
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