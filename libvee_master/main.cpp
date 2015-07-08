//             Copyright Evgeny Panasyuk 2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <type_traits>
#include <functional>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <typeinfo>
#include <utility>
#include <ostream>
#include <vector>
#include <string>

using namespace std;

// _____________________________Implementation__________________________________________

#define USE_VARIADIC_TEMPLATES 0

template<typename Callback, typename Function> inline
bool func_compare(const Function &lhs, const Function &rhs)
{
    typedef typename conditional
        <
        is_function<Callback>::value,
        typename add_pointer<Callback>::type,
        Callback
        > ::type request_type;
    if (const request_type *lhs_internal = lhs.template target<request_type>())
    {
        if (const request_type *rhs_internal = rhs.template target<request_type>())
        {
            return *rhs_internal == *lhs_internal;
        }
    }
    return false;
}

#if USE_VARIADIC_TEMPLATES
#define FUNC_SIG_TYPES typename ...Args
#define FUNC_SIG_TYPES_PASS Args...
#else
#define FUNC_SIG_TYPES typename function_signature
#define FUNC_SIG_TYPES_PASS function_signature
#endif

template<FUNC_SIG_TYPES>
struct function_comparable: function < FUNC_SIG_TYPES_PASS >
{
    typedef function<FUNC_SIG_TYPES_PASS> Function;
    bool(*type_holder)(const Function &, const Function &);
public:
    function_comparable()
    {
    }
    template<typename Func> function_comparable(Func f)
        : Function(f), type_holder(func_compare<Func, Function>)
    {
    }
    template<typename Func> function_comparable &operator=(Func f)
    {
        Function::operator=(f);
        type_holder = func_compare < Func, Function > ;
        return *this;
    }
    friend bool operator==(const Function &lhs, const function_comparable &rhs)
    {
        return rhs.type_holder(lhs, rhs);
    }
    friend bool operator==(const function_comparable &lhs, const Function &rhs)
    {
        return rhs == lhs;
    }
    // ...
    friend void swap(function_comparable &lhs, function_comparable &rhs)// noexcept
    {
        lhs.swap(rhs);
        lhs.type_holder.swap(rhs.type_holder);
    }
};

// ________________________________Example______________________________________________

typedef void (function_signature)();

void __stdcall func1()
{
    cout << "func1" << endl;
}

void __cdecl func3()
{
    cout << "func3" << endl;
}

class func2
{
    int data;
public:
    explicit func2(int n): data(n)
    {
    }
    friend bool operator==(const func2 &lhs, const func2 &rhs)
    {
        return lhs.data == rhs.data;
    }
    void operator()()
    {
        cout << "func2, data=" << data << endl;
    }
};
struct Caller
{
    template<typename Func>
    void operator()(Func f)
    {
        f();
    }
};
class Callbacks
{
    vector<function<function_signature>> v;
public:
    void register_callback_comparator(function_comparable<function_signature> callback)
    {
        v.push_back(callback);
    }
    void register_callback(function<function_signature> callback)
    {
        v.push_back(callback);
    }
    void unregister_callback(function_comparable<function_signature> callback)
    {
        auto it = find(v.begin(), v.end(), callback);
        if (it != v.end())
            v.erase(it);
        else
            throw runtime_error("not found");
    }
    void call_all()
    {
        for_each(v.begin(), v.end(), Caller());
        cout << string(16, '_') << endl;
    }
};

#include <vee/delegate.h>

int sum(int a, int b)
{
    printf("ret: %d\n", a + b);
    return a + b;
}

int main()
{
    _VEE delegate<int(int, int)> e;
    e += sum;
    e(1, 2);
    std::function<int(int, int)> wsum(sum);
    printf("unique_key(wsum): %d\n", _VEE unique_key<int(int, int)>(wsum));
    std::function<void()> wfunc1(func1);
    printf("unique_key(wfunc1): %d\n", _VEE unique_key<void()>(wfunc1));
    Callbacks cb;
    function_comparable<function_signature> f;
    f = func1;
    cb.register_callback_comparator(f);

    cb.register_callback(func2(1));
    cb.register_callback(func2(2));
    cb.register_callback(func3);
    cb.call_all();

    cb.unregister_callback(func2(2));
    cb.unregister_callback(func3);
    cb.call_all();
    cb.unregister_callback(func1);
    cb.call_all();
}