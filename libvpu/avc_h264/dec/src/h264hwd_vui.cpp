/*------------------------------------------------------------------------------
--                                                                            --
--       This software is confidential and proprietary and may be used        --
--        only as expressly authorized by a licensing agreement from          --
--                                                                            --
--                            Hantro Products Oy.                             --
--                                                                            --
--                   (C) COPYRIGHT 2006 HANTRO PRODUCTS OY                    --
--                            ALL RIGHTS RESERVED                             --
--                                                                            --
--                 The entire notice above must be reproduced                 --
--                  on all copies and should not be removed.                  --
--                                                                            --
--------------------------------------------------------------------------------
--
--  Abstract : Decode Video Usability Information (VUI) from the stream
--
--------------------------------------------------------------------------------
--
--  Version control information, please leave untouched.
--
--  $RCSfile: h264hwd_vui.c,v $
--  $Date: 2010/04/22 06:54:56 $
--  $Revision: 1.4 $
--
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------

    Table of contents

     1. Include headers
     2. External compiler flags
     3. Module defines
     4. Local function prototypes
     5. Functions
          h264bsdDecodeVuiParameters
          h264bsdDecodeHrdParameters

------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/

#include "dwl.h"
#include "h264hwd_vui.h"
#include "basetype.h"
#include "h264hwd_vlc.h"
#include "h264hwd_stream.h"
#include "h264hwd_util.h"

/*------------------------------------------------------------------------------
    2. External compiler flags
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
    3. Module defines
------------------------------------------------------------------------------*/

#define MAX_DPB_SIZE 16
#define MAX_BR       240000 /* for level 5.1 */
#define MAX_CPB      240000 /* for level 5.1 */

/*------------------------------------------------------------------------------
    4. Local function prototypes
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------

    Function: h264bsdDecodeVuiParameters

        Functional description:
            Decode VUI parameters from the stream. See standard for details.

        Inputs:
            pStrmData       pointer to stream data structure

        Outputs:
            pVuiParameters  decoded information is stored here

        Returns:
            HANTRO_OK       success
            HANTRO_NOK      invalid stream data or end of stream

------------------------------------------------------------------------------*/

u32 h264bsdDecodeVuiParameters(strmData_t *pStrmData,
    vuiParameters_t *pVuiParameters)
{

/* Variables */

    u32 tmp;

/* Code */

    ASSERT(pStrmData);
    ASSERT(pVuiParameters);

    memset(pVuiParameters, 0, sizeof(vuiParameters_t));

    tmp = h264bsdGetBits(pStrmData, 1);
    if (tmp == END_OF_STREAM)
        return(END_OF_STREAM);
    pVuiParameters->aspectRatioPresentFlag = (tmp == 1) ? HANTRO_TRUE : HANTRO_FALSE;

    if (pVuiParameters->aspectRatioPresentFlag)
    {
        tmp = h264bsdGetBits(pStrmData, 8);
        if (tmp == END_OF_STREAM)
            return(END_OF_STREAM);
        pVuiParameters->aspectRatioIdc = tmp;

        if (pVuiParameters->aspectRatioIdc == ASPECT_RATIO_EXTENDED_SAR)
        {
            tmp = h264bsdGetBits(pStrmData, 16);
            if (tmp == END_OF_STREAM)
                return(END_OF_STREAM);
            pVuiParameters->sarWidth = tmp;

            tmp = h264bsdGetBits(pStrmData, 16);
            if (tmp == END_OF_STREAM)
                return(END_OF_STREAM);
            pVuiParameters->sarHeight = tmp;
        }
    }

    tmp = h264bsdGetBits(pStrmData, 1);
    if (tmp == END_OF_STREAM)
        return(END_OF_STREAM);
    pVuiParameters->overscanInfoPresentFlag = (tmp == 1) ? HANTRO_TRUE : HANTRO_FALSE;

    if (pVuiParameters->overscanInfoPresentFlag)
    {
        tmp = h264bsdGetBits(pStrmData, 1);
        if (tmp == END_OF_STREAM)
            return(END_OF_STREAM);
        pVuiParameters->overscanAppropriateFlag = (tmp == 1) ? HANTRO_TRUE : HANTRO_FALSE;
    }

    tmp = h264bsdGetBits(pStrmData, 1);
    if (tmp == END_OF_STREAM)
        return(END_OF_STREAM);
    pVuiParameters->videoSignalTypePresentFlag = (tmp == 1) ? HANTRO_TRUE : HANTRO_FALSE;

    if (pVuiParameters->videoSignalTypePresentFlag)
    {
        tmp = h264bsdGetBits(pStrmData, 3);
        if (tmp == END_OF_STREAM)
            return(END_OF_STREAM);
        pVuiParameters->videoFormat = tmp;

        tmp = h264bsdGetBits(pStrmData, 1);
        if (tmp == END_OF_STREAM)
            return(END_OF_STREAM);
        pVuiParameters->videoFullRangeFlag = (tmp == 1) ? HANTRO_TRUE : HANTRO_FALSE;

        tmp = h264bsdGetBits(pStrmData, 1);
        if (tmp == END_OF_STREAM)
            return(END_OF_STREAM);
        pVuiParameters->colourDescriptionPresentFlag =
            (tmp == 1) ? HANTRO_TRUE : HANTRO_FALSE;

        if (pVuiParameters->colourDescriptionPresentFlag)
        {
            tmp = h264bsdGetBits(pStrmData, 8);
            if (tmp == END_OF_STREAM)
                return(END_OF_STREAM);
            pVuiParameters->colourPrimaries = tmp;

            tmp = h264bsdGetBits(pStrmData, 8);
            if (tmp == END_OF_STREAM)
                return(END_OF_STREAM);
            pVuiParameters->transferCharacteristics = tmp;

            tmp = h264bsdGetBits(pStrmData, 8);
            if (tmp == END_OF_STREAM)
                return(END_OF_STREAM);
            pVuiParameters->matrixCoefficients = tmp;
        }
        else
        {
            pVuiParameters->colourPrimaries         = 2;
            pVuiParameters->transferCharacteristics = 2;
            pVuiParameters->matrixCoefficients      = 2;
        }
    }
    else
    {
        pVuiParameters->videoFormat             = 5;
        pVuiParameters->colourPrimaries         = 2;
        pVuiParameters->transferCharacteristics = 2;
        pVuiParameters->matrixCoefficients      = 2;
    }

    tmp = h264bsdGetBits(pStrmData, 1);
    if (tmp == END_OF_STREAM)
        return(END_OF_STREAM);
    pVuiParameters->chromaLocInfoPresentFlag = (tmp == 1) ? HANTRO_TRUE : HANTRO_FALSE;

    if (pVuiParameters->chromaLocInfoPresentFlag)
    {
        tmp = h264bsdDecodeExpGolombUnsigned(pStrmData,
          &pVuiParameters->chromaSampleLocTypeTopField);
        if (tmp != HANTRO_OK)
            return(tmp);
        if (pVuiParameters->chromaSampleLocTypeTopField > 5)
            return(END_OF_STREAM);

        tmp = h264bsdDecodeExpGolombUnsigned(pStrmData,
          &pVuiParameters->chromaSampleLocTypeBottomField);
        if (tmp != HANTRO_OK)
            return(tmp);
        if (pVuiParameters->chromaSampleLocTypeBottomField > 5)
            return(END_OF_STREAM);
    }

    tmp = h264bsdGetBits(pStrmData, 1);
    if (tmp == END_OF_STREAM)
        return(END_OF_STREAM);
    pVuiParameters->timingInfoPresentFlag = (tmp == 1) ? HANTRO_TRUE : HANTRO_FALSE;

    if (pVuiParameters->timingInfoPresentFlag)
    {
        tmp = h264bsdShowBits(pStrmData,32);
        if (h264bsdFlushBits(pStrmData, 32) == END_OF_STREAM)
            return(END_OF_STREAM);
#ifdef HANTRO_PEDANTIC_MODE
        if (tmp == 0)
            return(HANTRO_NOK);
#endif /* HANTRO_PEDANTIC_MODE */
        pVuiParameters->numUnitsInTick = tmp;

        tmp = h264bsdShowBits(pStrmData,32);
        if (h264bsdFlushBits(pStrmData, 32) == END_OF_STREAM)
            return(END_OF_STREAM);
#ifdef HANTRO_PEDANTIC_MODE
        if (tmp == 0)
            return(HANTRO_NOK);
#endif /* HANTRO_PEDANTIC_MODE */
        pVuiParameters->timeScale = tmp;

        tmp = h264bsdGetBits(pStrmData, 1);
        if (tmp == END_OF_STREAM)
            return(END_OF_STREAM);
        pVuiParameters->fixedFrameRateFlag = (tmp == 1) ? HANTRO_TRUE : HANTRO_FALSE;
    }

    tmp = h264bsdGetBits(pStrmData, 1);
    if (tmp == END_OF_STREAM)
        return(END_OF_STREAM);
    pVuiParameters->nalHrdParametersPresentFlag = (tmp == 1) ? HANTRO_TRUE : HANTRO_FALSE;

    if (pVuiParameters->nalHrdParametersPresentFlag)
    {
        tmp = h264bsdDecodeHrdParameters(pStrmData,
            &pVuiParameters->nalHrdParameters);
        if (tmp != HANTRO_OK)
        {
            /* ignore error */
            return(HANTRO_OK);
        }
    }
    else
    {
        pVuiParameters->nalHrdParameters.cpbCnt          = 1;
        /* MaxBR and MaxCPB should be the values correspondig to the levelIdc
         * in the SPS containing these VUI parameters. However, these values
         * are not used anywhere and maximum for any level will be used here */
        pVuiParameters->nalHrdParameters.bitRateValue[0] = 1200 * MAX_BR + 1;
        pVuiParameters->nalHrdParameters.cpbSizeValue[0] = 1200 * MAX_CPB + 1;
        pVuiParameters->nalHrdParameters.initialCpbRemovalDelayLength = 24;
        pVuiParameters->nalHrdParameters.cpbRemovalDelayLength        = 24;
        pVuiParameters->nalHrdParameters.dpbOutputDelayLength         = 24;
        pVuiParameters->nalHrdParameters.timeOffsetLength             = 24;
    }

    tmp = h264bsdGetBits(pStrmData, 1);
    if (tmp == END_OF_STREAM)
        return(END_OF_STREAM);
    pVuiParameters->vclHrdParametersPresentFlag = (tmp == 1) ? HANTRO_TRUE : HANTRO_FALSE;

    if (pVuiParameters->vclHrdParametersPresentFlag)
    {
        tmp = h264bsdDecodeHrdParameters(pStrmData,
            &pVuiParameters->vclHrdParameters);
        if (tmp != HANTRO_OK)
        {
            /* ignore error */
            return(HANTRO_OK);
        }
    }
    else
    {
        pVuiParameters->vclHrdParameters.cpbCnt          = 1;
        /* MaxBR and MaxCPB should be the values correspondig to the levelIdc
         * in the SPS containing these VUI parameters. However, these values
         * are not used anywhere and maximum for any level will be used here */
        pVuiParameters->vclHrdParameters.bitRateValue[0] = 1000 * MAX_BR + 1;
        pVuiParameters->vclHrdParameters.cpbSizeValue[0] = 1000 * MAX_CPB + 1;
        pVuiParameters->vclHrdParameters.initialCpbRemovalDelayLength = 24;
        pVuiParameters->vclHrdParameters.cpbRemovalDelayLength        = 24;
        pVuiParameters->vclHrdParameters.dpbOutputDelayLength         = 24;
        pVuiParameters->vclHrdParameters.timeOffsetLength             = 24;
    }

    if (pVuiParameters->nalHrdParametersPresentFlag ||
      pVuiParameters->vclHrdParametersPresentFlag)
    {
        tmp = h264bsdGetBits(pStrmData, 1);
        if (tmp == END_OF_STREAM)
            return(END_OF_STREAM);
        pVuiParameters->lowDelayHrdFlag = (tmp == 1) ? HANTRO_TRUE : HANTRO_FALSE;
    }

    tmp = h264bsdGetBits(pStrmData, 1);
    if (tmp == END_OF_STREAM)
        return(END_OF_STREAM);
    pVuiParameters->picStructPresentFlag = (tmp == 1) ? HANTRO_TRUE : HANTRO_FALSE;

    tmp = h264bsdGetBits(pStrmData, 1);
    if (tmp == END_OF_STREAM)
        return(END_OF_STREAM);
    pVuiParameters->bitstreamRestrictionFlag = (tmp == 1) ? HANTRO_TRUE : HANTRO_FALSE;

    if (pVuiParameters->bitstreamRestrictionFlag)
    {
    	int vuistate = 0;
        tmp = h264bsdGetBits(pStrmData, 1);
        if (tmp == END_OF_STREAM)
            return(END_OF_STREAM);
        pVuiParameters->motionVectorsOverPicBoundariesFlag =
            (tmp == 1) ? HANTRO_TRUE : HANTRO_FALSE;

        tmp = h264bsdDecodeExpGolombUnsigned(pStrmData,
          &pVuiParameters->maxBytesPerPicDenom);
        if (tmp != HANTRO_OK)
            vuistate = 1;//return(HANTRO_OK);
		
#ifdef HANTRO_PEDANTIC_MODE
        if (pVuiParameters->maxBytesPerPicDenom > 16)
            return(HANTRO_NOK);
#endif /* HANTRO_PEDANTIC_MODE */

        tmp = h264bsdDecodeExpGolombUnsigned(pStrmData,
          &pVuiParameters->maxBitsPerMbDenom);
        if (tmp != HANTRO_OK)
            vuistate = 1;//return(HANTRO_OK);
#ifdef HANTRO_PEDANTIC_MODE
        if (pVuiParameters->maxBitsPerMbDenom > 16)
            return(HANTRO_NOK);
#endif /* HANTRO_PEDANTIC_MODE */

        tmp = h264bsdDecodeExpGolombUnsigned(pStrmData,
          &pVuiParameters->log2MaxMvLengthHorizontal);
        if (tmp != HANTRO_OK)
            vuistate = 1;//return(HANTRO_OK);
#ifdef HANTRO_PEDANTIC_MODE
        if (pVuiParameters->log2MaxMvLengthHorizontal > 16)
            return(HANTRO_NOK);
#endif /* HANTRO_PEDANTIC_MODE */

        tmp = h264bsdDecodeExpGolombUnsigned(pStrmData,
          &pVuiParameters->log2MaxMvLengthVertical);
        if (tmp != HANTRO_OK)
            vuistate = 1;//return(HANTRO_OK);
#ifdef HANTRO_PEDANTIC_MODE
        if (pVuiParameters->log2MaxMvLengthVertical > 16)
            return(HANTRO_NOK);
#endif /* HANTRO_PEDANTIC_MODE */

        tmp = h264bsdDecodeExpGolombUnsigned(pStrmData,
          &pVuiParameters->numReorderFrames);
        if (tmp != HANTRO_OK)
            vuistate = 1;//return(HANTRO_OK);

        tmp = h264bsdDecodeExpGolombUnsigned(pStrmData,
          &pVuiParameters->maxDecFrameBuffering);
        if (tmp != HANTRO_OK)
            vuistate = 1;//return(HANTRO_OK);

		if(vuistate)
		{
			pVuiParameters->motionVectorsOverPicBoundariesFlag = HANTRO_TRUE;
			pVuiParameters->maxBytesPerPicDenom 	  = 2;
			pVuiParameters->maxBitsPerMbDenom		  = 1;
			pVuiParameters->log2MaxMvLengthHorizontal = 16;
			pVuiParameters->log2MaxMvLengthVertical   = 16;
			pVuiParameters->numReorderFrames		  = MAX_DPB_SIZE;
			pVuiParameters->maxDecFrameBuffering	  = MAX_DPB_SIZE;
		}
    }
    else
    {
        pVuiParameters->motionVectorsOverPicBoundariesFlag = HANTRO_TRUE;
        pVuiParameters->maxBytesPerPicDenom       = 2;
        pVuiParameters->maxBitsPerMbDenom         = 1;
        pVuiParameters->log2MaxMvLengthHorizontal = 16;
        pVuiParameters->log2MaxMvLengthVertical   = 16;
        pVuiParameters->numReorderFrames          = MAX_DPB_SIZE;
        pVuiParameters->maxDecFrameBuffering      = MAX_DPB_SIZE;
    }

    return(HANTRO_OK);

}

/*------------------------------------------------------------------------------

    Function: h264bsdDecodeHrdParameters

        Functional description:
            Decode HRD parameters from the stream. See standard for details.

        Inputs:
            pStrmData       pointer to stream data structure

        Outputs:
            pHrdParameters  decoded information is stored here

        Returns:
            HANTRO_OK       success
            HANTRO_NOK      invalid stream data

------------------------------------------------------------------------------*/

u32 h264bsdDecodeHrdParameters(
  strmData_t *pStrmData,
  hrdParameters_t *pHrdParameters)
{

/* Variables */

    u32 tmp, i;

/* Code */

    ASSERT(pStrmData);
    ASSERT(pHrdParameters);


    tmp = h264bsdDecodeExpGolombUnsigned(pStrmData, &pHrdParameters->cpbCnt);
    if (tmp != HANTRO_OK)
        return(tmp);
    /* cpbCount = cpb_cnt_minus1 + 1 */
    pHrdParameters->cpbCnt++;
    if (pHrdParameters->cpbCnt > MAX_CPB_CNT)
        return(HANTRO_NOK);

    tmp = h264bsdGetBits(pStrmData, 4);
    if (tmp == END_OF_STREAM)
        return(END_OF_STREAM);
    pHrdParameters->bitRateScale = tmp;

    tmp = h264bsdGetBits(pStrmData, 4);
    if (tmp == END_OF_STREAM)
        return(END_OF_STREAM);
    pHrdParameters->cpbSizeScale = tmp;

    for (i = 0; i < pHrdParameters->cpbCnt; i++)
    {
        /* bit_rate_value_minus1 in the range [0, 2^32 - 2] */
        tmp = h264bsdDecodeExpGolombUnsigned(pStrmData,
          &pHrdParameters->bitRateValue[i]);
        if (tmp != HANTRO_OK)
            return(tmp);
        if (pHrdParameters->bitRateValue[i] > 4294967294U)
            return(HANTRO_NOK);
        pHrdParameters->bitRateValue[i]++;
        /* this may result in overflow, but this value is not used for
         * anything */
        pHrdParameters->bitRateValue[i] *=
            1 << (6 + pHrdParameters->bitRateScale);

        /* cpb_size_value_minus1 in the range [0, 2^32 - 2] */
        tmp = h264bsdDecodeExpGolombUnsigned(pStrmData,
          &pHrdParameters->cpbSizeValue[i]);
        if (tmp != HANTRO_OK)
            return(tmp);
        if (pHrdParameters->cpbSizeValue[i] > 4294967294U)
            return(HANTRO_NOK);
        pHrdParameters->cpbSizeValue[i]++;
        /* this may result in overflow, but this value is not used for
         * anything */
        pHrdParameters->cpbSizeValue[i] *=
            1 << (4 + pHrdParameters->cpbSizeScale);

        tmp = h264bsdGetBits(pStrmData, 1);
        if (tmp == END_OF_STREAM)
            return(END_OF_STREAM);
        pHrdParameters->cbrFlag[i] = (tmp == 1) ? HANTRO_TRUE : HANTRO_FALSE;
    }

    tmp = h264bsdGetBits(pStrmData, 5);
    if (tmp == END_OF_STREAM)
        return(END_OF_STREAM);
    pHrdParameters->initialCpbRemovalDelayLength = tmp + 1;

    tmp = h264bsdGetBits(pStrmData, 5);
    if (tmp == END_OF_STREAM)
        return(END_OF_STREAM);
    pHrdParameters->cpbRemovalDelayLength = tmp + 1;

    tmp = h264bsdGetBits(pStrmData, 5);
    if (tmp == END_OF_STREAM)
        return(END_OF_STREAM);
    pHrdParameters->dpbOutputDelayLength = tmp + 1;

    tmp = h264bsdGetBits(pStrmData, 5);
    if (tmp == END_OF_STREAM)
        return(END_OF_STREAM);
    pHrdParameters->timeOffsetLength = tmp;

    return(HANTRO_OK);

}
