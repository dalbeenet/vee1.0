#ifndef _VEE_ACTOR_H_
#define _VEE_ACTOR_H_

#include <vee/signal_channel.h>
#include <vee/delegate.h>
#include <vee/queue.h>
#include <tuple>
#include <mutex>

_VEE_BEGIN

namespace xactor {
    template <typename T>
    inline bool compare_and_swap_strong(::std::atomic<T>& dst, int exp, int value)
    {
        return dst.compare_exchange_strong(exp, value);
    }
}

template <class FTy>
class actor;

template <class RTy, class ...Args>
class actor < RTy(Args ...) >
{
    enum class sigid: int
    {
        spawn = 0,
        active,
        go_exit,
        exit_ok,
    };
public:
    typedef vee::delegate<RTy(Args ...)> delegate;
    typedef std::tuple<Args ...> argstuple;
    typedef std::pair<delegate, argstuple> job;
    actor(std::size_t queue_size):
        _queue_size(queue_size),
        _job_queue(queue_size)
    {
        _spawn();
    }
    ~actor()
    {
        kill();
    }
    template <class Delegate, typename ...FwdArgs>
    inline int request(Delegate&& _delegate, FwdArgs&& ... args)
    {
        return this->request(std::forward<Delegate>(_delegate), std::make_tuple(args ...));
    }
    template <class Delegate, class ArgsTuple>
    int request(Delegate&& _delegate, ArgsTuple&& _tuple)
    {
        /*_setup_delegate(std::forward<Delegate>(_delegate));
        _setup_argspack(std::forward<ArgsTuple>(_tuple));*/
        if (_state.load() == DEAD)
        {
            return -1;
        }
        int old_counter = _job_counter++;
        while (!_job_queue.enqueue(std::make_pair(std::forward<Delegate>(_delegate), std::forward<ArgsTuple>(_tuple))));
        if (old_counter == 0)
        {
            while(!_sigch.try_send(sigid::active));
        }
        return old_counter;
    }
    bool kill()
    {
        if (!xactor::compare_and_swap_strong(_state, IDLE, DEAD))
            return false;
        _thread.detach();
        while (!_sigch.try_send(sigid::go_exit));
        while (_sigch.recv() != sigid::exit_ok);
        return true;
    }
    bool respawn()
    {
        if (!xactor::compare_and_swap_strong(_state, DEAD, SPAWN))
            return false;
        _spawn();
        return true;
    }
    void waiting_for_exit()
    {
        if (false == _thread.joinable())
        {
            return;
        }
        else
        {
            _thread.join();
        }
    }
protected:
    void _spawn()
    {
        std::thread t(&actor<RTy(Args...)>::_actormain, this);
        if (_sigch.recv() != sigid::spawn)
        {
            throw std::runtime_error("actor constructor receives invalid signal!");
        }
        t.swap(_thread);
    }
    int _actormain()
    {
        if (_state != SPAWN)
        {
            throw std::runtime_error("actor thread creation failure! actor state has wrong value.");
        }
        _state.store(LOADED);
        while (!_sigch.try_send(sigid::spawn));
        _state.store(IDLE);
        while (true)
        {
            switch (_sigch.recv())
            {
            case sigid::active:
                _epoch();
                break;
            case sigid::go_exit:
                // exit process
                while(!_sigch.try_send(sigid::exit_ok));
                _state = DEAD;
                return 0;
            default:
                throw std::runtime_error("wrong signal received!");
                break;
            }
        }
        return 0;
    }
    bool _epoch()
    {
        job current_job;
        while (true)
        {
            if (!_job_queue.dequeue(current_job))
            {
                throw std::runtime_error("actor::_epoch - job queue is empty!");
                return false;
            }
            delegate& e = current_job.first;
            argstuple& args = current_job.second;
            e(args);
            int old_counter = --_job_counter;
            if (old_counter == 0)
                break;
        }
        if (!xactor::compare_and_swap_strong(_state, PROC, IDLE))
            return false;
        return true;
    }
private:
    vee::signal_channel<sigid> _sigch;
    vee::syncronized_ringqueue<job> _job_queue;
    std::atomic<int> _job_counter = 0;
    std::thread _thread;
    std::atomic<int> _state = SPAWN;
    std::size_t _queue_size;
    static const int SPAWN  = 0;
    static const int LOADED = 1;
    static const int IDLE   = 3;
    static const int PROC   = 4;
    static const int DEAD   = 5;
};

_VEE_END

#endif