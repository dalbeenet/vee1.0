#ifndef _VEE_LOCKFREE_H_
#define _VEE_LOCKFREE_H_

#include <atomic>
#include <vee/macro.h>

_VEE_BEGIN

namespace lockfree {

    template <class DataTy>
    class ring_buffer
    {
    public:
        typedef DataTy data_t;
        typedef ring_buffer<DataTy, Capacity> this_t;
        // ctor
        ring_buffer(int capacity)
        {

            std::_Atomic_thread_fence(std::memory_order_release);
    
        }
        // dtor
        ~ring_buffer()
        {
            
        }
    private:
        // defualt ctor is deleted.
        ring_buffer() = delete;
    protected:
        data_t* _container_ptr = nullptr;
        std::atomic_flag _container_flag;
        //std::atomic_flag _member_lock;
        int _rear     = 0;
        int _front    = 0;
        int _size     = 0;
        int _capacity = 0;
        //bool _resize;
    };

} // namespace lockfree

_VEE_END

#endif // _VEE_LOCKFREE_H_