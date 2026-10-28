/*
 *  Copyright (C) 2018-2021 Texas Instruments Incorporated
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "audio_algo_c7.h"

#define BIQUADCASCADE_NUM_STAGES        3
extern float app_samples[APP_SAMPLES_PER_BLOCK];
extern float app_samples2[APP_SAMPLES_PER_BLOCK];
DSPLIB_bufParams1D_t floatBufParams, q15BufParams;
/* q15 to float params */
DSPLIB_kernelHandle Q15FloatHandle;
DSPLIB_kernelHandle floatQ15Handle;
/* Biquad filter params */
DSPLIB_kernelHandle cbqHandle;

/* Gain/Mul Const Values */
DSPLIB_kernelHandle         mulConstHandle;

/* B0, b1, b2, a1, a2, etc */
float cbqCoeffC7[5*BIQUADCASCADE_NUM_STAGES] = {
    0.001525727119716,	0.001524273338300,	0,	                -0.493145426031337,	0,
    1.000000000000000,	2.001540692461095,	1.001541598764263,	-1.063121235293415,	0.340034989835867,
    1.000000000000000,	1.999412152511944,	0.999413059814508,	-1.335479454569789,	0.683335011962709,
};
float cbqStateC7[2*BIQUADCASCADE_NUM_STAGES*APP_NUM_OF_CHANNELS];

void audio_init_c7(void)
{
    for (uint32_t i = 0; i < APP_SAMPLES_PER_BLOCK; i++)
    {
        app_samples[i] = 0.00;
        app_samples2[i] = 0.00;
    }

    
    q15BufParams.data_type      = DSPLIB_INT16;
    q15BufParams.dim_x          = APP_SAMPLES_PER_BLOCK;
    floatBufParams.data_type    = DSPLIB_FLOAT32;
    floatBufParams.dim_x        = APP_SAMPLES_PER_BLOCK;

    /* Q15 to float conversion setup */
    DSPLIB_q15tofl_InitArgs q15FloatKerInitArgs;
    q15FloatKerInitArgs.dataSize  = APP_SAMPLES_PER_BLOCK;
    q15FloatKerInitArgs.funcStyle = DSPLIB_FUNCTION_OPTIMIZED;

    int32_t handleSize = DSPLIB_q15tofl_getHandleSize(&q15FloatKerInitArgs);
    Q15FloatHandle     = malloc(handleSize);

    int32_t status = DSPLIB_q15tofl_init_checkParams(Q15FloatHandle, &q15BufParams, &floatBufParams, &q15FloatKerInitArgs);
    if (status == DSPLIB_SUCCESS)
        status = DSPLIB_q15tofl_init(Q15FloatHandle, &q15BufParams, &floatBufParams, &q15FloatKerInitArgs);

    /* Cascade Biquad setup */
    audio_prv_cascadeBiquad_init_c7();


    /* Gain/Mul Constant setup */
    DSPLIB_mulConstant_InitArgs mulConstKerInitArgs;
    handleSize = DSPLIB_mulConstant_getHandleSize(&mulConstKerInitArgs);
    mulConstHandle     = malloc(handleSize);
    mulConstKerInitArgs.dataSize  = APP_SAMPLES_PER_BLOCK;
    mulConstKerInitArgs.funcStyle = DSPLIB_FUNCTION_OPTIMIZED;

    status = DSPLIB_SUCCESS;

    /* Initialization */
    if (status == DSPLIB_SUCCESS)
        status = DSPLIB_mulConstant_init(mulConstHandle, &floatBufParams, &floatBufParams, &mulConstKerInitArgs);
    
    /* Float to Q15 conversion setup */
    DSPLIB_fltoq15_InitArgs floatQ15KerInitArgs;
    floatQ15KerInitArgs.dataSize  = APP_SAMPLES_PER_BLOCK;
    floatQ15KerInitArgs.funcStyle = DSPLIB_FUNCTION_OPTIMIZED;

    handleSize = DSPLIB_fltoq15_getHandleSize(&floatQ15KerInitArgs);
    floatQ15Handle     = malloc(handleSize);

    status = DSPLIB_fltoq15_init_checkParams(floatQ15Handle, &floatBufParams, &q15BufParams, &floatQ15KerInitArgs);
    if (status == DSPLIB_SUCCESS)
        status = DSPLIB_fltoq15_init(floatQ15Handle, &floatBufParams, &q15BufParams, &floatQ15KerInitArgs);

}
void audio_prv_cascadeBiquad_init_c7()
{
    DSPLIB_bufParams2D_t bufParamsIn, bufParamsOut, bufParamsFilterVar, bufParamsFilterCoeff;
    DSPLIB_cascadeBiquad_InitArgs kerInitArgs;
    uint32_t handleSize = DSPLIB_cascadeBiquad_getHandleSize(&kerInitArgs);
    cbqHandle = malloc(handleSize);
    
    /* 64 samples x 2 channels = 128 */
    int32_t dataSize            = APP_SAMPLES_PER_CHANNEL;       // Number of samples per channel
    int32_t dataPitch           = 4 * APP_NUM_OF_CHANNELS;     // sizeof(data_type) * numChannels
    int32_t filterVarPitch      = 4 * APP_NUM_OF_CHANNELS;     // sizeof(data_type) * numChannels
    int32_t numChannels         = APP_NUM_OF_CHANNELS;
    int32_t numStages           = BIQUADCASCADE_NUM_STAGES;
    uint32_t enableNChCoeffs    = 0;                           // Use same coefficients across channels
    uint32_t biquadForm         = DSPLIB_BIQUAD_DF2;

    /* Cascade Biquad Arguments */
    kerInitArgs.dataSize        = dataSize;
    kerInitArgs.numChannels     = numChannels;
    kerInitArgs.numStages       = numStages;
    kerInitArgs.enableNChCoeffs = enableNChCoeffs;
    kerInitArgs.biquadForm      = biquadForm;
    kerInitArgs.funcStyle       = DSPLIB_FUNCTION_OPTIMIZED;

    /* Input Buffer Parameters */
    bufParamsIn.data_type       = DSPLIB_FLOAT32;
    bufParamsIn.dim_x           = numChannels;
    bufParamsIn.dim_y           = dataSize;
    bufParamsIn.stride_y        = dataPitch;

    /* Output Buffer Parameters */
    bufParamsOut.data_type      = DSPLIB_FLOAT32;
    bufParamsOut.dim_x          = numChannels;
    bufParamsOut.dim_y          = dataSize;
    bufParamsOut.stride_y       = dataPitch;

    /* Filter Variable Parameters */
    bufParamsFilterVar.data_type    = DSPLIB_FLOAT32;
    bufParamsFilterVar.dim_x        = numChannels;
    bufParamsFilterVar.dim_y        = numStages * DSPLIB_NUM_STATE_VARS_DF2;
    bufParamsFilterVar.stride_y     = filterVarPitch;

    /* Filter Coefficient Parameters */
    bufParamsFilterCoeff.data_type  = DSPLIB_FLOAT32;
    bufParamsFilterCoeff.dim_x      = numStages * DSPLIB_NUM_BIQUAD_COEFS;
    bufParamsFilterCoeff.dim_y      = (enableNChCoeffs == 0) ? 1 : numChannels;
    bufParamsFilterCoeff.stride_y   = bufParamsFilterCoeff.dim_x * DSPLIB_sizeof(bufParamsFilterCoeff.data_type);

    DebugP_assert(DSPLIB_cascadeBiquad_init_checkParams(cbqHandle, &bufParamsIn, &bufParamsFilterCoeff,
                                &bufParamsFilterVar, &bufParamsOut, &kerInitArgs) == DSPLIB_SUCCESS);
    // init
    DebugP_assert(DSPLIB_cascadeBiquad_init(cbqHandle, &bufParamsIn, &bufParamsFilterCoeff,
                                &bufParamsFilterVar, &bufParamsOut, &kerInitArgs) == DSPLIB_SUCCESS);
}


/* Used to run the algorithm */
uint16_t audio_run_c7(int16_t samplesIn[], int16_t samplesOut[], uint16_t num_samples, unsigned long timestamps[])
{
    unsigned long start_time, stop_time;
    int32_t status;
    /* The main audio algorithm:
    *    1) Convert Q15 to float
    *    2) BiQuad band pass filter(s)
    *    3) Gain
    *    4) Convert float to Q15
    */

    /* Quick check for overflow */
    if (num_samples > APP_SAMPLES_PER_BLOCK)
        return 0;

    // 1: Convert to float
    start_time = __TSC;
    status = DSPLIB_q15tofl_exec(Q15FloatHandle, samplesIn, app_samples);
    stop_time = __TSC;
    timestamps[0] = stop_time - start_time;
    DebugP_assert(status  == DSPLIB_SUCCESS);

    // 2: Run biquad filter (app samples to app samples2)
    start_time = __TSC;
    status = DSPLIB_cascadeBiquad_exec(cbqHandle, app_samples, cbqCoeffC7, cbqStateC7, app_samples2);
    stop_time = __TSC;
    timestamps[1] = stop_time - start_time;
    DebugP_assert(status  == DSPLIB_SUCCESS);

    // 3: Run gain (app samples 2 to app samples)
    float mulConst[] = { 0.75f };
    start_time = __TSC;
    status = DSPLIB_mulConstant_exec(mulConstHandle, mulConst, app_samples2, app_samples);
    stop_time = __TSC;
    timestamps[2] = stop_time - start_time;
    DebugP_assert(status  == DSPLIB_SUCCESS);

    // 4: Convert to q15
    start_time = __TSC;
    status = DSPLIB_fltoq15_exec(floatQ15Handle, app_samples, samplesOut);
    stop_time = __TSC;
    timestamps[3] = stop_time - start_time;
    DebugP_assert(status  == DSPLIB_SUCCESS);
    return 0;
}
