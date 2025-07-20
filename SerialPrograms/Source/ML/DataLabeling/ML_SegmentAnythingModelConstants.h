/*  ML Segment Anything Model Constants
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Constants for Segment Anything Model (SAM) to segment objects on images
 */

#ifndef PokemonAutomation_ML_SegmentAnythingModelConstants_H
#define PokemonAutomation_ML_SegmentAnythingModelConstants_H


const int SAM_EMBEDDER_INPUT_IMAGE_WIDTH = 1024;
const int SAM_EMBEDDER_INPUT_IMAGE_HEIGHT = 576;
const int SAM_EMBEDDER_OUTPUT_N_CHANNELS = 256;
const int SAM_EMBEDDER_OUTPUT_IMAGE_SIZE = 64;

const int SAM_EMBEDDER_INPUT_SIZE = SAM_EMBEDDER_INPUT_IMAGE_HEIGHT * SAM_EMBEDDER_INPUT_IMAGE_WIDTH * 3;
const int SAM_EMBEDDER_OUTPUT_SIZE = SAM_EMBEDDER_OUTPUT_N_CHANNELS * SAM_EMBEDDER_OUTPUT_IMAGE_SIZE * SAM_EMBEDDER_OUTPUT_IMAGE_SIZE;

const int SAM_N_INPUT_TENSORS = 6;
const int SAM_N_OUTPUT_TENSORS = 3;
const int SAM_LOW_RES_MASK_SIZE = 256;
const float SAM_OUTPUT_MASK_THRESHOLD = 0.0;



#endif
