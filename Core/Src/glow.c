/*
 * glow.c
 *
 *  Created on: Jun 6, 2023
 *      Author: ahuijsen
 */

#include "glow.h"
#include "main.h"
#include "tim.h"
#include <string.h>

extern DMA_HandleTypeDef hdma_tim3_ch4_up;

/* PWM value for logical 0 */
#define PWM_LO       (( 1 * (GLOW_TIMER - 1)) / 3)

/* Number of colours per pixel */
#define NUM_BPP      (3)
/* Number of bytes to store all pixel colours */
#define NUM_BYTES    (NUM_BPP * NUM_PIXELS)

/* GRB - Neopixel expects this colour order */
static uint8_t grb_arr[NUM_BYTES] = { 0 };

/* Smaller write buffer - only 2 LEDs */
#define WR_BUF_LEN   (NUM_BPP * 8 * 2)
static uint8_t wr_buf[WR_BUF_LEN] = { 0 };
static uint_fast8_t wr_buf_p = 0;

uint32_t hsl_to_rgb(uint8_t h, uint8_t s, uint8_t l)
{
   if (l == 0)
   {
      return 0;
   }

   volatile uint8_t  r, g, b, lo, c, x, m;
   volatile uint16_t h1, l1, H;
   l1 = l + 1;
   if (l < 128)    c = ((l1 << 1) * s) >> 8;
   else            c = (512 - (l1 << 1)) * s >> 8;

   H = h * 6;              // 0 to 1535 (actually 1530)
   lo = H & 255;           // Low byte  = primary/secondary color mix
   h1 = lo + 1;

   if ((H & 256) == 0)   x = h1 * c >> 8;          // even sextant, like red to yellow
   else                  x = (256 - h1) * c >> 8;  // odd sextant, like yellow to green

   m = l - (c >> 1);
   switch(H >> 8) {       // High byte = sextant of colorwheel
      case 0 : r = c; g = x; b = 0; break; // R to Y
      case 1 : r = x; g = c; b = 0; break; // Y to G
      case 2 : r = 0; g = c; b = x; break; // G to C
      case 3 : r = 0; g = x; b = c; break; // C to B
      case 4 : r = x; g = 0; b = c; break; // B to M
      default: r = c; g = 0; b = x; break; // M to R
   }

   return (((uint32_t)r + m) << 16) | (((uint32_t)g + m) << 8) | ((uint32_t)b + m);
}

static inline uint8_t scale8(uint8_t x, uint8_t scale)
{
   return ((uint16_t)x * scale) >> 8;
}

void glow_set_RGB(uint8_t index, uint32_t rgb)
{
   uint8_t r = (rgb >> 16) & 0xff;
   uint8_t g = (rgb >> 8) & 0xff;
   uint8_t b = rgb & 0xff;

   grb_arr[3 * index] = scale8(g, 0xB0);     // G
   grb_arr[3 * index + 1] = r;               // R
   grb_arr[3 * index + 2] = scale8(b, 0xF0); // B
}

void glow_render()
{
   if (wr_buf_p != 0 || hdma_tim3_ch4_up.State != HAL_DMA_STATE_READY)
   {
      return;
   }

   for (uint_fast8_t i = 0; i < 8; ++i)
   {
      for (uint_fast8_t j = 0; j < 6; ++j)
      {
         wr_buf[i + j * 8] = PWM_LO << (((grb_arr[j] << i) & 0x80) > 0);
      }
   }

   HAL_TIM_PWM_Start_DMA(&htim3, TIM_CHANNEL_4, (uint32_t *)wr_buf, WR_BUF_LEN);
   wr_buf_p = 2;
}

void HAL_TIM_PWM_PulseFinishedHalfCpltCallback(TIM_HandleTypeDef *htim)
{
   if (wr_buf_p < NUM_PIXELS)
   {
      for (uint_fast8_t i = 0; i < 8; ++i)
      {
         for (uint_fast8_t j = 0; j < 3; ++j)
         {
            wr_buf[i + j * 8] = PWM_LO << (((grb_arr[3 * wr_buf_p + j] << i) & 0x80) > 0);
         }
      }
   }
   else if (wr_buf_p < NUM_PIXELS + 2)
   {
      for (uint_fast8_t i = 0; i < WR_BUF_LEN / 2; ++i)
      {
         wr_buf[i] = 0;
      }
   }
   wr_buf_p++;
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
   if (wr_buf_p < NUM_PIXELS)
   {
      for (uint_fast8_t i = 0; i < 8; ++i)
      {
         for (uint_fast8_t j = 0; j < 3; ++j)
         {
            wr_buf[i + (j + 3) * 8] = PWM_LO << (((grb_arr[3 * wr_buf_p + j] << i) & 0x80) > 0);
         }
      }
      wr_buf_p++;
   }
   else if (wr_buf_p < NUM_PIXELS + 2)
   {
      for (uint_fast8_t i = WR_BUF_LEN / 2; i < WR_BUF_LEN; ++i)
      {
         wr_buf[i] = 0;
      }
      wr_buf_p++;
   }
   else
   {
      wr_buf_p = 0;
      HAL_TIM_PWM_Stop_DMA(&htim3, TIM_CHANNEL_4);
   }
}
