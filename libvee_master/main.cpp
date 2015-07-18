#include <vee/queue.h>
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
        return *this;
    }
    test_object& operator=(test_object&&)
    {
        printf("%s\n", __FUNCSIG__);
        return *this;
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
    {
        std::thread t1(sender);
        std::thread t2(receiver);
        t1.join();
        t2.join();
    }
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
    {
        vee::actor<int(int, int)> actor;
        vee::delegate<int(int, int)> e;
        e += sum;
        e(std::make_tuple(1, 2));
        //e(std::make_tuple(1, 2));
        while (!actor.try_request(e, 1, 2));
        //while (!actor.try_request(&e, 1, 2));
        actor.try_request(e, std::make_tuple(3, 4));
        while (!actor.try_request(std::move(e), 1, 2));
        //std::this_thread::sleep_for(std::chrono::milliseconds::duration(1000));
    }
    HR;
    printf("BUSYWAIT QUEUE TEST\n");
    {
        test_object a, b;
        puts("TYPE CAST");
        a = static_cast<test_object>(b);
        puts("REFERENCE CAST");
        a = static_cast<test_object&>(b);
        puts("RIGHT REFERENCE CAST");
        a = static_cast<test_object&&>(b);
        vee::syncronized_ringqueue<int> queue(100);
        vee::syncronized_ringqueue<int> result(500);
        auto producer = [&queue]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds::duration(100));
            int cnt = 0;
            for (int i = 0; i < 500; ++i)
            {
                while (!queue.enqueue(i))
                {
                    ++cnt;
                }
            }
            printf("Enqueue failure!!! %d times\n", cnt);
        };
        auto consumer = [&queue, &result]()
        {
            int buf = 0, cnt = 0;
            for (int i = 0; i < 500; ++i)
            {
                while (!queue.dequeue(buf))
                {
                    ++cnt;
                }
                //printf("tid: %d\n", std::this_thread::get_id());
                result.enqueue(buf);
                //printf("value: %d\n", buf);
                buf = -1;
            }
            printf("Dequeue failure!!! %d times\n", cnt);
        };
        std::thread t1(producer);
        std::thread t2(consumer);
        std::thread t3(consumer);
        t1.join();
        t2.join();
        t3.join();
        int buf = 0;
        while (result.dequeue(buf))
        {
            printf("value: %d\n", buf);
        }
    }
    printf("Press any key to exit!\n");
    getch();
    return 0;
}