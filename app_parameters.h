/* Test Cases to Run */
#define APP_CASE_CN
//#define APP_CASE_C7

/* App Parameters */
#define APP_NUM_OF_CHANNELS                 (8U)
#define APP_SAMPLES_PER_CHANNEL             (128U)
#define APP_BLOCKSIZE_BYTES                 (APP_SAMPLES_PER_CHANNEL * APP_BYTES_PER_SAMPLE)
#define APP_SAMPLES_PER_BLOCK               (APP_SAMPLES_PER_CHANNEL * APP_NUM_OF_CHANNELS)
#define APP_SAMPLE_RATE                     (48000U)
#define APP_ALGO_NUM_LOOPS                  (10U)
#define APP_BYTES_PER_SAMPLE                (2U)    /* This should not be changed, we're using uint16_t sample format */
