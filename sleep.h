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


#ifndef __SLEEP_H__
#define __SLEEP_H__

#include "_sleep.h"

#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
	extern semaphore_t sleepDisabled;
#ifdef __cplusplus
}
#endif
#endif /* _FROM_ASM_ */


#endif
