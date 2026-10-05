/****************************************************************************
 * MarcViiewLibGuiTest - legacy libogc compatibility
 * Minimal adaptation of libgui's OgcThreadDriver for older libogc.
 *
 * Older libogc exposes a much smaller LWP priority API. All libgui priority
 * classes therefore map to the available LWP_PRIO_HIGHEST value for this
 * compatibility test. Threading itself still uses the normal LWP primitives.
 ***************************************************************************/
#include <ogcsys.h>
#include <unistd.h>
#include <ogc/cond.h>

#include "drivers/ogc/OgcThreadDriver.h"

namespace
{
    struct OgcThreadHandle
    {
        lwp_t thread = LWP_THREAD_NULL;
        ThreadEntry entry = nullptr;
        void * arg = nullptr;
    };

    void * OgcThreadTrampoline(void * arg)
    {
        OgcThreadHandle * handle = static_cast<OgcThreadHandle *>(arg);
        return handle->entry(handle->arg);
    }

    int MapOgcPriority(ThreadPriority)
    {
        return LWP_PRIO_HIGHEST;
    }
}

void OgcThreadDriver::init()
{
}

void OgcThreadDriver::shutdown()
{
}

bool OgcThreadDriver::createThread(ThreadEntry entry, void * arg, uint32_t stackSize, ThreadPriority priority, void ** outHandle)
{
    OgcThreadHandle * handle = new OgcThreadHandle();
    handle->entry = entry;
    handle->arg = arg;
    *outHandle = handle;

    int32_t res = LWP_CreateThread(
        &handle->thread,
        OgcThreadTrampoline,
        handle,
        nullptr,
        stackSize,
        MapOgcPriority(priority));

    if(res != 0)
    {
        *outHandle = nullptr;
        delete handle;
        return false;
    }

    return true;
}

void OgcThreadDriver::joinThread(void * thread)
{
    if(!thread)
        return;

    OgcThreadHandle * handle = static_cast<OgcThreadHandle *>(thread);
    LWP_JoinThread(handle->thread, nullptr);
    delete handle;
}

void OgcThreadDriver::cancelThread(void * thread)
{
    if(!thread)
        return;

    OgcThreadHandle * handle = static_cast<OgcThreadHandle *>(thread);
    LWP_SuspendThread(handle->thread);
}

void OgcThreadDriver::suspendThread(void * thread)
{
    if(thread)
        LWP_SuspendThread(static_cast<OgcThreadHandle *>(thread)->thread);
}

void OgcThreadDriver::resumeThread(void * thread)
{
    if(thread)
        LWP_ResumeThread(static_cast<OgcThreadHandle *>(thread)->thread);
}

bool OgcThreadDriver::isThreadSuspended(void * thread)
{
    if(!thread)
        return false;

    return LWP_ThreadIsSuspended(static_cast<OgcThreadHandle *>(thread)->thread) != 0;
}

void * OgcThreadDriver::createMutex()
{
    mutex_t * mutex = new mutex_t;
    if(LWP_MutexInit(mutex, false) != 0)
    {
        delete mutex;
        return nullptr;
    }

    return mutex;
}

void OgcThreadDriver::destroyMutex(void * mutex)
{
    if(!mutex)
        return;

    mutex_t * m = static_cast<mutex_t *>(mutex);
    LWP_MutexDestroy(*m);
    delete m;
}

void OgcThreadDriver::lockMutex(void * mutex)
{
    if(mutex)
        LWP_MutexLock(*static_cast<mutex_t *>(mutex));
}

void OgcThreadDriver::unlockMutex(void * mutex)
{
    if(mutex)
        LWP_MutexUnlock(*static_cast<mutex_t *>(mutex));
}

void * OgcThreadDriver::createCond()
{
    cond_t * cond = new cond_t;
    if(LWP_CondInit(cond) != 0)
    {
        delete cond;
        return nullptr;
    }

    return cond;
}

void OgcThreadDriver::destroyCond(void * cond)
{
    if(!cond)
        return;

    cond_t * c = static_cast<cond_t *>(cond);
    LWP_CondDestroy(*c);
    delete c;
}

void OgcThreadDriver::waitCond(void * cond, void * mutex)
{
    if(cond && mutex)
        LWP_CondWait(*static_cast<cond_t *>(cond), *static_cast<mutex_t *>(mutex));
}

void OgcThreadDriver::signalCond(void * cond)
{
    if(cond)
        LWP_CondBroadcast(*static_cast<cond_t *>(cond));
}

void OgcThreadDriver::sleepMilliseconds(uint32_t ms)
{
    usleep(ms * 1000);
}

uintptr_t OgcThreadDriver::getCurrentThreadId()
{
    return (uintptr_t)LWP_GetSelf();
}
