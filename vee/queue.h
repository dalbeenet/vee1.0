#ifndef _VEE_QUEUE_H_
#define _VEE_QUEUE_H_

#include <atomic>
#include <vector>
#include <vee/lock.h>

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
        int rear = 0;
        std::unique_lock<vee::spin_lock> item_lock;
        {
            std::lock_guard<vee::spin_lock> index_lock(_indexs_guard);
            rear = _rear;
            if (is_full())
            {
                if (!_overwrite_mode)
                {
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
            std::unique_lock<vee::spin_lock> item_lock_temp(_data_guards[rear], std::adopt_lock);
            std::swap(item_lock, item_lock_temp);
        }
        _data_container[rear] = std::forward<FwdDataTy>(data);
        return true;
    }
    bool dequeue()
    {
        std::lock_guard<vee::spin_lock> index_lock(_indexs_guard);
        if (!_size)
        {
            return false;
        }
        --_size;
        int front = _front;
        ++_front %= _capacity;
        return true;
    }
    bool dequeue(data_type& out)
    {
        int front = 0;
        std::unique_lock<vee::spin_lock> item_lock;
        {
            std::lock_guard<vee::spin_lock> index_lock(_indexs_guard);
            if (!_size)
            {
                return false;
            }
            --_size;
            front = _front;
            ++_front %= _capacity;

            std::unique_lock<vee::spin_lock> item_lock_temp(_data_guards[front], std::adopt_lock);
            std::swap(item_lock, item_lock_temp);
        }
        typedef std::remove_reference<data_type>::type orig_type;
        typedef std::conditional <
            std::is_move_assignable<orig_type>::value,
            std::add_rvalue_reference<orig_type>::type,
            std::add_lvalue_reference<orig_type>::type >::type request_type;
        out = static_cast<request_type>(_data_container[front]);
        return true;
    }
    void clear()
    {
        while (!this->dequeue());
    }
    inline std::size_t capacity()
    {
        return _capacity;
    }
protected:
    /*void _deep_copy(syncronized_ringqueue<data_type>& rhs)
    {
        while (_indexs_guard.test_and_set(std::memory_order_acquire));
        while (rhs._indexs_guard.test_and_set(std::memory_order_acquire));

        _indexs_guard.clear(std::memory_order_release);
    }
    void _deep_copy(syncronized_ringqueue<data_type>&& rhs)
    {
        while (_indexs_guard.test_and_set(std::memory_order_acquire));

        _indexs_guard.clear(std::memory_order_release);
    }*/
private:
    // default ctor is deleted.
    syncronized_ringqueue() = delete;
protected:
    std::vector< vee::spin_lock > _data_guards;
    std::vector<data_type>  _data_container;
    std::size_t _capacity;
    vee::spin_lock _indexs_guard;
    bool _overwrite_mode;
    int _rear;
    int _front;
    int _size;
};

_VEE_END

#endif // _VEE_QUEUE_H_