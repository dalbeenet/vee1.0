#ifndef _VEE_QUEUE_H_
#define _VEE_QUEUE_H_

#include <atomic>
#include <vector>
#include <vee/macro.h>

_VEE_BEGIN

template <typename DataType>
class syncronized_ringqueue
{
public:
    typedef DataType data_type;
    syncronized_ringqueue(const std::size_t capacity):
        _capacity(capacity),
        _data_guards(capacity),
        _data_container(capacity),
        _overwrite_mode(false),
        _rear(0),
        _front(0),
        _size(0)
    {
        // in the msvc2013, std::atomic_flag is can not initialized with "ATOMIC_FLAG_INIT"
        _indexs_guard.clear();
        for (auto& it : _data_guards)
        {
            it.clear();
        }
    }
    ~syncronized_ringqueue()
    {
        
    }
    inline bool is_empty()
    {
        if (_size == 0)
            return true;
        return false;
    }
    inline bool is_full()
    {
        if (_size == _capacity)
            return true;
        return false;
    }
    template <class FwdDataTy>
    bool enqueue(FwdDataTy&& data)
    {
        while (_indexs_guard.test_and_set(std::memory_order_acquire));
        int rear = _rear;
        if (is_full())
        {
            if (!_overwrite_mode)
            {
                _indexs_guard.clear(std::memory_order_release);
                return false;
            }
            ++_rear %= _capacity;
            ++_front %= _capacity;
        }
        else
        {
            ++_rear %= _capacity;
            ++_size;
        }
        while (_data_guards[rear].test_and_set(std::memory_order_acquire));
        _indexs_guard.clear(std::memory_order_release);
        
        _data_container[rear] = std::forward<FwdDataTy>(data);
        _data_guards[rear].clear(std::memory_order_release);
        return true;
    }
    bool dequeue(data_type& out)
    {
        while (_indexs_guard.test_and_set(std::memory_order_acquire));
        if (!_size)
        {
            _indexs_guard.clear(std::memory_order_release);
            return false;
        }
        --_size;
        int front = _front;
        ++_front %= _capacity;
        while (_data_guards[front].test_and_set(std::memory_order_acquire));
        _indexs_guard.clear(std::memory_order_release);
        typedef std::remove_reference<data_type>::type orig_type;
        typedef std::conditional <
            std::is_move_assignable<orig_type>::value,
            std::add_rvalue_reference<orig_type>::type,
            std::add_lvalue_reference<orig_type>::type >::type request_type;
        //static_assert(std::is_rvalue_reference<request_type>::value, "THIS IS NOT RVALUE REFERENCE!");
        out = static_cast<request_type>(_data_container[front]);
        //out = std::move(_data_container[front]);
        _data_guards[front].clear(std::memory_order_release);
        return true;
    }
    inline std::size_t capacity()
    {
        return _capacity;
    }
private:
    // default ctor is deleted.
    syncronized_ringqueue() = delete;
protected:
    std::vector< std::atomic_flag > _data_guards;
    std::vector<data_type>  _data_container;
    std::size_t _capacity;
    std::atomic_flag _indexs_guard;
    bool _overwrite_mode;
    int _rear;
    int _front;
    int _size;
};

_VEE_END

#endif // _VEE_QUEUE_H_