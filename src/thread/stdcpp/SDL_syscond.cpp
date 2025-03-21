/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2024 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "SDL_internal.h"

extern "C" {
}

#include <chrono>
#include <condition_variable>
#include <ratio>
#include <system_error>

#include "SDL_sysmutex_c.h"

struct SDL_Condition
{
    std::condition_variable_any cpp_cond;
};

// Create a condition variable
extern "C"
SDL_Condition *SDL_CreateCondition(void)
{
    // Allocate and initialize the condition variable
    try {
        SDL_Condition *cond = new SDL_Condition;
        return cond;
    } catch (std::system_error &ex) {
        SDL_SetError("unable to create a C++ condition variable: code=%d; %s", ex.code(), ex.what());
        return NULL;
    } catch (std::bad_alloc &) {
        SDL_OutOfMemory();
        return NULL;
    }
}

// Destroy a condition variable
extern "C"
void SDL_DestroyCondition(SDL_Condition *cond)
{
    if (cond) {
        delete cond;
    }
}

// Restart one of the threads that are waiting on the condition variable
extern "C"
void SDL_SignalCondition(SDL_Condition *cond)
{
    if (!cond) {
        return;
    }

    cond->cpp_cond.notify_one();
}

// Restart all threads that are waiting on the condition variable
extern "C"
void SDL_BroadcastCondition(SDL_Condition *cond)
{
    if (!cond) {
        return;
    }

    cond->cpp_cond.notify_all();
}

extern "C"
bool SDL_WaitConditionTimeoutNS(SDL_Condition *cond, SDL_Mutex *mutex, Sint64 timeoutNS)
{
    if (!cond || !mutex) {
        return true;
    }

    try {
        std::unique_lock<std::recursive_mutex> cpp_lock(mutex->cpp_mutex, std::adopt_lock_t());
        if (timeoutNS < 0) {
            cond->cpp_cond.wait(cpp_lock);
            cpp_lock.release();
            return true;
        } else {
            auto wait_result = cond->cpp_cond.wait_for(
                cpp_lock,
                std::chrono::duration<Sint64, std::nano>(timeoutNS));
            cpp_lock.release();
            if (wait_result == std::cv_status::timeout) {
                return false;
            } else {
                return true;
            }
        }
    } catch (std::system_error &) {
        return false;
    }
}
