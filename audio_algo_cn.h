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
#include <inttypes.h>
#include <stddef.h> // for NULL
#include <stdint.h>
#include <stdio.h> // for printf
#include <stdlib.h>
#include <kernel/dpl/DebugP.h>
#include "FreeRTOS.h"
#include "app_parameters.h"

typedef struct BiquadCascadeDF2T {
    float *coeff;
    float *state;
    int num_stages;
} BiquadCascadeDF2T;

/* Used to initialize any arrays */
void audio_init_cn(void);

/* Used to run the algorithm */
uint16_t audio_run_cn(int16_t samplesIn[], int16_t samplesOut[], uint16_t num_samples, unsigned long timestamps[]);

uint16_t audio_prv_qint_to_fl(int16_t samplesIn[], float samplesOut[], uint16_t num_samples);

uint16_t audio_prv_fl_to_qint(float samplesIn[], int16_t samplesOut[], uint16_t num_samples);

uint16_t audio_prv_gain(float *restrict samplesIn, float *restrict samplesOut, uint16_t num_samples, float gain);

uint16_t audio_prv_cascadeBiquad_init(BiquadCascadeDF2T *filter, int stages, float *coeffs, float *state_buf);
float audio_prv_cascadeBiquad_process_sample(BiquadCascadeDF2T *filter, float input);
uint16_t audio_prv_cascadeBiquad_run(float samplesIn[], float samplesOut[], uint16_t num_samples);
