/*
 * glow.h
 *
 *  Created on: Jun 6, 2023
 *      Author: ahuijsen
 */

#ifndef GLOW_H_
#define GLOW_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f3xx_hal.h"

/* Number of pixels in the glow device */
#define NUM_PIXELS   (13)

uint32_t hsl_to_rgb(uint8_t h, uint8_t s, uint8_t l);
void glow_set_RGB(uint8_t index, uint32_t rgb);
void glow_render();

#ifdef __cplusplus
}
#endif

#endif /* GLOW_H_ */
