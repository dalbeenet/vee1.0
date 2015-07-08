/******************************************************************* 
FileName: thread_model.h
Author  : vee
Date    : 2015-07-01 00:48:17
Desc    : thread model class
*******************************************************************/ 

#ifndef _VEE_LOCK_H_
#define _VEE_LOCK_H_

#include <mutex>
#include <vee/macro.h>

_VEE_BEGIN

struct empty_mutex
{
    __forceinline void lock() VEE_EMPTY_FUNCTION;
    __forceinline bool try_lock()
    {
        return true;
    }
    __forceinline void unlock() VEE_EMPTY_FUNCTION;
    __forceinline std::mutex::native_handle_type native_handle()
    {
        return NULL;
    }
};

_VEE_END

#endif // _VEE_LOCK_H_