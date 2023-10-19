/**
 * \file IfxDsadc_Rdc.c
 * \brief DSADC RDC details
 *
 * \version iLLD_1_0_1_12_0
 * \copyright Copyright (c) 2017 Infineon Technologies AG. All rights reserved.
 *
 *
 *                                 IMPORTANT NOTICE
 *
 * Use of this file is subject to the terms of use agreed between (i) you or
 * the company in which ordinary course of business you are acting and (ii)
 * Infineon Technologies AG or its licensees. If and as long as no such terms
 * of use are agreed, use of this file is subject to following:
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer, must
 * be included in all copies of the Software, in whole or in part, and all
 * derivative works of the Software, unless such copies or derivative works are
 * solely in the form of machine-executable object code generated by a source
 * language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/

#include "IfxDsadc_Rdc.h"
#include "string.h"
#include "Gtm/Trig/IfxGtm_Trig.h"

/** \addtogroup IfxLld_Dsadc_Rdc_func_config
 * \{ */

/******************************************************************************/
/*-----------------------Private Function Prototypes--------------------------*/
/******************************************************************************/

/**
 * \param gtm Pointer to GTM module registers
 * \param tim TIM Number
 * \param timCh TIM Channel Number
 * \param risingEdge Signal level control
 */
IFX_STATIC Ifx_GTM_TIM_CH *IfxDsadc_Rdc_initGtmTim(Ifx_GTM *gtm, IfxGtm_Tim tim, IfxGtm_Tim_Ch timCh, boolean risingEdge);

/** \brief Initialise the DSADC hardware channels
 * \param driver Driver handle
 * \param config DSADC RDC configuration structure
 */
IFX_STATIC boolean IfxDsadc_Rdc_initHwChannels(IfxDsadc_Rdc *driver, const IfxDsadc_Rdc_Config *config);

/** \brief Initialise the GTM timestamp hardware resources
 * \param driver Driver handle
 * \param config DSADC RDC configuration structure
 * \return None
 */
IFX_STATIC void IfxDsadc_Rdc_initHwTimestamp(IfxDsadc_Rdc *driver, const IfxDsadc_Rdc_Config *config);

/** \} */

/** \addtogroup IfxLld_Dsadc_Rdc_func_utility
 * \{ */
/******************************************************************************/
/*------------------------Inline Function Prototypes--------------------------*/
/******************************************************************************/

/** \brief Updating RDC timestamps
 * \param driver Driver handle
 * \return None
 */
IFX_INLINE void IfxDsadc_Rdc_updateTimestamp(IfxDsadc_Rdc *driver);

/******************************************************************************/
/*-----------------------Private Function Prototypes--------------------------*/
/******************************************************************************/

/** \brief Group Dalay calculation
 * \param driver Driver handle
 */
IFX_STATIC float32 IfxDsadc_Rdc_calculateGroupDelay(IfxDsadc_Rdc *driver);

/**
 * \param config DSADC RDC configuration structure
 */
IFX_STATIC float32 IfxDsadc_Rdc_calculateTimestampPeriod(const IfxDsadc_Rdc_Config *config);

/** \brief Return the update period.
 * \param driver Driver handle
 * \return Return the update period in [seconds], i.e. the period of new result (and interrupt)
 */
IFX_STATIC float32 IfxDsadc_Rdc_getUpdatePeriod(IfxDsadc_Rdc *driver);

/** \} */

/** \addtogroup IfxLld_Dsadc_Rdc_variable
 * \{ */

/******************************************************************************/
/*------------------------Private Variables/Constants-------------------------*/
/******************************************************************************/

/** \brief GTM TIM Configuration settings
 */
IFX_STATIC IFX_CONST Ifx_GTM_TIM_CH_CTRL_Bits IfxDsadc_Rdc_GtmTimCfg = {
    .TIM_EN   = 1,
    .TIM_MODE = IfxGtm_Tim_Mode_inputEvent,
    .GPR0_SEL = IfxGtm_Tim_GprSel_tbuTs0,
    .GPR1_SEL = IfxGtm_Tim_GprSel_tbuTs0,
    .CNTS_SEL = IfxGtm_Tim_CntsSel_cntReg,
    /*.EGPR0_SEL= 0, //not available in A-step */
    .DSL      = 0, /* 0 = falling or low */
    .ISL      = 0, /* 1 = both edges, ignore DSL */
};

/** \} */

/******************************************************************************/
/*---------------------Inline Function Implementations------------------------*/
/******************************************************************************/

IFX_INLINE void IfxDsadc_Rdc_updateTimestamp(IfxDsadc_Rdc *driver)
{
    IfxDsadc_Rdc_Ts *timestamp  = &driver->timestamp;
    IfxDsadc_Rdc_Hw *hwHandle   = &driver->hardware;

    uint32           tsPwm      = hwHandle->pwmTimCh->GPR0.B.GPR0;
    uint32           clockTicks = (tsPwm - timestamp->rdc) % timestamp->maxTicks;
#    if IFXDSADC_RDC_CFG_DEBUG
    timestamp->inTicks   = clockTicks;
    timestamp->pwm       = tsPwm;
#    endif
    timestamp->inSeconds = timestamp->clockPeriod * (float32)clockTicks;
}


/******************************************************************************/
/*-------------------------Function Implementations---------------------------*/
/******************************************************************************/

IFX_STATIC float32 IfxDsadc_Rdc_calculateGroupDelay(IfxDsadc_Rdc *driver)
{
    Ifx_DSADC *dsadc = driver->hardware.inputSin.module;
    return IfxDsadc_getMainGroupDelay(dsadc, driver->hardware.inputSin.channelId);
}


IFX_STATIC float32 IfxDsadc_Rdc_calculateTimestampPeriod(const IfxDsadc_Rdc_Config *config)
{
    Ifx_GTM *gtm = config->hardware.gtmTimestamp.gtm;
    return 1.0F / IfxGtm_Tbu_getClockFrequency(gtm, IfxGtm_Tbu_Ts_0);
}


float32 IfxDsadc_Rdc_getAbsolutePosition(IfxDsadc_Rdc *driver)
{
    return Ifx_AngleTrkF32_getAbsolutePosition(&driver->angleTrk);
}


IfxStdIf_Pos_Dir IfxDsadc_Rdc_getDirection(IfxDsadc_Rdc *driver)
{
    return Ifx_AngleTrkF32_getDirection(&driver->angleTrk);
}


sint32 IfxDsadc_Rdc_getOffset(IfxDsadc_Rdc *driver)
{
    return Ifx_AngleTrkF32_getOffset(&driver->angleTrk);
}


float32 IfxDsadc_Rdc_getPosition(IfxDsadc_Rdc *driver)
{
    return Ifx_AngleTrkF32_getPosition(&driver->angleTrk);
}


float32 IfxDsadc_Rdc_getRefreshPeriod(IfxDsadc_Rdc *driver)
{
    return Ifx_AngleTrkF32_getRefreshPeriod(&driver->angleTrk);
}


sint32 IfxDsadc_Rdc_getResolution(IfxDsadc_Rdc *driver)
{
    return Ifx_AngleTrkF32_getResolution(&driver->angleTrk);
}


IfxStdIf_Pos_SensorType IfxDsadc_Rdc_getSensorType(IfxDsadc_Rdc *driver)
{
    return IfxStdIf_Pos_SensorType_resolver;
}


sint32 IfxDsadc_Rdc_getTurn(IfxDsadc_Rdc *driver)
{
    return Ifx_AngleTrkF32_getTurn(&driver->angleTrk);
}


IFX_STATIC float32 IfxDsadc_Rdc_getUpdatePeriod(IfxDsadc_Rdc *driver)
{
    IfxDsadc_Rdc_Hw *hwHandle = &(driver->hardware);
    return 1.0F / IfxDsadc_getIntegratorOutFreq(hwHandle->inputSin.module, hwHandle->inputSin.channelId);
}


boolean IfxDsadc_Rdc_init(IfxDsadc_Rdc *driver, const IfxDsadc_Rdc_Config *config)
{
    boolean result = TRUE;
    /* Initialise the DSADC hardware channels */
    result &= IfxDsadc_Rdc_initHwChannels(driver, config);

    /* Initialise the GTM timestamp hardware resources */
    IfxDsadc_Rdc_initHwTimestamp(driver, config);

    /* Initialise the software resources */
    {
        driver->updatePeriod          = IfxDsadc_Rdc_getUpdatePeriod(driver);
        driver->groupDelay            = IfxDsadc_Rdc_calculateGroupDelay(driver);
        driver->timestamp.enabled     = TRUE;
        driver->timestamp.clockPeriod = IfxDsadc_Rdc_calculateTimestampPeriod(config);
        driver->timestamp.maxTicks    = (uint32)(driver->updatePeriod / driver->timestamp.clockPeriod);
    }

    /* Initialise angle-tracking observer */
    {
        Ifx_AngleTrkF32_Config atoConfig;
        Ifx_AngleTrkF32_initConfig(&atoConfig, &(driver->sinIn), &(driver->cosIn));

        atoConfig.offset            = config->offset;
        atoConfig.speedLpfFc        = config->speedLpfFc;
        atoConfig.reversed          = config->reversed;
        atoConfig.errorThreshold    = config->errorThreshold;
        atoConfig.sqrAmplMax        = config->sqrAmplMax;
        atoConfig.sqrAmplMin        = config->sqrAmplMin;
        atoConfig.periodPerRotation = config->periodPerRotation;

        atoConfig.kp                = config->kp;
        atoConfig.ki                = config->ki;
        atoConfig.kd                = config->kd;
        atoConfig.resolution        = config->resolution;

#if IFXDSADC_RDC_CFG_PRE_OBSERVER_CORRECTION
        Ifx_AngleTrkF32_init(&(driver->angleTrk), &atoConfig, config->userTs);
#else
        Ifx_AngleTrkF32_init(&(driver->angleTrk), &atoConfig, driver->updatePeriod);
#endif
    }

    /* Optional calibration init */
    {}
    return result;
}


void IfxDsadc_Rdc_initConfig(IfxDsadc_Rdc_Config *config)
{
    config->kp                                   = 0; /* Force to used default from Ifx_AngleTrkF32 */
    config->ki                                   = 0;
    config->kd                                   = 0;
    config->speedLpfFc                           = 100;
    config->errorThreshold                       = 5.0f / 180 * IFX_PI;
    config->userTs                               = 0;
    config->sqrAmplMax                           = (sint32)((1.01f * 1.01f) * 2048);
    config->sqrAmplMin                           = (sint32)((0.99f * 0.99f) * 2048);
    config->periodPerRotation                    = 1;
    config->reversed                             = FALSE;
    config->resolution                           = (1UL << 12); /** \brief 12-bit default resolution */
    config->offset                               = 0;
    IfxDsadc_Dsadc_initCarrierGenConfig(&config->hardware.carrierGen);
    config->hardware.gtmTimestamp.gtm            = &MODULE_GTM;
    config->hardware.gtmTimestamp.pwmTim         = NULL_PTR;
    config->hardware.gtmTimestamp.rdcTim         = IfxGtm_Tim_0;
    config->hardware.gtmTimestamp.rdcTimChannel  = IfxGtm_Tim_Ch_0;
    config->hardware.gtmTimestamp.rdcTimMuxValue = 0;
    IfxDsadc_Dsadc_initChannelConfig(&config->hardware.inputConfig, NULL_PTR);
    config->hardware.inputCos                    = IfxDsadc_ChannelId_0;

    config->hardware.inputSin                    = IfxDsadc_ChannelId_2;

    config->hardware.outputClock                 = NULL_PTR;
    config->hardware.servReqPriority             = 0;
    config->hardware.servReqProvider             = IfxSrc_Tos_cpu0;
    config->hardware.startScan                   = FALSE;
}


IFX_STATIC Ifx_GTM_TIM_CH *IfxDsadc_Rdc_initGtmTim(Ifx_GTM *gtm, IfxGtm_Tim tim, IfxGtm_Tim_Ch timCh, boolean risingEdge)
{
    Ifx_GTM_TIM_CH     *pTimCh = IfxGtm_Tim_getChannel(&gtm->TIM[tim], timCh);
    Ifx_GTM_TIM_CH_CTRL ctrl;
    ctrl.B         = IfxDsadc_Rdc_GtmTimCfg;
    ctrl.B.DSL     = (risingEdge != FALSE) ? 1 : 0;
    pTimCh->CTRL.U = ctrl.U;
    return pTimCh;
}


IFX_STATIC boolean IfxDsadc_Rdc_initHwChannels(IfxDsadc_Rdc *driver, const IfxDsadc_Rdc_Config *config)
{
    //   #if IFXDSADC_HW_INIT
    IfxDsadc_Rdc_Hw             *hwHandle = &(driver->hardware);
    const IfxDsadc_Rdc_ConfigHw *configHw = &config->hardware;
    Ifx_DSADC                   *module   = configHw->inputConfig.module;
    boolean                      result   = (module != NULL_PTR) ? TRUE : FALSE;

    if (result != FALSE)
    {
        /* Initialise input channels */
        {
            IfxDsadc_Dsadc_ChannelConfig channelConfig = configHw->inputConfig;
            channelConfig.channelId = configHw->inputCos;
            IfxDsadc_Dsadc_initChannel(&hwHandle->inputCos, &channelConfig);

            channelConfig.channelId = configHw->inputSin;
            IfxDsadc_Dsadc_initChannel(&hwHandle->inputSin, &channelConfig);

            if (configHw->servReqPriority != 0)
            {
                IfxDsadc_ChannelId     ch = channelConfig.channelId;

                volatile Ifx_SRC_SRCR *srcr;

                if (ch == 0)
                {
                    srcr = &MODULE_SRC.DSADC.DSADC0.SRM;
                }

                else if (ch == 2)
                {
                    srcr = &MODULE_SRC.DSADC.DSADC2.SRM;
                }

                else if (ch == 3)
                {
                    srcr = &MODULE_SRC.DSADC.DSADC3.SRM;
                }

                IfxSrc_init(srcr, configHw->servReqProvider, configHw->servReqPriority);
                IfxSrc_enable(srcr);
            }
        }

        /* Initialise carrier generator, if necessary */
        if ((configHw->carrierGen.pinNeg != NULL_PTR) || (configHw->carrierGen.pinPos != NULL_PTR))
        {
            IfxDsadc_Dsadc_initCarrierGen(config->dsadc, &configHw->carrierGen);
        }

        /* Start conversion, if necessary */
        if (configHw->startScan != FALSE)
        {
            IfxDsadc_Rdc_startConversion(driver);
        }

        /* Initialise modulator clock output, if necessary */
        if (configHw->outputClock != NULL_PTR)
        {
            const IfxPort_Pin *clkPin = &(configHw->outputClock->pin);
            IfxPort_setPinModeOutput(clkPin->port, clkPin->pinIndex, configHw->carrierGen.pinMode, configHw->outputClock->select);
            IfxPort_setPinPadDriver(clkPin->port, clkPin->pinIndex, IfxPort_PadDriver_cmosAutomotiveSpeed1);
        }
    }

    return result;
}


IFX_STATIC void IfxDsadc_Rdc_initHwTimestamp(IfxDsadc_Rdc *driver, const IfxDsadc_Rdc_Config *config)
{
    IfxDsadc_Rdc_Hw                 *hwHandle = &(driver->hardware);
    const IfxDsadc_Rdc_ConfigHw     *configHw = &config->hardware;
    const IfxDsadc_Rdc_GtmTimestamp *tsConfig = &(configHw->gtmTimestamp);
    Ifx_GTM                         *gtm      = tsConfig->gtm;

    /** - Initialize TIM channel which is triggered by application's PWM */
    {
        boolean risingEdge = (configHw->inputConfig.demodulator.timestampTrigger == IfxDsadc_TimestampTrigger_risingEdge);
        hwHandle->pwmTimCh = IfxDsadc_Rdc_initGtmTim(gtm, tsConfig->pwmTim->tim, tsConfig->pwmTim->channel, risingEdge);

        /* connect into TIM timer input: */
        IfxGtm_PinMap_setTimTin(tsConfig->pwmTim, IfxPort_InputMode_undefined);
    }
    /** - Initialize TIM channel which is triggered by DSADC sample event */
    {
        hwHandle->rdcTimCh = IfxDsadc_Rdc_initGtmTim(gtm, tsConfig->rdcTim, tsConfig->rdcTimChannel, TRUE);

        /** - Connect DSADC trigger to TIM mux */
        IfxGtm_Trig_fromDsadc(gtm, configHw->inputSin, tsConfig->rdcTim, tsConfig->rdcTimChannel);

        /** - Connect into TIM input */
        uint32 shift = tsConfig->rdcTimChannel * 4;
        __ldmst_c(&(gtm->INOUTSEL.TIM[tsConfig->rdcTim].INSEL.U), (0xFU << shift), (tsConfig->rdcTimMuxValue << shift));
    }
    IfxGtm_Tbu_enableChannel(gtm, IfxGtm_Tbu_Ts_0);
}


void IfxDsadc_Rdc_onEventA(IfxDsadc_Rdc *driver)
{
    driver->timestamp.rdc = driver->hardware.rdcTimCh->GPR0.B.GPR0;

    Ifx_DSADC *dsadc = driver->hardware.inputSin.module;
    driver->sinIn = IfxDsadc_getMainResult(dsadc, driver->hardware.inputSin.channelId);
    driver->cosIn = IfxDsadc_getMainResult(dsadc, driver->hardware.inputCos.channelId);
    /*
     */

#if IFXDSADC_RDC_CFG_PRE_OBSERVER_CORRECTION == 0
    {
        sint16 sinIn = driver->sinIn;
        sint16 cosIn = driver->cosIn;

        /* tracking observer (note: atan2 lookup function is available inside) */
        Ifx_AngleTrkF32_step(&(driver->angleTrk), sinIn, cosIn, 0);
        Ifx_AngleTrkF32_updateStatus(&(driver->angleTrk), sinIn, cosIn);
    }
#endif
}


void IfxDsadc_Rdc_reset(IfxDsadc_Rdc *driver)
{
    Ifx_AngleTrkF32_reset(&driver->angleTrk);
}


void IfxDsadc_Rdc_resetFaults(IfxDsadc_Rdc *driver)
{
    Ifx_AngleTrkF32_resetFaults(&driver->angleTrk);
}


void IfxDsadc_Rdc_setOffset(IfxDsadc_Rdc *driver, sint32 offset)
{
    Ifx_AngleTrkF32_setOffset(&driver->angleTrk, offset);
}


void IfxDsadc_Rdc_setRefreshPeriod(IfxDsadc_Rdc *driver, float32 updatePeriod)
{
    Ifx_AngleTrkF32_setRefreshPeriod(&driver->angleTrk, updatePeriod);
}


void IfxDsadc_Rdc_startConversion(IfxDsadc_Rdc *driver)
{
    Ifx_DSADC *module = driver->hardware.inputSin.module;
    uint32     mask   =
        (1U << driver->hardware.inputCos.channelId) |
        (1U << driver->hardware.inputSin.channelId);
    IfxDsadc_startScan(module, mask, mask);
}


boolean IfxDsadc_Rdc_stdIfPosInit(IfxStdIf_Pos *stdif, IfxDsadc_Rdc *driver)
{
    /* Ensure the stdif is reset to zeros */
    memset(stdif, 0, sizeof(IfxStdIf_Pos));

    /* Set the driver */
    stdif->driver = driver;

    /* *INDENT-OFF* Note: this file was indented manually by the author. */
    /* Set the API link */
	stdif->onZeroIrq          =(IfxStdIf_Pos_OnZeroIrq               )NULL_PTR;
	stdif->getAbsolutePosition=(IfxStdIf_Pos_GetAbsolutePosition     )&IfxDsadc_Rdc_getAbsolutePosition;
	stdif->getDirection		  =(IfxStdIf_Pos_GetDirection            )&IfxDsadc_Rdc_getDirection;
	stdif->getFault           =(IfxStdIf_Pos_GetFault                )&IfxDsadc_Rdc_getFault;
	stdif->getOffset		  =(IfxStdIf_Pos_GetOffset			     )&IfxDsadc_Rdc_getOffset;
	stdif->getPeriodPerRotation  =(IfxStdIf_Pos_GetPeriodPerRotation )&IfxDsadc_Rdc_getPeriodPerRotation;
	stdif->getPosition		  =(IfxStdIf_Pos_GetPosition			 )&IfxDsadc_Rdc_getPosition;
	stdif->getRawPosition	  =(IfxStdIf_Pos_GetRawPosition          )&IfxDsadc_Rdc_getRawPosition;
	stdif->getRefreshPeriod   =(IfxStdIf_Pos_GetRefreshPeriod        )&IfxDsadc_Rdc_getRefreshPeriod;
	stdif->getResolution      =(IfxStdIf_Pos_GetResolution           )&IfxDsadc_Rdc_getResolution;
    stdif->getSensorType      =(IfxStdIf_Pos_GetSensorType           )&IfxDsadc_Rdc_getSensorType;
	stdif->reset			  =(IfxStdIf_Pos_Reset				     )&IfxDsadc_Rdc_reset;
	stdif->resetFaults		  =(IfxStdIf_Pos_ResetFaults			 )&IfxDsadc_Rdc_resetFaults;
	stdif->getSpeed           =(IfxStdIf_Pos_GetSpeed                )&IfxDsadc_Rdc_getSpeed;
	stdif->update			  =(IfxStdIf_Pos_Update				     )&IfxDsadc_Rdc_update;
	stdif->setPosition		  =(IfxStdIf_Pos_SetPosition			 )NULL_PTR;
	stdif->setRawPosition	  =(IfxStdIf_Pos_SetRawPosition			 )NULL_PTR;
	stdif->setSpeed		      =(IfxStdIf_Pos_SetSpeed			     )NULL_PTR;
	stdif->setOffset		  =(IfxStdIf_Pos_SetOffset			     )&IfxDsadc_Rdc_setOffset;
	stdif->setRefreshPeriod   =(IfxStdIf_Pos_SetRefreshPeriod        )&IfxDsadc_Rdc_setRefreshPeriod;
	stdif->getTurn            =(IfxStdIf_Pos_GetTurn                 )&IfxDsadc_Rdc_getTurn;
	stdif->onEventA            =(IfxStdIf_Pos_OnEventA               )&IfxDsadc_Rdc_onEventA;
    /* *INDENT-ON* */

    return TRUE;
}


void IfxDsadc_Rdc_update(IfxDsadc_Rdc *driver)
{
    float32 groupDelayAngle;
    float32 timeStampAngle;
    float32 angleOut;
    float32 speedEst;
    float32 angleCorrection;

    /* previous estimated speed */
    speedEst = Ifx_AngleTrkF32_getLoopSpeed(&(driver->angleTrk));

    /* angular component due to group delay */
    groupDelayAngle = driver->groupDelay * speedEst;

    /* angular component due to time-stamp, if time-stamping is enabled */
    if (driver->timestamp.enabled != FALSE)
    {
        IfxDsadc_Rdc_updateTimestamp(driver);
        timeStampAngle = IfxDsadc_Rdc_getTimestamp(driver) * speedEst;
    }
    else
    {
        timeStampAngle = 0;
    }

    /* angle correction value */
    angleCorrection = groupDelayAngle + timeStampAngle;

#if IFXDSADC_RDC_CFG_PRE_OBSERVER_CORRECTION != 0
    /*
     *  driver->sinIn = (sint16)driver->hardware.inputSin.channel->TSTMP.B.RESULT;
     *  driver->cosIn = (sint16)driver->hardware.inputCos.channel->TSTMP.B.RESULT;
     */

    {
        sint16 sinIn = driver->sinIn;
        sint16 cosIn = driver->cosIn;

        /* tracking observer (note: atan2 lookup function is available inside) */
        Ifx_AngleTrkF32_step(&(driver->angleTrk), sinIn, cosIn, angleCorrection);
        Ifx_AngleTrkF32_updateStatus(&(driver->angleTrk), sinIn, cosIn);
    }
    angleOut = driver->angleTrk.angleEst;
#else
    angleOut = driver->angleTrk.angleEst + angleCorrection;
#endif

    /* final output estimation */
    {
        Ifx_AngleTrkF32_PosIf *base        = &(driver->angleTrk).base;
        IfxStdIf_Pos_RawAngle  newPosition = (IfxStdIf_Pos_RawAngle)(angleOut * (base->resolution / 2) / IFX_PI);
        newPosition       = (newPosition + base->offset) & (base->resolution - 1);
        base->rawPosition = newPosition;
    }
}
