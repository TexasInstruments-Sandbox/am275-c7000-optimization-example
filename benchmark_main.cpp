#include <stdio.h>
#include <kernel/dpl/DebugP.h>
#include "ti_drivers_config.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"
#include "audio_sample.h"
#include "app_parameters.h"
#include "audio_algo_c7.h"
#include "audio_algo_cn.h"

/* Change the benchmark parameters by editing the settings in app_parameters.h */

/*  0 = Run time of function
    1 = Sum of individual kernel times
    2 = Q15 to Float time
    3 = 3 stage cascade Biquad time
    4 = Gain time
    5 = Float to Q15 time*/
#define NUMBER_OF_TIME_PROFILES     6

uint32_t audio_sample_index;
int16_t samplesIn[APP_SAMPLES_PER_BLOCK], samplesOut[APP_SAMPLES_PER_BLOCK];
float app_samples[APP_SAMPLES_PER_BLOCK];
float app_samples2[APP_SAMPLES_PER_BLOCK];

void benchmark_init(void)
{
    audio_sample_index = 0;

    for (int16_t i = 0; i < APP_SAMPLES_PER_BLOCK; i++)
    {
        samplesIn[i] = 0;
        samplesOut[i] = 0;
    }
}

void benchmark_main(void)
{
    unsigned long start_time, stop_time;
    unsigned long num_cycles[APP_ALGO_NUM_LOOPS][NUMBER_OF_TIME_PROFILES] = {0};
    unsigned long warm_cache_avg[NUMBER_OF_TIME_PROFILES] = {0};
    /* Run any initialization */
    benchmark_init();
    #ifdef APP_CASE_CN
    audio_init_cn();
    #endif
    #ifdef APP_CASE_C7
    audio_init_c7();
    #endif
    

    /* Run the benchmark */
    for (uint16_t i = 0; i < APP_ALGO_NUM_LOOPS; i++)
    {
        for (uint32_t j = 0; j < APP_SAMPLES_PER_CHANNEL; j++ )
        {
            /* Grab samples from audio_sample.h and loop/fill channels per parameters */
            for (uint8_t chan = 0; chan < APP_NUM_OF_CHANNELS; chan++)
            {
                uint16_t sampleIndex = (j * APP_NUM_OF_CHANNELS) + chan;
                DebugP_assert(sampleIndex < APP_SAMPLES_PER_BLOCK);
                samplesIn[sampleIndex] = audio_sample_sine[audio_sample_index];
            }
            if (++audio_sample_index >= audio_sample_sine_size)
                audio_sample_index = 0;
        }
        /* Run a loop */
        start_time = __TSC;
        #ifdef APP_CASE_CN
        audio_run_cn(samplesIn, samplesOut, APP_SAMPLES_PER_BLOCK, &num_cycles[i][2]);
        #endif
        #ifdef APP_CASE_C7
        audio_run_c7(samplesIn, samplesOut, APP_SAMPLES_PER_BLOCK, &num_cycles[i][2]);
        #endif
        stop_time = __TSC;

        if (i == 0)
        {
            uint16_t maxSize = audio_sample_sine_size-1;
            if (APP_SAMPLES_PER_BLOCK < audio_sample_sine_size)
                maxSize = APP_SAMPLES_PER_BLOCK-1;
            /* On first run, validate the output of the filter with reference data */
            for (uint16_t j = 0; j < maxSize; j++)
            {
                for (uint8_t chan = 0; chan < APP_NUM_OF_CHANNELS; chan++)
                {
                    if (audio_sample_sine_filter_out_ref[j] != samplesOut[(j * APP_NUM_OF_CHANNELS) + chan])
                        printf("ERROR: Output did not match what was expected.\n\r");
                }
            }
        }
        /* Calculate the number of cycles 2 ways: sum of each kernel and the total time in the _run function */
        num_cycles[i][0] = stop_time - start_time;
        num_cycles[i][1] = num_cycles[i][2] + num_cycles[i][3] + num_cycles[i][4] + num_cycles[i][5];
        if (i >= 2)
        {
            for (uint8_t avgIndex = 0; avgIndex < NUMBER_OF_TIME_PROFILES; avgIndex++)
                warm_cache_avg[avgIndex] += num_cycles[i][avgIndex];
        }
            
    }
    for (uint8_t avgIndex = 0; avgIndex < NUMBER_OF_TIME_PROFILES; avgIndex++)
        warm_cache_avg[avgIndex] = (warm_cache_avg[avgIndex] / (APP_ALGO_NUM_LOOPS - 2));
    uint8_t vector_size = 8;
    DebugP_log((char*)"Number of samples %u - Num of Channels %u - Num of samples/chan %u\n", APP_SAMPLES_PER_BLOCK, APP_NUM_OF_CHANNELS, APP_SAMPLES_PER_CHANNEL);
    DebugP_log((char*)"Vector-Size %u - Cold-Cache-Run-1: %lu, Cold-Cache-Run-2: %lu, Warm-Cache-Run-Average (Total): %lu, Warm cache-Run-Average (Sum): %lu,  Warm Single Sample: %f\n", vector_size, num_cycles[0][0], num_cycles[1][0], warm_cache_avg[0], warm_cache_avg[1], (float)warm_cache_avg[0]/APP_SAMPLES_PER_BLOCK);
    DebugP_log((char*)"Cold Numbers - Q15 to Float: %lu, 3-Biquad: %lu, Gain: %lu, Float to Q15: %lu\n", num_cycles[0][2], num_cycles[0][3], num_cycles[0][4], num_cycles[0][5]);
    DebugP_log((char*)"Warm Run Avg - Q15 to Float: %lu, 3-Biquad: %lu, Gain: %lu, Float to Q15: %lu\n", warm_cache_avg[2], warm_cache_avg[3], warm_cache_avg[4], warm_cache_avg[5]);

    /* CSV output easy */
    DebugP_log((char*)"Samples/Channel, # of Channels, Total, Q15toFL, Biquad, Gain, FLtoQ15\n");
    DebugP_log((char*)"%u, %u, %lu, %lu, %lu, %lu, %lu\n", APP_SAMPLES_PER_CHANNEL, APP_NUM_OF_CHANNELS, warm_cache_avg[0], warm_cache_avg[2], warm_cache_avg[3], warm_cache_avg[4], warm_cache_avg[5]);


    while(1);
}
