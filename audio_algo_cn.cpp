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

#include "audio_algo_cn.h"

#define BIQUADCASCADE_NUM_STAGES        3
extern float app_samples[APP_SAMPLES_PER_BLOCK];
extern float app_samples2[APP_SAMPLES_PER_BLOCK];
BiquadCascadeDF2T cbqFilter[APP_NUM_OF_CHANNELS];

/* B0, b1, b2, a1, a2, etc */
float cbqCoeff[5*BIQUADCASCADE_NUM_STAGES] = {
    0.001525727119716,	0.001524273338300,	0,	-0.493145426031337,	0,
    1.000000000000000,	2.001540692461095,	1.001541598764263,	-1.063121235293415,	0.340034989835867,
    1.000000000000000,	1.999412152511944,	0.999413059814508,	-1.335479454569789,	0.683335011962709,
};
float cbqState[APP_NUM_OF_CHANNELS][2*BIQUADCASCADE_NUM_STAGES];

void audio_init_cn(void)
{
    for (uint32_t i = 0; i < APP_SAMPLES_PER_BLOCK; i++)
    {
        app_samples[i] = 0.00;
        app_samples2[i] = 0.00;
    }

    /* Cascade Biquad setup */
    for (uint8_t i = 0; i < APP_NUM_OF_CHANNELS; i++)
        audio_prv_cascadeBiquad_init(&cbqFilter[i], BIQUADCASCADE_NUM_STAGES, cbqCoeff, &cbqState[i][0]);
}

/* Used to run the algorithm */
uint16_t audio_run_cn(int16_t samplesIn[], int16_t samplesOut[], uint16_t num_samples, unsigned long timestamps[])
{
    uint16_t result;
    unsigned long start_time, stop_time;
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
    result = audio_prv_qint_to_fl(samplesIn, app_samples, num_samples);
    stop_time = __TSC;
    timestamps[0] = stop_time - start_time;
    DebugP_assert(result == num_samples);
    
    // 2: Run biquad filter (app samples to app samples2)
    start_time = __TSC;
    for (uint16_t chan = 0; chan < APP_NUM_OF_CHANNELS; chan++)
    {
        /* Data is interleaved */
        for (uint16_t iSample = 0; iSample < APP_SAMPLES_PER_CHANNEL; iSample++)
        {
            uint16_t dataIndex = chan + (iSample * APP_NUM_OF_CHANNELS);
            app_samples2[dataIndex] = audio_prv_cascadeBiquad_process_sample(&cbqFilter[chan], app_samples[dataIndex]);
        }
    }
    stop_time = __TSC;
    timestamps[1] = stop_time - start_time;

    // 3: Run gain (app samples 2 to app samples)
    start_time = __TSC;
    audio_prv_gain(app_samples2, app_samples, num_samples, 0.75f);
    stop_time = __TSC;
    timestamps[2] = stop_time - start_time;

    // 4: Convert to q15
    start_time = __TSC;
    result = audio_prv_fl_to_qint(app_samples, samplesOut, num_samples);
    stop_time = __TSC;
    timestamps[3] = stop_time - start_time;
    DebugP_assert(result == num_samples);
    
    return result;
}

uint16_t audio_prv_qint_to_fl(int16_t samplesIn[], float samplesOut[], uint16_t num_samples)
{
    for (uint16_t i = 0; i < num_samples; i++)
        samplesOut[i] = (float)samplesIn[i] / 32768.0f;
    return num_samples;
}

uint16_t audio_prv_fl_to_qint(float samplesIn[], int16_t samplesOut[], uint16_t num_samples)
{
    float a;
    float mulFactor = 32768.00;
    int32_t aInt = 0;

    for (int32_t i = 0; i < (int32_t) num_samples; i++) {
        a = samplesIn[i];
        float mulVal = a * mulFactor;

        int32_t intPart  = (int32_t) mulVal;
        float fracPart = mulVal - intPart;

        if ((intPart % 2 == 0) && ((fracPart == 0.5) || (fracPart == -0.5)))
            aInt = intPart;
        else if (a < 0)
            aInt = (int32_t) (mulVal - 0.5f);
        else
            aInt = (int32_t) (mulVal + 0.5f);

        /* saturate to 16-bit */
        if (aInt > 32767) {
            aInt = 32767;
        }
        if (aInt < -32768) {
            aInt = -32768;
        }

        samplesOut[i] = (int16_t) aInt;
    }
    return num_samples;
}


uint16_t audio_prv_gain(float samplesIn[], float samplesOut[restrict], uint16_t num_samples, float gain)
{
    for (int32_t i = 0; i < (int32_t) num_samples; i++)
        *(samplesOut + i) = *(samplesIn + i) * gain;
    return num_samples;
}


uint16_t audio_prv_cascadeBiquad_init(BiquadCascadeDF2T *filter, int stages, float *coeffs, float *state_buf)
{
    filter->num_stages = stages;
    filter->coeff = coeffs;
    filter->state = state_buf;
    
    // Initialize state variables to zero
    for (int i = 0; i < stages * 2; i++) {
        filter->state[i] = 0.0f;
    }
    return 0;
}

float audio_prv_cascadeBiquad_process_sample(BiquadCascadeDF2T *filter, float input)
{
    float output = input;
    
    for (int i = 0; i < filter->num_stages; i++) {
        // Coefficients for the current stage
        // Note: a0 is assumed to be 1.0, and a1, a2 typically stored with a sign flip
        float b0 = filter->coeff[i * 5 + 0];
        float b1 = filter->coeff[i * 5 + 1];
        float b2 = filter->coeff[i * 5 + 2];
        float a1 = filter->coeff[i * 5 + 3]; // Stored as positive values (used as -a1*y[n-1])
        float a2 = filter->coeff[i * 5 + 4]; // Stored as positive values (used as -a2*y[n-2])
        
        // State variables for the current stage
        float d1 = filter->state[i * 2 + 0];
        float d2 = filter->state[i * 2 + 1];
        
        // Direct Form II Transposed difference equations:
        // y[n] = b0*x[n] + d1
        // d1' = b1*x[n] - a1*y[n] + d2
        // d2' = b2*x[n] - a2*y[n]

        float y_n = b0 * output + d1;
        
        // Update state variables for the next sample
        filter->state[i * 2 + 0] = b1 * output - a1 * y_n + d2;
        filter->state[i * 2 + 1] = b2 * output - a2 * y_n;
        
        output = y_n; // Output of current stage is input to the next
    }
    
    return output;
}
