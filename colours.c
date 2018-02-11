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
const colour coord_lookup[] = {
  {.white=0x00, .red=0xFF, .green=0xBF, .blue=0x00 }, /* 0, 0 */
  {.white=0x00, .red=0xFF, .green=0x9C, .blue=0x00 }, /* 1, 0 */
  {.white=0x00, .red=0xFF, .green=0x70, .blue=0x00 }, /* 2, 0 */
  {.white=0x00, .red=0xFF, .green=0x3B, .blue=0x00 }, /* 3, 0 */
  {.white=0x00, .red=0xFF, .green=0x00, .blue=0x00 }, /* 4, 0 */
  {.white=0x00, .red=0xFF, .green=0x00, .blue=0x3B }, /* 5, 0 */
  {.white=0x00, .red=0xFF, .green=0x00, .blue=0x70 }, /* 6, 0 */
  {.white=0x00, .red=0xFF, .green=0x00, .blue=0x9C }, /* 7, 0 */
  {.white=0x00, .red=0xFF, .green=0x00, .blue=0xBF }, /* 8, 0 */

  {.white=0x00, .red=0xFF, .green=0xE1, .blue=0x00 }, /* 0, 1 */
  {.white=0x00, .red=0xFF, .green=0xBF, .blue=0x00 }, /* 1, 1 */
  {.white=0x04, .red=0xFF, .green=0x93, .blue=0x09 }, /* 2, 1 */
  {.white=0x14, .red=0xFF, .green=0x6A, .blue=0x27 }, /* 3, 1 */
  {.white=0x1A, .red=0xFF, .green=0x32, .blue=0x33 }, /* 4, 1 */
  {.white=0x14, .red=0xFF, .green=0x27, .blue=0x6A }, /* 5, 1 */
  {.white=0x04, .red=0xFF, .green=0x09, .blue=0x93 }, /* 6, 1 */
  {.white=0x00, .red=0xFF, .green=0x00, .blue=0xBF }, /* 7, 1 */
  {.white=0x00, .red=0xFF, .green=0x00, .blue=0xE1 }, /* 8, 1 */

  {.white=0x00, .red=0xF0, .green=0xFF, .blue=0x00 }, /* 0, 2 */
  {.white=0x04, .red=0xFF, .green=0xEF, .blue=0x09 }, /* 1, 2 */
  {.white=0x21, .red=0xFF, .green=0xCE, .blue=0x3E }, /* 2, 2 */
  {.white=0x3A, .red=0xFF, .green=0xAA, .blue=0x66 }, /* 3, 2 */
  {.white=0x44, .red=0xFF, .green=0x77, .blue=0x77 }, /* 4, 2 */
  {.white=0x3A, .red=0xFF, .green=0x66, .blue=0xAA }, /* 5, 2 */
  {.white=0x21, .red=0xFF, .green=0x3E, .blue=0xCE }, /* 6, 2 */
  {.white=0x04, .red=0xFF, .green=0x09, .blue=0xEF }, /* 7, 2 */
  {.white=0x00, .red=0xF0, .green=0x00, .blue=0xFF }, /* 8, 2 */

  {.white=0x00, .red=0xBB, .green=0xFF, .blue=0x00 }, /* 0, 3 */
  {.white=0x14, .red=0xD5, .green=0xFF, .blue=0x27 }, /* 1, 3 */
  {.white=0x3A, .red=0xF6, .green=0xFF, .blue=0x66 }, /* 2, 3 */
  {.white=0x62, .red=0xFF, .green=0xE6, .blue=0x9E }, /* 3, 3 */
  {.white=0x7B, .red=0xFF, .green=0xBB, .blue=0xBB }, /* 4, 3 */
  {.white=0x62, .red=0xFF, .green=0x9E, .blue=0xE6 }, /* 5, 3 */
  {.white=0x3A, .red=0xF6, .green=0x66, .blue=0xFF }, /* 6, 3 */
  {.white=0x14, .red=0xD5, .green=0x27, .blue=0xFF }, /* 7, 3 */
  {.white=0x00, .red=0xBB, .green=0x00, .blue=0xFF }, /* 8, 3 */

  {.white=0x00, .red=0x7F, .green=0xFF, .blue=0x00 }, /* 0, 4 */
  {.white=0x1A, .red=0x98, .green=0xFF, .blue=0x32 }, /* 1, 4 */
  {.white=0x44, .red=0xBA, .green=0xFF, .blue=0x77 }, /* 2, 4 */
  {.white=0x7B, .red=0xDC, .green=0xFF, .blue=0xBB }, /* 3, 4 */
  {.white=0xFF, .red=0xFF, .green=0xFF, .blue=0xFF }, /* 4, 4 */
  {.white=0x7B, .red=0xDC, .green=0xBA, .blue=0xFF }, /* 5, 4 */
  {.white=0x44, .red=0xBA, .green=0x77, .blue=0xFF }, /* 6, 4 */
  {.white=0x1A, .red=0x98, .green=0x32, .blue=0xFF }, /* 7, 4 */
  {.white=0x00, .red=0x7F, .green=0x00, .blue=0xFF }, /* 8, 4 */

  {.white=0x00, .red=0x43, .green=0xFF, .blue=0x00 }, /* 0, 5 */
  {.white=0x14, .red=0x51, .green=0xFF, .blue=0x27 }, /* 1, 5 */
  {.white=0x3A, .red=0x6F, .green=0xFF, .blue=0x66 }, /* 2, 5 */
  {.white=0x62, .red=0x9E, .green=0xFF, .blue=0xB6 }, /* 3, 5 */
  {.white=0x7B, .red=0xBA, .green=0xFF, .blue=0xFF }, /* 4, 5 */
  {.white=0x62, .red=0x9E, .green=0xB6, .blue=0xFF }, /* 5, 5 */
  {.white=0x3A, .red=0x6F, .green=0x66, .blue=0xFF }, /* 6, 5 */
  {.white=0x14, .red=0x51, .green=0x27, .blue=0xFF }, /* 7, 5 */
  {.white=0x00, .red=0x43, .green=0x00, .blue=0xFF }, /* 8, 5 */

  {.white=0x00, .red=0x0E, .green=0xFF, .blue=0x00 }, /* 0, 6 */
  {.white=0x04, .red=0x09, .green=0xFF, .blue=0x18 }, /* 1, 6 */
  {.white=0x21, .red=0x3E, .green=0xFF, .blue=0x6E }, /* 2, 6 */
  {.white=0x3A, .red=0x66, .green=0xFF, .blue=0xBB }, /* 3, 6 */
  {.white=0x44, .red=0x77, .green=0xFF, .blue=0xFF }, /* 4, 6 */
  {.white=0x3A, .red=0x66, .green=0xBB, .blue=0xFF }, /* 5, 6 */
  {.white=0x21, .red=0x3E, .green=0x6E, .blue=0xFF }, /* 6, 6 */
  {.white=0x04, .red=0x09, .green=0x18, .blue=0xFF }, /* 7, 6 */
  {.white=0x00, .red=0x0E, .green=0x00, .blue=0xFF }, /* 8, 6 */

  {.white=0x00, .red=0x00, .green=0xFF, .blue=0x1D }, /* 0, 7 */
  {.white=0x00, .red=0x00, .green=0xFF, .blue=0x3F }, /* 1, 7 */
  {.white=0x04, .red=0x09, .green=0xFF, .blue=0x75 }, /* 2, 7 */
  {.white=0x14, .red=0x27, .green=0xFF, .blue=0xBC }, /* 3, 7 */
  {.white=0x1A, .red=0x32, .green=0xFF, .blue=0xFF }, /* 4, 7 */
  {.white=0x14, .red=0x27, .green=0xBC, .blue=0xFF }, /* 5, 7 */
  {.white=0x04, .red=0x09, .green=0x75, .blue=0xFF }, /* 6, 7 */
  {.white=0x00, .red=0x00, .green=0x3F, .blue=0xFF }, /* 7, 7 */
  {.white=0x00, .red=0x00, .green=0x1D, .blue=0xFF }, /* 8, 7 */

  {.white=0x00, .red=0x00, .green=0xFF, .blue=0x3F }, /* 0, 8 */
  {.white=0x00, .red=0x00, .green=0xFF, .blue=0x62 }, /* 1, 8 */
  {.white=0x00, .red=0x00, .green=0xFF, .blue=0x8E }, /* 2, 8 */
  {.white=0x00, .red=0x00, .green=0xFF, .blue=0xC3 }, /* 3, 8 */
  {.white=0x00, .red=0x00, .green=0xFF, .blue=0xFF }, /* 4, 8 */
  {.white=0x00, .red=0x00, .green=0xC3, .blue=0xFF }, /* 5, 8 */
  {.white=0x00, .red=0x00, .green=0x8E, .blue=0xFF }, /* 6, 8 */
  {.white=0x00, .red=0x00, .green=0x62, .blue=0xFF }, /* 7, 8 */
  {.white=0x00, .red=0x00, .green=0x3F, .blue=0xFF }  /* 8, 8 */
};