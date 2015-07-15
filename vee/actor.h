#ifndef _VEE_ACTOR_H_
#define _VEE_ACTOR_H_

#include <vee/signal_channel.h>
#include <vee/delegate.h>
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
    actor()
    {
        _spawn();
    }
    ~actor()
    {
        while(!try_kill());
    }
    template <class Delegate, typename ...FwdArgs>
    bool try_request(Delegate&& _delegate, FwdArgs&& ... args)
    {
        return this->try_request(std::forward<Delegate>(_delegate), std::make_tuple(args ...));
    }
    template <class Delegate, class ArgsTuple>
    bool try_request(Delegate&& _delegate, ArgsTuple&& _tuple)
    {
        if (!xactor::compare_and_swap_strong(_state, IDLE, READY))
            return false;
        _setup_delegate(std::forward<Delegate>(_delegate));
        _setup_argspack(std::forward<ArgsTuple>(_tuple));
        while(!_sigch.try_send(sigid::active));
        return true;
    }
    bool try_kill()
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
        std::thread t(&actor<RTy(Args...)>::_tmain, this);
        if (_sigch.recv() != sigid::spawn)
        {
            throw std::runtime_error("actor constructor receives invalid signal!");
        }
        t.swap(_thread);
    }
    int _tmain()
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
                DEBUG_PRINT("job requested!\n");
                _epoch();
                break;
            case sigid::go_exit:
                //DEBUG_PRINT("go exit!");
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
        if (!xactor::compare_and_swap_strong(_state, READY, PROC))
            return false;
        auto& e = *_target_delegate;
        e(*_target_argstuple);
        if (!xactor::compare_and_swap_strong(_state, PROC, IDLE))
            return false;
        return true;
    }
    void _setup_delegate(delegate& e) // lvalue type
    {
        DEBUG_PRINT("lvalue type %s\n", __FUNCTION__);
        _delegate_container = e;
        _target_delegate = &_delegate_container;
    }
    void _setup_delegate(delegate&& e) // rvalue type
    {
        DEBUG_PRINT("rvalue type %s\n", __FUNCTION__);
        _delegate_container = static_cast<delegate&&>(e);
        _target_delegate = &_delegate_container;
    }
    void _setup_delegate(delegate* e_ptr) // pointer type
    {
        DEBUG_PRINT("pointer type %s\n", __FUNCTION__);
        _target_delegate = e_ptr;
    }
    void _setup_argspack(argstuple& _tuple) // lvaue type
    {
        DEBUG_PRINT("lvaue type %s\n", __FUNCTION__);
        _args_container = _tuple;
        _target_argstuple = &_args_container;
    }
    void _setup_argspack(argstuple&& _tuple) // rvalue type
    {
        DEBUG_PRINT("rvalue type %s\n", __FUNCTION__);
        _args_container = static_cast<argstuple&&>(_tuple);
        _target_argstuple = &_args_container;
    }
    void _setup_argspack(argstuple* _tuple_ptr) // pointer type
    {
        DEBUG_PRINT("pointer type %s\n", __FUNCTION__);
        _target_argstuple = _tuple_ptr;
    }
protected:
    vee::signal_channel<sigid> _sigch;
    std::thread _thread;
    std::atomic<int> _state = SPAWN;
    delegate _delegate_container;
    argstuple _args_container;
    delegate* _target_delegate = nullptr;
    argstuple* _target_argstuple = nullptr;
    static const int SPAWN  = 0;
    static const int LOADED = 1;
    static const int READY  = 2;
    static const int IDLE   = 3;
    static const int PROC   = 4;
    static const int DEAD   = 5;
};

_VEE_END

#endif