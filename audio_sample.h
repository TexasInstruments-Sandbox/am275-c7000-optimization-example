/*
 *  Copyright (C) 2023 Texas Instruments Incorporated
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

#ifndef AUDIO_SAMPLE_H_
#define AUDIO_SAMPLE_H_

#ifdef __cplusplus
extern "C"
{
#endif

/* This is a 2 khz + 6 kHz sine wave for testing a LPF digital filter */
const int16_t audio_sample_sine[] = {
    0,
    20918,
    32767,
    31306,
    19194,
    4160,
    -5623,
    -6228,
    0,
    6227,
    5622,
    -4161,
    -19195,
    -31307,
    -32768,
    -20919,
    0,
    20918,
    32767,
    31306,
    19194,
    4160,
    -5623,
    -6228,
    0,
    6227,
    5622,
    -4161,
    -19195,
    -31307,
    -32768,
    -20919,
    0,
    20918,
    32767,
    31306,
    19194,
    4160,
    -5623,
    -6228,
    0,
    6227,
    5622,
    -4161,
    -19195,
    -31307,
    -32768,
    -20919,
    0,
    20918,
    32767,
    31306,
    19194,
    4160,
    -5623,
    -6228,
    0,
    6227,
    5622,
    -4161,
    -19195,
    -31307,
    -32768,
    -20919
    };

const uint32_t audio_sample_sine_size = sizeof(audio_sample_sine)/sizeof(audio_sample_sine[0]);

const int16_t audio_sample_sine_filter_out_ref[] = {
        0,
        24,
        226,
        1031,
        3032,
        6518,
        10939,
        14861,
        16600,
        15165,
        10892,
        5311,
        302,
        -2946,
        -4525,
        -5567,
        -7322,
        -10140,
        -13009,
        -13995,
        -11369,
        -4776,
        4273,
        12906,
        18187,
        18490,
        14226,
        7508,
        984,
        -3518,
        -5742,
        -6795,
        -8129,
        -10378,
        -12776,
        -13521,
        -10897,
        -4469,
        4360,
        12813,
        18002,
        18307,
        14108,
        7476,
        1021,
        -3446,
        -5672,
        -6750,
        -8117,
        -10393,
        -12803,
        -13548,
        -10914,
        -4474,
        4365,
        12823,
        18012,
        18314,
        14110,
        7474,
        1017,
        -3450,
        -5674
    };
#ifdef __cplusplus
}
#endif

#endif
