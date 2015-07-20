#ifndef _VEE_SCHEDULER_H_
#define _VEE_SCHEDULER_H_

#include <mutex>
#include <vee/actor.h>

_VEE_BEGIN

template <class FTy>
class actor_group
{
    typedef std::shared_ptr< actor<FTy> > actor_ptr_type;
public:
    actor_group(unsigned int count, unsigned int mailbox_size)
    {
        for (int i = 0; i < count; ++i)
        {
            add_actor(mailbox_size);
        }
    }
    std::size_t add_actor(std::size_t mailbox_size)
    {
        std::lock_guard<std::mutex> guard(_container_lock);
        _actor_pointers.push_back(std::make_shared(mailbox_size));
        return _actor_pointers.size();
    }
    std::size_t remove_actor()
    {
        std::lock_guard<std::mutex> guard(_container_lock);
        _actor_pointers.pop_back();
        return _actor_pointers.size();
    }
    inline actor<FTy>& at(unsigned int idx)
    {
        if (idx >= _actor_pointers.size())
        {
            throw ::std::runtime_error("actor_group::at - out of bounds!");
        }
        actor<FTy>& ref = *_actor_pointers[idx];
        return ref;
    }
    inline actor<FTy>& operator[](unsigned int idx)
    {
        actor<FTy>& ref = *_actor_pointers[idx];
        return ref;
    }
protected:
    std::vector<actor_ptr_type> _actor_pointers;
    std::mutex  _container_lock;
};

_VEE_END

#endif // _VEE_SCHEDULER_H_