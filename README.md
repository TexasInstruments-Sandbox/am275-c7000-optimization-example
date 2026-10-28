# C7000 Optimization Starting Guide Example

This project demonstrates audio algorithm optimization techniques for the TI AM275 processor's C7x core. It is intended as example code to accompany the **C7000 Optimization Starting Guide** application note ([SDAA248](https://www.ti.com/lit/sdaa248)).

## Overview

This example contains two variants of a C-based audio algorithm:

1. **Baseline Implementation** - Standard C code without C7x-specific optimizations
2. **Optimized Implementation** - Algorithm utilizing optimized kernels from the FreeRTOS SDK for AM275

By comparing these two implementations, you can learn optimization techniques and measure the performance improvements achieved through C7x-specific optimization strategies.

## Project Contents

- `app_parameters.h` - Configuration parameters for the demo algorithm
- Baseline and optimized algorithm implementations
- Example code demonstrating how to use the audio processing kernels

## Installation

### Prerequisites

- Code Composer Studio 20 or later
- TI AM275 SDK or compatible development environment

### Steps

1. Open Code Composer Studio 20+
2. Select **File → Import Projects**
3. Browse to and select this project directory
4. Select the `audio_algorithm_am275x-evm_c75ss0-0_freertos_ti-c7000` project
5. Click **Finish** to import the project

## Configuration

Use `app_parameters.h` to configure the example:

| Parameter | Description |
|-----------|-------------|
| `APP_CASE_CN` | Enable/disable baseline (non-optimized) implementation |
| `APP_CASE_C7` | Enable/disable C7x optimized implementation |
| `APP_NUM_OF_CHANNELS` | Number of audio channels (default: 8) |
| `APP_SAMPLES_PER_CHANNEL` | Samples per channel per block (default: 128) |
| `APP_SAMPLE_RATE` | Audio sample rate in Hz (default: 48000) |
| `APP_ALGO_NUM_LOOPS` | Number of algorithm iterations (default: 10) |
| `APP_BYTES_PER_SAMPLE` | Bytes per sample - fixed at 2 for uint16_t format |

## Usage

1. Configure desired test cases in `app_parameters.h`:
   - Uncomment `#define APP_CASE_CN` to run baseline version, making sure to comment `#define APP_CASE_C7`
   - Uncomment `#define APP_CASE_C7` to run optimized version, making sure to comment `#define APP_CASE_CN`

2. Build and run the project in Code Composer Studio

3. Observe performance metrics and compare results between implementations

## References

- **C7000 Optimization Starting Guide (SDAA248)**: https://www.ti.com/lit/sdaa248
- AM275 FreeRTOS SDK Documentation
- TI AM275 Technical Reference Manual

## Support

For questions or issues related to this example code, refer to the associated application note or TI's technical support resources.
