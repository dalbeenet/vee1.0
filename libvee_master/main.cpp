#include <vee/delegate.h>

int __cdecl case_cdecl(int a, int b)
{
    printf("CALL \t %s\n", __FUNCTION__);
    return a + b;
}

int __stdcall case_stdcall(int a, int b)
{
    printf("CALL \t %s\n", __FUNCTION__);
    return 0;
}

struct case_functor
{
    int i;
    case_functor(int arg)
    {
        i = arg;
    }
    int operator()(int, int)
    {
        printf("CALL \t %s [i:%d]\n", __FUNCTION__, i);
        return 0;
    }
    friend bool operator==(const case_functor& lhs,  const case_functor& rhs)
    {
        return lhs.i == rhs.i;
    }
};

struct case_functor2
{
    int i;
    int j;
    case_functor2(int arg)
    {
        i = arg;
    }
    int operator()(int, int)
    {
        printf("CALL \t %s [i:%d]\n", __FUNCTION__, i);
        return 0;
    }
    friend bool operator==(const case_functor2& lhs, const case_functor2& rhs)
    {
        return lhs.i == rhs.i;
    }
};

class case_bind_requirement
{
    int i;
public:
    case_bind_requirement(int a)
    {
        i = a;
    }
    int func(int bindit, int, int)
    {
        printf("CALL \t %s [bindit: %d, i: %d]\n", __FUNCTION__, bindit, i);
        return 0;
    }
    friend bool operator==(const case_bind_requirement& lhs, const case_bind_requirement& rhs)
    {
        return lhs.i == rhs.i;
    }
};

#define HR puts("______________________________________________")
int main()
{
    HR;
    printf("INSERT\n");
    _VEE delegate<int(int, int)> e;
    e += case_stdcall;
    e += case_cdecl;
    e += case_functor(200);
    e += std::make_pair((__int64)1, case_functor2(100));
    case_bind_requirement bind_test(999);
    e += std::bind(&case_bind_requirement::func, // function addr
                   &bind_test, // binding this pointer
                   1,
                   std::placeholders::_1, // placeholder for arg1 
                   std::placeholders::_2); // placehoder for arg2
    e += std::bind(&case_bind_requirement::func, // function addr
                   &bind_test, // binding this pointer
                   2,
                   std::placeholders::_1, // placeholder for arg1 
                   std::placeholders::_2); // placehoder for arg2
    
    HR;
    printf("CALL_TEST\n");
    e(1, 1); // call
    
    HR;
    printf("REMOVE\n");
    e -= case_stdcall;
    e -= case_cdecl;
    e -= case_functor(200);
    e -= (__int64)1;

    HR;
    printf("CALL_TEST\n");
    e(1, 1); // call
}