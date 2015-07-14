#include <vee/actor.h>
#include <conio.h>
#define HR for(int i = 0; i < 80; ++i) printf("_")

vee::alram_channel signal;

int receiver()
{
    signal.recv();
    printf("signal received!!\n");
    return 0;
}

int sender()
{
    do
    {
        printf("try send a signal\n");
    }
    while (!signal.try_send());
    return 0;
}

int sum(int a, int b)
{
    printf("%d + %d = %d\n", a, b, a + b);
    return a + b;
}

struct test_object
{
    test_object()
    {
        printf("%s\n", __FUNCSIG__);
    }
    ~test_object()
    {
        printf("%s\n", __FUNCSIG__);
    }
    test_object(const test_object&)
    {
        printf("%s\n", __FUNCSIG__);
    }
    test_object(test_object&&)
    {
        printf("%s\n", __FUNCSIG__);
    }
    test_object& operator=(test_object&)
    {
        printf("%s\n", __FUNCSIG__);
    }
    test_object& operator=(test_object&&)
    {
        printf("%s\n", __FUNCSIG__);
    }
};

void copy_test(test_object& lvalue)
{
    printf("%s\n", __FUNCSIG__);
}
//
//void copy_test(test_object&& rvalue)
//{
//    printf("%s\n", __FUNCSIG__);
//}
//
//void copy_test(test_object* ptr)
//{
//    printf("%s\n", __FUNCSIG__);
//}

#include <typeinfo.h>

int main()
{
    HR;
    printf("# SIGNAL TEST\n");
    std::thread t1(sender);
    std::thread t2(receiver);
    t1.join();
    t2.join();
    HR;
    printf("# TUPLE UNPACKER TEST\n");
    {
        std::function<int(int, int)> func = sum;
        vee::call_by_tuple(sum, std::make_tuple(1, 2)); // function type
        vee::call_by_tuple(func, std::make_tuple(3, 4)); // std::function type
    }
    HR;
    printf("# FORWARD TEST\n");
    {
        vee::call_by_tuple(copy_test, std::make_tuple(test_object()));
    }
    HR;
    printf("# ACTOR TEST\n");
    vee::actor<int(int, int)> actor;
    vee::delegate<int(int, int)> e;
    e += sum;
    e(std::make_tuple(1, 2));
    //e(std::make_tuple(1, 2));
    while(!actor.try_request(e, 1, 2));
    while(!actor.try_request(&e, 1, 2));
    actor.try_request(e, std::make_tuple(3, 4));
    while(!actor.try_request(std::move(e), 1, 2));
    getch();
    return 0;
}