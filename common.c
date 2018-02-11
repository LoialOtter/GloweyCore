/*
    Glowey Core - Copyright (C) 2017..2018 Matthew Peters

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "common.h"
#include "ch.h"

#define RAND_A 
#define RAND_B

static uint32_t rand_state;

uint32_t rand(uint16_t a, uint16_t b) {
  rand_state = ((rand_state * 1103515245) + 12345) & 0x7FFFFFFF;
  rand_state ^= ((uint32_t) a << 16) | b;
  return rand_state;
}


uint16_t int_sqrt32(uint32_t x) {
  uint16_t res=0;
  uint16_t add= 0x8000;
  uint16_t temp;
  uint32_t g2;
  for(add = 0x8000; add > 0; add >>= 1) {
    temp = res | add;
    g2   = temp*temp;
    if (x >= g2) {
      res = temp;           
    }
  }
  return res;
}

uint32_t SquareRoot(uint32_t a_nInput) {
  uint32_t op  = a_nInput;
  uint32_t res = 0;
  uint32_t one = 1uL << 30; // The second-to-top bit is set: use 1u << 14 for uint16_t type; use 1uL<<30 for uint32_t type
  
  // "one" starts at the highest power of four <= than the argument.
  for (one = 1<<30; one > op; one >>= 2);
  
  for (res = 0; one > 0; one >>= 2) {
    if (op >= res + one) {
      op = op - (res + one);
      res = res + 2*one;
      res>>=1;
    }
  }
  return res;
}


void hs_to_xy(u8 hue, u8 saturation, s16* out_x, s16* out_y) {
  s32 x, y;
  u8 h_coarse, h_fine;
  h_coarse = (hue >> 4) & 0x0F;
  h_fine   = (hue << 4);
  
  x  = hue_lookup[h_coarse  ].x * (255 - h_fine);
  x += hue_lookup[h_coarse+1].x * (h_fine);
  x >>= 8;
  
  y  = hue_lookup[h_coarse  ].y * (255 - h_fine);
  y += hue_lookup[h_coarse+1].y * (h_fine);
  y >>= 8;

  x = (x * saturation) >> 8;
  y = (y * saturation) >> 8;
  
  *out_x = x;
  *out_y = y;
}


void xy_to_rgb(s16 x, s16 y, colour* out) {
  u8 x_coarse, x_fine, nx_fine;
  u8 y_coarse, y_fine, ny_fine;
  u16 scale_a, scale_b, scale_c, scale_d;
  s32 red, green, blue, white;
  u8 offset;
  x = x + 16383;
  //if (x > 32767) x = 32767;
  if (x < 0) x = 0;
  y = y + 16383;
  //if (y > 32767) y = 32767;
  if (y < 0) y = 0;
  x_coarse = x >> 12;
  x_fine   = x >> 4;
  y_coarse = y >> 12;
  y_fine   = y >> 4;
  nx_fine = (0xFF-x_fine);
  ny_fine = (0xFF-y_fine);
  scale_a = nx_fine * ny_fine;
  scale_b =  x_fine * ny_fine;
  scale_c = nx_fine *  y_fine;
  scale_d =  x_fine *  y_fine;
  
  offset = x_coarse + y_coarse * 9;
  white  = coord_lookup[offset +  0].white * scale_a;
  white += coord_lookup[offset +  1].white * scale_b;
  white += coord_lookup[offset +  9].white * scale_c;
  white += coord_lookup[offset + 10].white * scale_d;
  white >>= 16;

  red    = coord_lookup[offset +  0].red   * scale_a;
  red   += coord_lookup[offset +  1].red   * scale_b;
  red   += coord_lookup[offset +  9].red   * scale_c;
  red   += coord_lookup[offset + 10].red   * scale_d;
  red   >>= 16;
  
  green  = coord_lookup[offset +  0].green * scale_a;
  green += coord_lookup[offset +  1].green * scale_b;
  green += coord_lookup[offset +  9].green * scale_c;
  green += coord_lookup[offset + 10].green * scale_d;
  green >>= 16;
  
  blue   = coord_lookup[offset +  0].blue  * scale_a;
  blue  += coord_lookup[offset +  1].blue  * scale_b;
  blue  += coord_lookup[offset +  9].blue  * scale_c;
  blue  += coord_lookup[offset + 10].blue  * scale_d;
  blue  >>= 16;

  out->white = white;
  out->red   = red;
  out->green = green;
  out->blue  = blue;
}

void scale_rgb(s16 scale, colour* out) {
  out->white = ((s32)out->white * (s32)scale) >> 12;
  out->red   = ((s32)out->red   * (s32)scale) >> 12;
  out->green = ((s32)out->green * (s32)scale) >> 12;
  out->blue  = ((s32)out->blue  * (s32)scale) >> 12;
}


void Pause(int number) {
  for(; number > 0; number--) {
	    __NOP();
	    __NOP();
	    __NOP();
	    __NOP();
  };
}






