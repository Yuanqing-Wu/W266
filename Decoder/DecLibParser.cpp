#include <memory>
#include <cmath>

#include "DecLibParser.h"
#include "Common/Slice.h"
#include "Common/Def.h"
#include "Common/Common.h"

bool DecLibParser::parse( InputNALUnit& nalu ) {
    switch( nalu.m_nalUnitType ) {
    case NAL_UNIT_CODED_SLICE_TRAIL:
    case NAL_UNIT_CODED_SLICE_STSA:
    case NAL_UNIT_CODED_SLICE_RADL:
    case NAL_UNIT_CODED_SLICE_RASL:
    case NAL_UNIT_CODED_SLICE_IDR_W_RADL:
    case NAL_UNIT_CODED_SLICE_IDR_N_LP:
    case NAL_UNIT_CODED_SLICE_CRA:
    case NAL_UNIT_CODED_SLICE_GDR:
    case NAL_UNIT_OPI:
        // NOT IMPLEMENTED
        return false;

    case NAL_UNIT_DCI:
        return false;

    case NAL_UNIT_VPS:
        return false;

    case NAL_UNIT_SPS:
        xDecodeSPS( nalu );
        return false;

    case NAL_UNIT_PPS:
        //xDecodePPS( nalu );
        return false;

    case NAL_UNIT_PREFIX_APS:
    case NAL_UNIT_SUFFIX_APS:
        //xDecodeAPS( nalu );
        return false;

    case NAL_UNIT_PH:
        //xDecodePicHeader( nalu );
        return false;

    case NAL_UNIT_ACCESS_UNIT_DELIMITER:
        return false;

    case NAL_UNIT_EOS:
        return false;

    case NAL_UNIT_EOB:
        return false;

    case NAL_UNIT_PREFIX_SEI:
        return false;

    case NAL_UNIT_SUFFIX_SEI:
        return false;

    case NAL_UNIT_FD:
        return false;

    case NAL_UNIT_RESERVED_VCL_4:
    case NAL_UNIT_RESERVED_VCL_5:
    case NAL_UNIT_RESERVED_VCL_6:
    case NAL_UNIT_RESERVED_IRAP_VCL_11:
        return false;
    case NAL_UNIT_RESERVED_NVCL_26:
    case NAL_UNIT_RESERVED_NVCL_27:
        return false;
    case NAL_UNIT_UNSPECIFIED_28:
    case NAL_UNIT_UNSPECIFIED_29:
    case NAL_UNIT_UNSPECIFIED_30:
    case NAL_UNIT_UNSPECIFIED_31:
        return false;
    case NAL_UNIT_INVALID:
    default:
        THROW_RECOVERABLE( "Invalid NAL unit type" );
        break;
    }

    return false;
}

void DecLibParser::xDecodeSPS( InputNALUnit& nalu ) {
    std::unique_ptr<SPS> sps( new SPS() );
    m_HLSReader.setBitstream( &nalu.getBitstream() );
    m_HLSReader.parseSPS( sps.get() );
}

void HLSyntaxReader::parseSPS( SPS* sps ) {
    X_READ_CODE_NO_RANGE( sps_seq_parameter_set_id, 4 );
    sps->setSPSId( sps_seq_parameter_set_id );

    X_READ_CODE_NO_RANGE( sps_video_parameter_set_id, 4 );
    (void) sps_video_parameter_set_id;   // sps->setVPSId( sps_video_parameter_set_id ); // TODO: change to support VPS
    const VPS* vps = nullptr;

    X_READ_CODE( sps_max_sublayers_minus1, 3, 0, 6 );
    sps->setMaxTLayers( sps_max_sublayers_minus1 + 1 );

    X_READ_CODE_NO_RANGE( sps_chroma_format_idc, 2 );
    // it is a requirement of bitstream conformance that the value of sps_chroma_format_idc shall
    // be less than or equal to the value of vps_ols_dpb_chroma_format[ i ].
    sps->setChromaFormatIdc( ChromaFormat( sps_chroma_format_idc ) );

    X_READ_CODE( sps_log2_ctu_size_minus5, 2, 0, 2 );
    sps->setCTUSize( 1 << ( sps_log2_ctu_size_minus5 + 5 ) );

    const int CtbLog2SizeY = sps_log2_ctu_size_minus5 + 5;
    const int CtbSizeY     = 1 << CtbLog2SizeY;
    sps->setMaxCUWidth( sps->getCTUSize() );
    sps->setMaxCUHeight( sps->getCTUSize() );

    X_READ_FLAG( sps_ptl_dpb_hrd_params_present_flag );
    CHECK( sps_video_parameter_set_id == 0 && !sps_ptl_dpb_hrd_params_present_flag,
                        "When sps_video_parameter_set_id is equal to 0, the value of sps_ptl_dpb_hrd_params_present_flag shall be equal to 1" );
    sps->setPtlDpbHrdParamsPresentFlag( sps_ptl_dpb_hrd_params_present_flag );

    if( sps_ptl_dpb_hrd_params_present_flag ) {
        // parseProfileTierLevel( sps->getProfileTierLevel(), true, sps->getMaxTLayers() - 1 );
    }

    X_READ_FLAG( sps_gdr_enabled_flag );
    sps->setGDREnabledFlag( sps_gdr_enabled_flag );

    X_READ_FLAG( sps_ref_pic_resampling_enabled_flag );
    sps->setRprEnabledFlag( sps_ref_pic_resampling_enabled_flag );

    if( sps_ref_pic_resampling_enabled_flag )
    {
        X_READ_FLAG( sps_res_change_in_clvs_allowed_flag );
        sps->setResChangeInClvsEnabledFlag( sps_res_change_in_clvs_allowed_flag );
    }

    X_READ_UVLC_NO_RANGE( sps_pic_width_max_in_luma_samples );
    sps->setMaxPicWidthInLumaSamples( sps_pic_width_max_in_luma_samples );

    X_READ_UVLC_NO_RANGE( sps_pic_height_max_in_luma_samples );
    sps->setMaxPicHeightInLumaSamples( sps_pic_height_max_in_luma_samples );

    const int SubWidthC  = 1 << getChannelTypeScaleX( CHANNEL_TYPE_CHROMA, sps->getChromaFormatIdc() );
    const int SubHeightC = 1 << getChannelTypeScaleY( CHANNEL_TYPE_CHROMA, sps->getChromaFormatIdc() );

    X_READ_FLAG( sps_conformance_window_flag );
    sps->setConformanceWindowPresentFlag( sps_conformance_window_flag );

    if( sps_conformance_window_flag )
    {
        X_READ_UVLC_NO_RANGE( sps_conf_win_left_offset );
        X_READ_UVLC_NO_RANGE( sps_conf_win_right_offset );
        X_READ_UVLC_NO_RANGE( sps_conf_win_top_offset );
        X_READ_UVLC_NO_RANGE( sps_conf_win_bottom_offset );

        CHECK( SubWidthC * ( sps_conf_win_left_offset + sps_conf_win_right_offset ) > sps_pic_width_max_in_luma_samples,
            "The value of SubWidthC * ( sps_conf_win_left_offset + sps_conf_win_right_offset ) shall be less than sps_pic_width_max_in_luma_samples." );
        CHECK( SubHeightC * ( sps_conf_win_top_offset + sps_conf_win_bottom_offset ) > sps_pic_height_max_in_luma_samples,
            "The value of SubHeightC * ( sps_conf_win_top_offset + sps_conf_win_bottom_offset ) shall be less than sps_pic_height_max_in_luma_samples." );

        Window& conf = sps->getConformanceWindow();
        conf.setWindowLeftOffset( sps_conf_win_left_offset );
        conf.setWindowRightOffset( sps_conf_win_right_offset );
        conf.setWindowTopOffset( sps_conf_win_top_offset );
        conf.setWindowBottomOffset( sps_conf_win_bottom_offset );
    }
    X_READ_FLAG( sps_subpic_info_present_flag );
    CHECK( sps->getResChangeInClvsEnabledFlag() && sps_subpic_info_present_flag,
            "When sps_res_change_in_clvs_allowed_flag is equal to 1, the value of sps_subpic_info_present_flag shall be equal to 0." )
    sps->setSubPicInfoPresentFlag( sps_subpic_info_present_flag );

    {
        X_READ_UVLC_NO_RANGE( sps_num_subpics_minus1 );
        CHECK( sps_num_subpics_minus1 + 1 > ( ( sps_pic_width_max_in_luma_samples + CtbSizeY - 1 ) / CtbSizeY )
                                                        * ( ( sps_pic_height_max_in_luma_samples + CtbSizeY - 1 ) / CtbSizeY ),
                        "Invalid sps_num_subpics_minus1 value" );
        sps->setNumSubPics( sps_num_subpics_minus1 + 1 );

        if( sps_num_subpics_minus1 == 0 )
        {
        sps->setSubPicCtuTopLeftX( 0, 0 );
        sps->setSubPicCtuTopLeftY( 0, 0 );
        sps->setSubPicWidth( 0, ( sps->getMaxPicWidthInLumaSamples() + sps->getCTUSize() - 1 ) >> getLog2( sps->getCTUSize() ) );
        sps->setSubPicHeight( 0, ( sps->getMaxPicHeightInLumaSamples() + sps->getCTUSize() - 1 ) >> getLog2( sps->getCTUSize() ) );
        sps->setIndependentSubPicsFlag( 1 );
        sps->setSubPicSameSizeFlag( 0 );
        sps->setSubPicTreatedAsPicFlag( 0, 1 );
        sps->setLoopFilterAcrossSubpicEnabledFlag( 0, 0 );
        }
        else   // ( sps_num_subpics_minus1 > 0 )
        {
        X_READ_FLAG( sps_independent_subpics_flag );
        sps->setIndependentSubPicsFlag( sps_independent_subpics_flag );

        X_READ_FLAG( sps_subpic_same_size_flag );
        sps->setSubPicSameSizeFlag( sps_subpic_same_size_flag );

        const uint32_t tmpWidthVal = ( sps->getMaxPicWidthInLumaSamples() + sps->getCTUSize() - 1 ) / sps->getCTUSize();
        const uint32_t tmpHeightVal = ( sps->getMaxPicHeightInLumaSamples() + sps->getCTUSize() - 1 ) / sps->getCTUSize();

        const int ceilLog2tmpWidth  = (int) ceil( log2( tmpWidthVal ) );
        const int ceilLog2tmpHeight = (int) ceil( log2( tmpHeightVal ) );
        for( unsigned picIdx = 0; picIdx < sps->getNumSubPics(); picIdx++ )
        {
            if( !sps_subpic_same_size_flag || picIdx == 0 )
            {
            if( picIdx > 0 && sps_pic_width_max_in_luma_samples > CtbSizeY )
            {
                X_READ_CODE_NO_RANGE_idx( sps_subpic_ctu_top_left_x, "[ i ]", ceilLog2tmpWidth );
                sps->setSubPicCtuTopLeftX( picIdx, sps_subpic_ctu_top_left_x );
            }
            else
            {
                sps->setSubPicCtuTopLeftX( picIdx, 0 );
            }

            if( picIdx > 0 && sps_pic_height_max_in_luma_samples > CtbSizeY )
            {
                X_READ_CODE_NO_RANGE_idx( sps_subpic_ctu_top_left_y, "[ i ]", ceilLog2tmpHeight );
                sps->setSubPicCtuTopLeftY( picIdx, sps_subpic_ctu_top_left_y );
            }
            else
            {
                sps->setSubPicCtuTopLeftY( picIdx, 0 );
            }

            if( picIdx < sps_num_subpics_minus1 && sps_pic_width_max_in_luma_samples > CtbSizeY )
            {
                X_READ_CODE_NO_RANGE_idx( sps_subpic_width_minus1, "[ i ]", ceilLog2tmpWidth );
                sps->setSubPicWidth( picIdx, sps_subpic_width_minus1 + 1 );
            }
            else
            {
                // If sps_subpic_same_size_flag is equal to 0 or i is equal to 0, the value of sps_subpic_width_minus1[ i ] is inferred
                //   to be equal to tmpWidthVal - sps_subpic_ctu_top_left_x[ i ] - 1.
                sps->setSubPicWidth( picIdx, tmpWidthVal - sps->getSubPicCtuTopLeftX( picIdx ) );
            }

            if( picIdx < sps_num_subpics_minus1 && sps_pic_height_max_in_luma_samples > CtbSizeY )
            {
                X_READ_CODE_NO_RANGE_idx( sps_subpic_height_minus1, "[ i ]", ceilLog2tmpHeight );
                sps->setSubPicHeight( picIdx, sps_subpic_height_minus1 + 1 );
            }
            else
            {
                // If sps_subpic_same_size_flag is equal to 0 or i is equal to 0, the value of sps_subpic_height_minus1[ i ] is inferred
                //   to be equal to tmpHeightVal - sps_subpic_ctu_top_left_y[ i ] − 1.
                sps->setSubPicHeight( picIdx, tmpHeightVal - sps->getSubPicCtuTopLeftY( picIdx ) );
            }
            }
            else   // ( sps_subpic_same_size_flag && picIdx != 0 )
            {
            const int numSubpicCols = tmpWidthVal / sps->getSubPicWidth( 0 );

            // const int numSubpicCols = tmpWidthVal / ( sps->getSubPicWidth( 0 ) + 1 );
            CHECK( sps_subpic_same_size_flag && numSubpicCols * tmpHeightVal / sps->getSubPicHeight( 0 ) - 1 != sps_num_subpics_minus1,
                    "When sps_subpic_same_size_flag is equal to 1, the value of numSubpicCols * tmpHeightVal / ( sps_subpic_height_minus1[ 0 ] + 1 ) − 1"
                    " shall be equal to sps_num_subpics_minus1." )
            CHECK( sps_subpic_same_size_flag && tmpWidthVal % sps->getSubPicWidth( 0 ) != 0,
                    "When sps_subpic_same_size_flag is equal to 1, the value of tmpWidthVal % ( sps_subpic_width_minus1[ 0 ] + 1 ) shall"
                    " be equal to 0." );
            CHECK( sps_subpic_same_size_flag && tmpHeightVal % sps->getSubPicHeight( 0 ) != 0,
                    "When sps_subpic_same_size_flag is equal to 1, the value of tmpHeightVal % ( sps_subpic_height_minus1[ 0 ] + 1 ) shall"
                    " be equal to 0." )

            // Otherwise, the value of sps_subpic_ctu_top_left_x[ i ] is inferred to be equal to ( i % numSubpicCols ) * ( sps_subpic_width_minus1[ 0 ] + 1 ).
            // Otherwise, the value of sps_subpic_ctu_top_left_y[ i ] is inferred to be equal to ( i / numSubpicCols ) * ( sps_subpic_height_minus1[ 0 ] + 1 ).
            sps->setSubPicCtuTopLeftX( picIdx, ( picIdx % numSubpicCols ) * sps->getSubPicWidth( 0 ) );
            sps->setSubPicCtuTopLeftY( picIdx, ( picIdx / numSubpicCols ) * sps->getSubPicHeight( 0 ) );
            // Otherwise, the value of sps_subpic_width_minus1[ i ] is inferred to be equal to sps_subpic_width_minus1[ 0 ].
            // Otherwise, the value of sps_subpic_height_minus1[ i ] is inferred to be equal to sps_subpic_height_minus1[ 0 ].
            sps->setSubPicWidth( picIdx, sps->getSubPicWidth( 0 ) );
            sps->setSubPicHeight( picIdx, sps->getSubPicHeight( 0 ) );
            }

            // TODO: It is a requirement of bitstream conformance that the shapes of the subpictures shall be such that each subpicture, when
            //       decoded, shall have its entire left boundary and entire top boundary consisting of picture boundaries or consisting of
            //       boundaries of previously decoded subpictures.
            //       For each subpicture with subpicture index i in the range of 0 to sps_num_subpics_minus1, inclusive, it is a requirement
            //       of bitstream conformance that all of the following conditions are true:
            //       - The value of ( sps_subpic_ctu_top_left_x[ i ] * CtbSizeY ) shall be less than ( sps_pic_width_max_in_luma_samples - sps_conf_win_right_offset * SubWidthC ).
            //       - The value of ( ( sps_subpic_ctu_top_left_x[ i ] + sps_subpic_width_minus1[ i ] + 1 ) * CtbSizeY ) shall be greater than ( sps_conf_win_left_offset * SubWidthC ).
            //       - The value of ( sps_subpic_ctu_top_left_y[ i ] * CtbSizeY ) shall  be less than ( sps_pic_height_max_in_luma_samples - sps_conf_win_bottom_offset * SubHeightC ).
            //       - The value of ( ( sps_subpic_ctu_top_left_y[ i ] + sps_subpic_height_minus1[ i ] + 1 ) * CtbSizeY ) shall be greater than ( sps_conf_win_top_offset * SubHeightC ).

            Window& conf = sps->getConformanceWindow();
            CHECK( sps->getSubPicCtuTopLeftX( picIdx ) * CtbSizeY >= sps->getMaxPicWidthInLumaSamples() - conf.getWindowRightOffset() * SubWidthC,
                "The value of ( sps_subpic_ctu_top_left_x[ i ] * CtbSizeY )"
                " shall be less than ( sps_pic_width_max_in_luma_samples - sps_conf_win_right_offset * SubWidthC )." );
            CHECK( ( sps->getSubPicCtuTopLeftX( picIdx ) + sps->getSubPicWidth( picIdx ) ) * CtbSizeY <= conf.getWindowLeftOffset() * SubWidthC,
                "The value of ( ( sps_subpic_ctu_top_left_x[ i ] + sps_subpic_width_minus1[ i ] + 1 ) * CtbSizeY )"
                " shall be greater than ( sps_conf_win_left_offset * SubWidthC )." );
            CHECK( sps->getSubPicCtuTopLeftY( picIdx ) * CtbSizeY >= sps->getMaxPicHeightInLumaSamples() - conf.getWindowBottomOffset() * SubHeightC,
                "The value of ( sps_subpic_ctu_top_left_y[ i ] * CtbSizeY )"
                " shall  be less than ( sps_pic_height_max_in_luma_samples - sps_conf_win_bottom_offset * SubHeightC )." );
            CHECK( ( sps->getSubPicCtuTopLeftX( picIdx ) + sps->getSubPicHeight( picIdx ) ) * CtbSizeY <= conf.getWindowTopOffset() * SubHeightC,
                "The value of ( ( sps_subpic_ctu_top_left_y[ i ] + sps_subpic_height_minus1[ i ] + 1 ) * CtbSizeY )"
                " shall be greater than ( sps_conf_win_top_offset * SubHeightC )." );

            if( !sps_independent_subpics_flag )
            {
            X_READ_FLAG_idx( sps_subpic_treated_as_pic_flag, "[ i ]" );
            sps->setSubPicTreatedAsPicFlag( picIdx, sps_subpic_treated_as_pic_flag );

            X_READ_FLAG_idx( sps_loop_filter_across_subpic_enabled_flag, "[ i ]" );
            sps->setLoopFilterAcrossSubpicEnabledFlag( picIdx, sps_loop_filter_across_subpic_enabled_flag );
            }
            else
            {
            // should be set as default
                sps->setSubPicTreatedAsPicFlag( picIdx, 1 );
            }
        }   //      for( unsigned picIdx = 0; picIdx < sps->getNumSubPics(); picIdx++ )
        }


        X_READ_UVLC( sps_subpic_id_len_minus1, 0, 15 );
        CHECK( 1 << ( sps_subpic_id_len_minus1 + 1 ) < sps_num_subpics_minus1 + 1,
            "The value of 1 << ( sps_subpic_id_len_minus1 + 1 ) shall be greater than or equal to sps_num_subpics_minus1 + 1" );
        sps->setSubPicIdLen( sps_subpic_id_len_minus1 + 1 );

        X_READ_FLAG( sps_subpic_id_mapping_explicitly_signalled_flag );
        sps->setSubPicIdMappingExplicitlySignalledFlag( sps_subpic_id_mapping_explicitly_signalled_flag );

        if( sps_subpic_id_mapping_explicitly_signalled_flag )
        {
        X_READ_FLAG( sps_subpic_id_mapping_present_flag );
        sps->setSubPicIdMappingPresentFlag( sps_subpic_id_mapping_present_flag );

        if( sps_subpic_id_mapping_present_flag )
        {
            for( int picIdx = 0; picIdx <= sps_num_subpics_minus1; picIdx++ )
            {
            X_READ_CODE_NO_RANGE_idx( sps_subpic_id, "[ i ]", sps->getSubPicIdLen() );
            sps->setSubPicId( picIdx, sps_subpic_id );
            }
        }
        }
    }   // sps_subpic_info_present_flag

    X_READ_UVLC( sps_bitdepth_minus8, 0, 8 );
    sps->setBitDepth( 8 + sps_bitdepth_minus8 );
    sps->setQpBDOffset( 6 * sps_bitdepth_minus8 );
    const int BitDepth = 8 + sps_bitdepth_minus8;
    const int QpBdOffset = 6 * sps_bitdepth_minus8;

    X_READ_FLAG( sps_entropy_coding_sync_enabled_flag );
    sps->setEntropyCodingSyncEnabledFlag( sps_entropy_coding_sync_enabled_flag );

    X_READ_FLAG( sps_entry_point_offsets_present_flag );
    sps->setEntryPointsPresentFlag( sps_entry_point_offsets_present_flag );

    X_READ_CODE( sps_log2_max_pic_order_cnt_lsb_minus4, 4, 0, 12 );
    sps->setBitsForPOC( sps_log2_max_pic_order_cnt_lsb_minus4 + 4 );

    X_READ_FLAG( sps_poc_msb_cycle_flag );
    sps->setPocMsbFlag( sps_poc_msb_cycle_flag );

    if( sps_poc_msb_cycle_flag )
    {
        X_READ_UVLC( sps_poc_msb_cycle_len_minus1, 0, 32 - sps_log2_max_pic_order_cnt_lsb_minus4 - 5 );
        sps->setPocMsbLen( sps_poc_msb_cycle_len_minus1 + 1 );
    }

    // extra bits are for future extensions, we will read, but ignore them,
    // unless a meaning is specified in the spec
    X_READ_CODE( sps_num_extra_ph_bytes, 2, 0, 2 );
    sps->setNumExtraPHBitsBytes( sps_num_extra_ph_bytes );

    X_READ_CODE( sps_num_extra_sh_bytes, 2, 0, 2 );
    sps->setNumExtraSHBitsBytes( sps_num_extra_sh_bytes );

    X_READ_UVLC( sps_log2_min_luma_coding_block_size_minus2, 0, std::min( 4u, sps_log2_ctu_size_minus5 + 3 ) );
    sps->setLog2MinCodingBlockSize( sps_log2_min_luma_coding_block_size_minus2 + 2 );

    const int MinCbLog2SizeY = sps_log2_min_luma_coding_block_size_minus2 + 2;
    const int MinCbSizeY     = 1 << MinCbLog2SizeY;
    const int VSize          = std::min( 64, CtbSizeY );
    CHECK( MinCbSizeY > VSize, "The value of MinCbSizeY shall be less than or equal to VSize." )
    CHECK( MinCbLog2SizeY > CtbLog2SizeY, "Invalid log2_min_luma_coding_block_size_minus2 signalled" );

    // postponed checks for sps_pic_{width,height}_max_in_luma_samples,
    CHECK( sps_pic_width_max_in_luma_samples == 0 || sps_pic_width_max_in_luma_samples & ( std::max( 8, MinCbSizeY ) - 1 ),
            "sps_pic_width_max_in_luma_samples shall not be equal to 0 and shall be an integer multiple of Max( 8, MinCbSizeY )" );
    CHECK( sps_pic_height_max_in_luma_samples == 0 || sps_pic_height_max_in_luma_samples & ( std::max( 8, MinCbSizeY ) - 1 ),
            "sps_pic_height_max_in_luma_samples shall not be equal to 0 and shall be an integer multiple of Max( 8, MinCbSizeY )" );

    const int minCuSize = 1 << sps->getLog2MinCodingBlockSize();
    CHECK( ( sps->getMaxPicWidthInLumaSamples() % ( std::max( 8, minCuSize ) ) ) != 0, "Coded frame width must be a multiple of Max(8, the minimum unit size)" );
    CHECK( ( sps->getMaxPicHeightInLumaSamples() % ( std::max( 8, minCuSize ) ) ) != 0, "Coded frame height must be a multiple of Max(8, the minimum unit size)" );

    X_READ_FLAG( sps_partition_constraints_override_enabled_flag );
    sps->setSplitConsOverrideEnabledFlag( sps_partition_constraints_override_enabled_flag );

    X_READ_UVLC( sps_log2_diff_min_qt_min_cb_intra_slice_luma, 0, std::min( 6, CtbLog2SizeY ) - MinCbLog2SizeY );
    const unsigned MinQtLog2SizeIntraY = sps_log2_diff_min_qt_min_cb_intra_slice_luma + MinCbLog2SizeY;

    X_READ_UVLC( sps_max_mtt_hierarchy_depth_intra_slice_luma, 0, 2 * ( CtbLog2SizeY - MinCbLog2SizeY ) );

    PartitionConstraints minQT     = { 1u << MinQtLog2SizeIntraY, 0, 0 };
    PartitionConstraints maxBTD    = { sps_max_mtt_hierarchy_depth_intra_slice_luma, 0, 0 };
    PartitionConstraints maxTTSize = { 1u << MinQtLog2SizeIntraY, 0, 0 };
    PartitionConstraints maxBTSize = { 1u << MinQtLog2SizeIntraY, 0, 0 };

    int spsLog2DiffMinQtMinCbIntraSliceLuma = 0;
    if( sps_max_mtt_hierarchy_depth_intra_slice_luma != 0 )
    {
        X_READ_UVLC( sps_log2_diff_max_bt_min_qt_intra_slice_luma, 0, CtbLog2SizeY - MinQtLog2SizeIntraY );
        maxBTSize[0] <<= sps_log2_diff_max_bt_min_qt_intra_slice_luma;
        spsLog2DiffMinQtMinCbIntraSliceLuma = sps_log2_diff_max_bt_min_qt_intra_slice_luma;

        X_READ_UVLC( sps_log2_diff_max_tt_min_qt_intra_slice_luma, 0, std::min( 6, CtbLog2SizeY ) - MinQtLog2SizeIntraY );
        maxTTSize[0] <<= sps_log2_diff_max_tt_min_qt_intra_slice_luma;
    }
    CHECK( maxTTSize[0] > 64, "The value of sps_log2_diff_max_tt_min_qt_intra_slice_luma shall be in the range of 0 to min(6,CtbLog2SizeY) - MinQtLog2SizeIntraY" );

    if( sps_chroma_format_idc != CHROMA_400 )
    {
        X_READ_FLAG( sps_qtbtt_dual_tree_intra_flag );
        sps->setUseDualITree( sps_qtbtt_dual_tree_intra_flag );

        (void)spsLog2DiffMinQtMinCbIntraSliceLuma;
        // this breaks the testset (TREE_C_HHI_3.bit) although specified in the spec
        // CHECK( spsLog2DiffMinQtMinCbIntraSliceLuma > std::min( 6, CtbLog2SizeY ) - MinQtLog2SizeIntraY && sps_qtbtt_dual_tree_intra_flag,
        //        "When sps_log2_diff_max_bt_min_qt_intra_slice_luma is greater than Min( 6, CtbLog2SizeY ) - MinQtLog2SizeIntraY, the value of "
        //        "sps_qtbtt_dual_tree_intra_flag shall be equal to 0." )
    }


    if( sps->getUseDualITree() )
    {
        X_READ_UVLC( sps_log2_diff_min_qt_min_cb_intra_slice_chroma, 0, std::min( 6, CtbLog2SizeY ) - MinCbLog2SizeY );
        const int MinQtLog2SizeIntraC = sps_log2_diff_min_qt_min_cb_intra_slice_chroma + MinCbLog2SizeY;

        X_READ_UVLC( sps_max_mtt_hierarchy_depth_intra_slice_chroma, 0, 2 * ( CtbLog2SizeY - MinCbLog2SizeY ) );
        maxBTD[2] = sps_max_mtt_hierarchy_depth_intra_slice_chroma;

        minQT[2] = 1 << MinQtLog2SizeIntraC;
        maxTTSize[2] = maxBTSize[2] = minQT[2];
        if( sps_max_mtt_hierarchy_depth_intra_slice_chroma != 0 )
        {
        X_READ_UVLC( sps_log2_diff_max_bt_min_qt_intra_slice_chroma, 0, std::min( 6, CtbLog2SizeY ) - MinQtLog2SizeIntraC );
        maxBTSize[2] <<= sps_log2_diff_max_bt_min_qt_intra_slice_chroma;

        X_READ_UVLC( sps_log2_diff_max_tt_min_qt_intra_slice_chroma, 0, std::min( 6, CtbLog2SizeY ) - MinQtLog2SizeIntraC );
        maxTTSize[2] <<= sps_log2_diff_max_tt_min_qt_intra_slice_chroma;

        CHECK( maxTTSize[2] > 64, "The value of sps_log2_diff_max_tt_min_qt_intra_slice_chroma shall be in the range of 0 to min(6,CtbLog2SizeY) - MinQtLog2SizeIntraChroma" );
        CHECK( maxBTSize[2] > 64, "The value of sps_log2_diff_max_bt_min_qt_intra_slice_chroma shall be in the range of 0 to min(6,CtbLog2SizeY) - MinQtLog2SizeIntraChroma" );
        }
    }
    // minQT[2] = 1 << MinQtLog2SizeIntraC; // THIS WAS MISSING? -> only read for dual tree

    X_READ_UVLC( sps_log2_diff_min_qt_min_cb_inter_slice, 0, std::min( 6, CtbLog2SizeY ) - MinCbLog2SizeY );
    const int MinQtLog2SizeInterY = sps_log2_diff_min_qt_min_cb_inter_slice + MinCbLog2SizeY;

    X_READ_UVLC( sps_max_mtt_hierarchy_depth_inter_slice, 0, 2 * ( CtbLog2SizeY - MinCbLog2SizeY ) );
    maxBTD[1] = sps_max_mtt_hierarchy_depth_inter_slice;

    minQT[1] = 1 << MinQtLog2SizeInterY;
    maxTTSize[1] = maxBTSize[1] = minQT[1];
    if( sps_max_mtt_hierarchy_depth_inter_slice != 0 )
    {
        X_READ_UVLC( sps_log2_diff_max_bt_min_qt_inter_slice, 0, CtbLog2SizeY - MinQtLog2SizeInterY );
        maxBTSize[1] <<= sps_log2_diff_max_bt_min_qt_inter_slice;

        X_READ_UVLC( sps_log2_diff_max_tt_min_qt_inter_slice, 0, std::min( 6, CtbLog2SizeY ) - MinQtLog2SizeInterY );
        maxTTSize[1] <<= sps_log2_diff_max_tt_min_qt_inter_slice;
    }

    sps->setMinQTSizes( minQT );
    sps->setMaxMTTHierarchyDepths( maxBTD );
    sps->setMaxBTSizes( maxBTSize );
    sps->setMaxTTSizes( maxTTSize );

    if( CtbSizeY > 32 )
    {
        X_READ_FLAG( sps_max_luma_transform_size_64_flag );
        sps->setLog2MaxTbSize( 5 + sps_max_luma_transform_size_64_flag );
    }
    else
    {
        sps->setLog2MaxTbSize( 5 );
    }

    X_READ_FLAG( sps_transform_skip_enabled_flag );
    sps->setTransformSkipEnabledFlag( sps_transform_skip_enabled_flag );

    if( sps_transform_skip_enabled_flag )
    {
        X_READ_UVLC( sps_log2_transform_skip_max_size_minus2, 0, 3 );
        sps->setLog2MaxTransformSkipBlockSize( sps_log2_transform_skip_max_size_minus2 + 2 );

        X_READ_FLAG( sps_bdpcm_enabled_flag );
        sps->setBDPCMEnabledFlag( sps_bdpcm_enabled_flag );
    }

    X_READ_FLAG( sps_mts_enabled_flag );
    sps->setUseMTS( sps_mts_enabled_flag );

    if( sps_mts_enabled_flag )
    {
        X_READ_FLAG( sps_explicit_mts_intra_enabled_flag );
        sps->setUseIntraMTS( sps_explicit_mts_intra_enabled_flag );

        X_READ_FLAG( sps_explicit_mts_inter_enabled_flag );
        sps->setUseInterMTS( sps_explicit_mts_inter_enabled_flag );
    }

    X_READ_FLAG( sps_lfnst_enabled_flag );
    sps->setUseLFNST( sps_lfnst_enabled_flag );

    if( sps_chroma_format_idc != CHROMA_400 )
    {
        X_READ_FLAG( sps_joint_cbcr_enabled_flag );
        sps->setJointCbCrEnabledFlag( sps_joint_cbcr_enabled_flag );

        X_READ_FLAG( sps_same_qp_table_for_chroma_flag );

    }


    X_READ_FLAG( sps_sao_enabled_flag );
    sps->setUseSAO( sps_sao_enabled_flag );

    X_READ_FLAG( sps_alf_enabled_flag );
    sps->setUseALF( sps_alf_enabled_flag );

    if( sps_alf_enabled_flag && sps_chroma_format_idc != CHROMA_400 )
    {
        X_READ_FLAG( sps_ccalf_enabled_flag );
        sps->setUseCCALF( sps_ccalf_enabled_flag );
    }
    else
    {
        sps->setUseCCALF( false );
    }

    X_READ_FLAG( sps_lmcs_enable_flag );
    sps->setUseReshaper( sps_lmcs_enable_flag );

    X_READ_FLAG( sps_weighted_pred_flag );
    sps->setUseWP( sps_weighted_pred_flag );

    X_READ_FLAG( sps_weighted_bipred_flag );
    sps->setUseWPBiPred( sps_weighted_bipred_flag );

    X_READ_FLAG( sps_long_term_ref_pics_flag );
    sps->setLongTermRefsPresent( sps_long_term_ref_pics_flag );

    if( sps_video_parameter_set_id > 0 )
    {
        X_READ_FLAG( sps_inter_layer_prediction_enabled_flag );
        sps->setInterLayerPresentFlag( sps_inter_layer_prediction_enabled_flag );
        // CHECK( vps->getIndependentLayerFlag( vps->getGeneralLayerIdx( nuh_layer_id ) ) == 1 && sps_inter_layer_prediction_enabled_flag != 0,
        //        "When vps_independent_layer_flag[ GeneralLayerIdx[ nuh_layer_id ] ] is equal to 1,"
        //        " the value of sps_inter_layer_prediction_enabled_flag shall be equal to 0." )
    }

    X_READ_FLAG( sps_idr_rpl_present_flag );
    sps->setIDRRefParamListPresent( sps_idr_rpl_present_flag );

    X_READ_FLAG( sps_rpl1_same_as_rpl0_flag );
    sps->setRPL1CopyFromRPL0Flag( sps_rpl1_same_as_rpl0_flag );

    for( unsigned i = 0; i < ( sps_rpl1_same_as_rpl0_flag ? 1 : 2 ); i++ ) {
        X_READ_UVLC_idx( sps_num_ref_pic_lists, "[i]", 0, 64 );
    }
    if( sps_rpl1_same_as_rpl0_flag ) {
    }


    X_READ_FLAG( sps_ref_wraparound_enabled_flag );
    sps->setUseWrapAround( sps_ref_wraparound_enabled_flag );
    for( int i = 0; i < sps->getNumSubPics(); ++i )
    {
        CHECK( sps->getSubPicTreatedAsPicFlag( i ) == 1 && sps->getSubPicWidth( i ) != ( sps_pic_width_max_in_luma_samples + CtbSizeY - 1 ) >> CtbLog2SizeY
                && sps_ref_wraparound_enabled_flag != 0,
            "It is a requirement of bitstream conformance that, when there is one or more values of i in the range of 0 to sps_num_subpics_minus1, inclusive,"
            " for which sps_subpic_treated_as_pic_flag[ i ] is equal to 1 and sps_subpic_width_minus1[ i ] plus 1 is not equal to"
            " ( sps_pic_width_max_in_luma_samples + CtbSizeY− 1 ) >> CtbLog2SizeY ), the value of sps_ref_wraparound_enabled_flag shall be equal to 0." );
    }

    X_READ_FLAG( sps_temporal_mvp_enabled_flag );
    sps->setSPSTemporalMVPEnabledFlag( sps_temporal_mvp_enabled_flag );

    if( sps_temporal_mvp_enabled_flag )
    {
        X_READ_FLAG( sps_sbtmvp_enabled_flag );
        sps->setSBTMVPEnabledFlag( sps_sbtmvp_enabled_flag );
    }

    X_READ_FLAG( sps_amvr_enabled_flag );
    sps->setAMVREnabledFlag( sps_amvr_enabled_flag );

    X_READ_FLAG( sps_bdof_enabled_flag );
    sps->setUseBIO( sps_bdof_enabled_flag );

    if( sps_bdof_enabled_flag )
    {
        X_READ_FLAG( sps_bdof_control_present_in_ph_flag );
        sps->setBdofControlPresentInPhFlag( sps_bdof_control_present_in_ph_flag );
    }

    X_READ_FLAG( sps_smvd_enabled_flag );
    sps->setUseSMVD( sps_smvd_enabled_flag );

    X_READ_FLAG( sps_dmvr_enabled_flag );
    sps->setUseDMVR( sps_dmvr_enabled_flag );

    if( sps_dmvr_enabled_flag )
    {
        X_READ_FLAG( sps_dmvr_control_present_in_ph_flag );
        sps->setDmvrControlPresentInPhFlag( sps_dmvr_control_present_in_ph_flag );
    }

    X_READ_FLAG( sps_mmvd_enabled_flag );
    sps->setUseMMVD( sps_mmvd_enabled_flag );

    if( sps->getUseMMVD() )
    {
        X_READ_FLAG( sps_mmvd_fullpel_only_flag );
        sps->setFpelMmvdEnabledFlag( sps_mmvd_fullpel_only_flag );
    }

    X_READ_UVLC( sps_six_minus_max_num_merge_cand, 0, 5 );
    const unsigned MaxNumMergeCand = MRG_MAX_NUM_CANDS - sps_six_minus_max_num_merge_cand;
    sps->setMaxNumMergeCand( MaxNumMergeCand );

    X_READ_FLAG( sps_sbt_enabled_flag );
    sps->setUseSBT( sps_sbt_enabled_flag );

    X_READ_FLAG( sps_affine_enabled_flag );
    sps->setUseAffine( sps_affine_enabled_flag );

    if( sps_affine_enabled_flag )
    {
        X_READ_UVLC( sps_five_minus_max_num_subblock_merge_cand, 0, 5 - sps->getSBTMVPEnabledFlag() );
        X_READ_FLAG( sps_6param_affine_enabled_flag );
        sps->setUseAffineType( sps_6param_affine_enabled_flag );

        if( sps_amvr_enabled_flag )
        {
        X_READ_FLAG( sps_affine_amvr_enabled_flag );
        sps->setAffineAmvrEnabledFlag( sps_affine_amvr_enabled_flag );
        }

        X_READ_FLAG( sps_affine_prof_enabled_flag );
        sps->setUsePROF( sps_affine_prof_enabled_flag );

        if( sps_affine_prof_enabled_flag )
        {
        X_READ_FLAG( sps_prof_control_present_in_ph_flag );
        sps->setProfControlPresentInPhFlag( sps_prof_control_present_in_ph_flag );
        }
    }

    X_READ_FLAG( sps_bcw_enabled_flag );
    sps->setUseBcw( sps_bcw_enabled_flag );

    X_READ_FLAG( sps_ciip_enabled_flag );
    sps->setUseCiip( sps_ciip_enabled_flag );

    if( MaxNumMergeCand >= 2 )
    {
        X_READ_FLAG( sps_gpm_enabled_flag );
        sps->setUseGeo( sps_gpm_enabled_flag );

        if( sps_gpm_enabled_flag && MaxNumMergeCand >= 3 )
        {
        X_READ_UVLC( sps_max_num_merge_cand_minus_max_num_gpm_cand, 0, MaxNumMergeCand - 2 );
        sps->setMaxNumGeoCand( MaxNumMergeCand - sps_max_num_merge_cand_minus_max_num_gpm_cand );
        }
        else if( sps_gpm_enabled_flag )
        {
        sps->setMaxNumGeoCand( 2 );
        }
    }

    X_READ_UVLC( sps_log2_parallel_merge_level_minus2, 0, CtbLog2SizeY - 2 );
    sps->setLog2ParallelMergeLevelMinus2( sps_log2_parallel_merge_level_minus2 );

    X_READ_FLAG( sps_isp_enabled_flag );
    sps->setUseISP( sps_isp_enabled_flag );

    X_READ_FLAG( sps_mrl_enabled_flag );
    sps->setUseMRL( sps_mrl_enabled_flag );

    X_READ_FLAG( sps_mip_enabled_flag );
    sps->setUseMIP( sps_mip_enabled_flag );

    if( sps_chroma_format_idc != CHROMA_400 )
    {
        X_READ_FLAG( sps_cclm_enabled_flag );
        sps->setUseLMChroma( sps_cclm_enabled_flag );
    }

    if( sps_chroma_format_idc == CHROMA_420 )
    {
        X_READ_FLAG( sps_chroma_horizontal_collocated_flag );
        sps->setHorCollocatedChromaFlag( sps_chroma_horizontal_collocated_flag );

        X_READ_FLAG( sps_chroma_vertical_collocated_flag );
        sps->setVerCollocatedChromaFlag( sps_chroma_vertical_collocated_flag );
    }

    X_READ_FLAG( sps_palette_enabled_flag );
    CHECK( sps_palette_enabled_flag, "palette mode is not yet supported" );

    if( sps_chroma_format_idc == CHROMA_444 && sps->getLog2MaxTbSize() != 6 )
    {
        X_READ_FLAG( sps_act_enabled_flag );
        sps->setUseColorTrans( sps_act_enabled_flag );
    }

    if( sps_transform_skip_enabled_flag || sps_palette_enabled_flag )
    {
        X_READ_UVLC( sps_internal_bit_depth_minus_input_bit_depth, 0, 8 );   // Why is this called sps_min_qp_prime_ts in the standard?
        sps->setInternalMinusInputBitDepth( sps_internal_bit_depth_minus_input_bit_depth );
    }

    X_READ_FLAG( sps_ibc_enabled_flag );
    sps->setIBCFlag( sps_ibc_enabled_flag );

    X_READ_FLAG( sps_explicit_scaling_list_enabled_flag );
    sps->setScalingListFlag( sps_explicit_scaling_list_enabled_flag );

    if( sps_lfnst_enabled_flag && sps_explicit_scaling_list_enabled_flag )
    {
        X_READ_FLAG( sps_scaling_matrix_for_lfnst_disabled_flag );
        sps->setDisableScalingMatrixForLfnstBlks( sps_scaling_matrix_for_lfnst_disabled_flag );
    }

    if( sps->getUseColorTrans() && sps_explicit_scaling_list_enabled_flag )
    {
        X_READ_FLAG( sps_scaling_matrix_for_alternative_colour_space_disabled_flag );
        sps->setScalingMatrixForAlternativeColourSpaceDisabledFlag( sps_scaling_matrix_for_alternative_colour_space_disabled_flag );

        if( sps_scaling_matrix_for_alternative_colour_space_disabled_flag )
        {
        X_READ_FLAG( sps_scaling_matrix_designated_colour_space_flag );
        sps->setScalingMatrixDesignatedColourSpaceFlag( sps_scaling_matrix_designated_colour_space_flag );
        }
    }

    X_READ_FLAG( sps_dep_quant_enabled_flag );
    sps->setDepQuantEnabledFlag( sps_dep_quant_enabled_flag );

    X_READ_FLAG( sps_sign_data_hiding_enabled_flag );
    sps->setSignDataHidingEnabledFlag( sps_sign_data_hiding_enabled_flag );

    X_READ_FLAG( sps_virtual_boundaries_enabled_flag );
    sps->setVirtualBoundariesEnabledFlag( sps_virtual_boundaries_enabled_flag );

    if( sps_virtual_boundaries_enabled_flag )
    {
        X_READ_FLAG( sps_virtual_boundaries_present_flag );
        sps->setVirtualBoundariesPresentFlag( sps_virtual_boundaries_present_flag );

        if( sps_virtual_boundaries_present_flag )
        {
        X_READ_UVLC( sps_num_ver_virtual_boundaries, 0, sps_pic_width_max_in_luma_samples <= 8 ? 0 : 3 );
        sps->setNumVerVirtualBoundaries( sps_num_ver_virtual_boundaries );

        for( unsigned i = 0; i < sps_num_ver_virtual_boundaries; i++ )
        {
            X_READ_UVLC_idx( sps_virtual_boundary_pos_x_minus1, "[i]", 0, ( sps_pic_width_max_in_luma_samples + 7 ) / 8 - 2 );
            sps->setVirtualBoundariesPosX( ( sps_virtual_boundary_pos_x_minus1 + 1 ) << 3, i );
        }

        X_READ_UVLC_idx( sps_num_hor_virtual_boundaries, "[i]", 0, sps_pic_height_max_in_luma_samples <= 8 ? 0 : 3  );
        sps->setNumHorVirtualBoundaries( sps_num_hor_virtual_boundaries );

        for( unsigned i = 0; i <sps_num_hor_virtual_boundaries; i++ )
        {
            X_READ_UVLC_idx( sps_virtual_boundary_pos_y_minus1, "[i]", 0, ( sps_pic_height_max_in_luma_samples + 7 ) / 8 - 2 );
            sps->setVirtualBoundariesPosY( (sps_virtual_boundary_pos_y_minus1 + 1) << 3, i );
        }
        }
    }

    if( sps_ptl_dpb_hrd_params_present_flag ) {

    }

    X_READ_FLAG( sps_field_seq_flag );
    sps->setFieldSeqFlag( sps_field_seq_flag );

    X_READ_FLAG( sps_vui_parameters_present_flag );
    sps->setVuiParametersPresentFlag( sps_vui_parameters_present_flag );

    if( sps_vui_parameters_present_flag )
    {
        X_READ_UVLC( sps_vui_payload_size_minus1, 0, 1023 );
        sps->setVuiPayloadSize( sps_vui_payload_size_minus1 + 1 );

        while( !isByteAligned() )
        {
        X_READ_FLAG( sps_vui_alignment_zero_bit );
        CHECK( sps_vui_alignment_zero_bit, "sps_vui_alignment_zero_bit not equal to 0" );
        }
    }

    X_READ_FLAG( sps_extension_present_flag );
    if( sps_extension_present_flag )
    {
        while( xMoreRbspData() )
        {
        X_READ_FLAG( sps_extension_data_flag );
        (void)sps_extension_data_flag;
        }
    }

    xReadRbspTrailingBits();
}

bool HLSyntaxReader::xMoreRbspData() {
    int bitsLeft = m_pcBitstream->getNumBitsLeft();

    // if there are more than 8 bits, it cannot be rbsp_trailing_bits
    if( bitsLeft > 8 )
    {
        return true;
    }

    uint8_t lastByte = m_pcBitstream->peekBits( bitsLeft );
    int cnt = bitsLeft;

    // remove trailing bits equal to zero
    while( ( cnt > 0 ) && ( ( lastByte & 1 ) == 0 ) )
    {
        lastByte >>= 1;
        cnt--;
    }
    // remove bit equal to one
    cnt--;

    // we should not have a negative number of bits
    CHECK( cnt<0, "Negative number of bits") ;

    // we have more data, if cnt is not zero
    return ( cnt>0 );
}

void VLCReader::xReadFlag( uint32_t& ruiCode )
{
    ruiCode = m_pcBitstream->read( 1 );
}

void VLCReader::xReadUvlc( uint32_t& ruiVal )
{
    uint32_t uiVal = 0;
    uint32_t uiCode = 0;
    uint32_t uiLength = 0;

    uiCode = m_pcBitstream->read( 1 );
    if( 0 == uiCode )
    {
        uiLength = 0;

        while( ! ( uiCode & 1 ) )
        {
            uiCode = m_pcBitstream->read( 1 );
            uiLength++;
        }

        uiVal = m_pcBitstream->read( uiLength );

        uiVal += ( 1 << uiLength ) - 1;
    }

    ruiVal = uiVal;
}

void VLCReader::xReadSvlc( int32_t& riVal )
{
    uint32_t uiBits = m_pcBitstream->read( 1 );
    if( 0 == uiBits )
    {
        uint32_t uiLength = 0;

        while( ! ( uiBits & 1 ) )
        {
            uiBits = m_pcBitstream->read( 1 );
            uiLength++;
        }

        uiBits = m_pcBitstream->read( uiLength );

        uiBits += ( 1 << uiLength );
        riVal = ( uiBits & 1 ) ? -(int32_t)( uiBits>>1 ) : (int32_t)( uiBits>>1 );
    }
    else
    {
        riVal = 0;
    }
}

void VLCReader::xReadCode( uint32_t uiLength, uint32_t& ruiCode )
{
    CHECK( uiLength == 0, "Reading a code of lenght '0'" );
    ruiCode = m_pcBitstream->read( uiLength );
}

void VLCReader::xReadSCode( uint32_t length, int32_t& value )
{
    CHECK( length == 0 || length > 31, "wrong" );
    uint32_t val = m_pcBitstream->read( length );
    value = length >= 32 ? int32_t( val ) : ( ( -int32_t( val & ( uint32_t( 1 ) << ( length - 1 ) ) ) ) | int32_t( val ) );
}

void VLCReader::xReadFlag( uint32_t& rValue, const char *pSymbolName )
{
    xReadFlag( rValue );
}

void VLCReader::xReadUvlc( uint32_t& rValue, const char *pSymbolName )
{
    xReadUvlc( rValue );
}

void VLCReader::xReadSvlc( int32_t& rValue, const char *pSymbolName )
{
    xReadSvlc( rValue );
}

void VLCReader::xReadCode( uint32_t length, uint32_t& rValue, const char *pSymbolName )
{
    xReadCode( length, rValue );
}

void VLCReader::xReadSCode( uint32_t length, int32_t& value, const char *pSymbolName )
{
    xReadSCode( length, value );
}

// ====================================================================================================================
//  read functions returning the result value
// ====================================================================================================================

bool VLCReader::xReadFlag()
{
  return m_pcBitstream->read( 1 ) & 0x1;
}

uint32_t VLCReader::xReadUvlc()
{
    uint32_t uiVal = 0;
    uint32_t uiCode = 0;
    uint32_t uiLength = 0;

    uiCode = m_pcBitstream->read( 1 );
    if( 0 == uiCode )
    {
        uiLength = 0;

        while( ! ( uiCode & 1 ) )
        {
            uiCode = m_pcBitstream->read( 1 );
            uiLength++;
        }

        uiVal = m_pcBitstream->read( uiLength );

        uiVal += ( 1 << uiLength ) - 1;
    }

    return uiVal;
}

int32_t VLCReader::xReadSvlc()
{
    uint32_t uiBits = m_pcBitstream->read( 1 );
    if( 0 == uiBits )
    {
        uint32_t uiLength = 0;

        while( ! ( uiBits & 1 ) )
        {
            uiBits = m_pcBitstream->read( 1 );
            uiLength++;
        }

        uiBits = m_pcBitstream->read( uiLength );

        uiBits += ( 1 << uiLength );
        return ( uiBits & 1 ) ? -(int32_t)( uiBits>>1 ) : (int32_t)( uiBits>>1 );
    }
    else
    {
        return  0;
    }
}

uint32_t VLCReader::xReadCode( uint32_t uiLength )
{
    CHECK( uiLength == 0, "Reading a code of lenght '0'" );
    return m_pcBitstream->read( uiLength );
}

int32_t VLCReader::xReadSCode( uint32_t length )
{
    CHECK( length == 0 || length > 31, "wrong" );
    uint32_t val = m_pcBitstream->read( length );
    return length >= 32 ? int32_t( val ) : ( ( -int32_t( val & ( uint32_t( 1 ) << ( length - 1 ) ) ) ) | int32_t( val ) );
}

bool VLCReader::xReadFlag( const char* pSymbolName )
{
    bool value = xReadFlag();
    return value;
}

uint32_t VLCReader::xReadUvlc( const char* pSymbolName )
{
    uint32_t value = xReadUvlc();
    return value;
}

int32_t VLCReader::xReadSvlc( const char* pSymbolName )
{
    int32_t value = xReadSvlc();
    return value;
}

uint32_t VLCReader::xReadCode( uint32_t length, const char* pSymbolName )
{
    uint32_t value = xReadCode( length );
    return value;
}

int32_t VLCReader::xReadSCode( uint32_t length, const char* pSymbolName )
{
    int32_t value = xReadSCode( length );
    return value;
}

void VLCReader::xReadRbspTrailingBits()
{
    X_READ_FLAG( rbsp_stop_one_bit );
    CHECK( rbsp_stop_one_bit != 1, "Trailing bit not '1'" );
    int cnt = 0;
    while( m_pcBitstream->getNumBitsUntilByteAligned() )
    {
        X_READ_FLAG( rbsp_alignment_zero_bit );
        CHECK( rbsp_alignment_zero_bit != 0, "Alignment bit is not '0'" );
        cnt++;
    }
    CHECK( cnt >= 8, "Read more than '8' trailing bits" );
}