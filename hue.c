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
const colour_coord hue_lookup[] = {
  {.x=     0,.y= 16383},
  {.x=  6269,.y= 15135},
  {.x= 11584,.y= 11584},
  {.x= 15135,.y=  6269},
  {.x= 16383,.y=     0},
  {.x= 15135,.y= -6269},
  {.x= 11584,.y=-11584},
  {.x=  6269,.y=-15135},
  {.x=     0,.y=-16383},
  {.x= -6269,.y=-15135},
  {.x=-11584,.y=-11584},
  {.x=-15135,.y= -6269},
  {.x=-16383,.y=     0},
  {.x=-15135,.y=  6269},
  {.x=-11584,.y= 11584},
  {.x= -6269,.y= 15135},
  {.x=     0,.y= 16383}
};
