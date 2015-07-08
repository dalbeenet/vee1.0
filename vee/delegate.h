#ifndef _VEE_DELEGATE_H_
#define _VEE_DELEGATE_H_

#include <vector>
#include <functional>
#include <vee/macro.h>

_VEE_BEGIN

template <class T>
void* addr_cast(T pointer)
{
    auto ptr = pointer;
    void* addr = *(void**)(&ptr);
    return addr;
}

template <typename FuncSig, typename _Function>
bool compare_functions(const _Function& lhs, const _Function& rhs)
{
    typedef typename std::conditional
        <
        std::is_function<FuncSig>::value,
        typename std::add_pointer<FuncSig>::type,
        FuncSig
        > ::type funcptr_type;
    if (const funcptr_type* lhs_internal = lhs.template target<funcptr_type>())
    {
        if (const funcptr_type* rhs_internal = rhs.template target<funcptr_type>())
            return *rhs_internal == *lhs_internal;
    }
    return false;
}

template <typename FuncSig, typename _Function>
int unique_key(_Function& f)
{
    typedef typename std::conditional
        <
        std::is_function<FuncSig>::value,
        typename std::add_pointer<FuncSig>::type,
        FuncSig
        > ::type funcptr_type;
    const funcptr_type* internal = f.template target<funcptr_type>();
    return (int)internal;
}

template <typename FTy>
class compareable_function: std::function < FTy >
{
    typedef std::function<FTy> _function_t;
    bool(*_type_holder)(const _function_t&, const _function_t&);
public:
    compareable_function() = default;
    template <typename Func> compareable_function(Func f):
        _function_t(f),
        _type_holder(compare_functions<Func, _function_t>)
    {
        // empty
    }
    template <typename Func> compareable_function& operator=(Func f)
    {
        _function_t::operator =(f);
        _type_holder = compare_functions < Func, _function_t > ;
        return *this;
    }
    friend bool operator==(const _function_t &lhs, const compareable_function &rhs)
    {
        return rhs._type_holder(lhs, rhs);
    }
    friend bool operator==(const compareable_function &lhs, const _function_t &rhs)
    {
        return rhs == lhs;
    }
    // ...
    friend void swap(compareable_function &lhs, compareable_function &rhs)// noexcept
    {
        lhs.swap(rhs);
        lhs._type_holder.swap(rhs._type_holder);
    }
};

template <class T>
class delegate
{
};

template <class RTy, class ...Args>
class delegate < RTy(Args ...) >
{
public:
    typedef delegate< RTy(Args ...) > type;
    typedef type& reference_t;
    typedef type&& rreference_t;
    typedef RTy(*funcptr_t)(Args ...);
    typedef RTy(func_t)(Args ...);
private:
    typedef std::function< func_t > _binder_t;
    typedef std::vector<std::pair<int, _binder_t>> _container_t;
public:
    // ctor
    delegate() = default;
    // dtor
    ~delegate() = default;
    // copy ctor
    delegate(reference_t _left)
    {
        this->_deep_copy(_left);
    }
    // move ctor
    delegate(rreference_t _right)
    {
        this->_deep_copy(_right);
    }
    void operator()(Args&& ...args)
    {
        for (auto& it : _container)
        {
            it.second.operator()(std::forward<decltype(args)>(args)...);
        }
    }
    void operator+=(_binder_t func)
    {
        printf("%s\nkey: %d\n", __FUNCTION__, unique_key<func_t, _binder_t>(func));
        _container.push_back(std::make_pair(unique_key<func_t, _binder_t>(func), func));
    }
    void operator-=(_binder_t func)
    {
        int key = unique_key<func_t, _binder_t>(func);
        printf("%s\nkey: %d\n", __FUNCTION__, key);
        for (auto& it : _container)
        {
            if (it.first == key)
            {
                _container.erase(it);
                break;
            }
        }
    }
    /*template <class CallableObj>
    void operator+=(CallableObj& func)
    {
    printf("%s\nkey: %d\n", __FUNCTION__, unique_key(func));
    _binder_t binder(func);
    _container.push_back(std::make_pair(unique_key(func), binder));
    puts("Success to push function!");
    }
    template <class CallableObj>
    void operator-=(CallableObj& func)
    {
    printf("%s\nkey: %d\n", __FUNCTION__, unique_key(func));
    int key = unique_key(func);
    for (auto& it : _container)
    {
    if (it.first == key)
    {
    _container.erase(it);
    puts("Success to erase function!");
    break;
    }
    }
    }*/
private:
    void _deep_copy(reference_t _left)
    {
        _container = _left._container;
    }
    void _deep_copy(rreference_t _right)
    {
        _container = std::move(_right._container);
    }
private:
    _container_t _container;
};

_VEE_END

#endif // _VEE_DELEGATE_H_