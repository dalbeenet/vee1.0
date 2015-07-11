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

int main()
{
    HR;
    printf("SIGNAL TEST\n");
    std::thread t1(sender);
    std::thread t2(receiver);
    t1.join();
    t2.join();
    HR;
    vee::actor<int(int, int)> actor;
    vee::delegate<int(int, int)> e;
    e += sum;
    actor.request(e, 1, 2);
    actor.request(std::move(e), 1, 2);
    actor.request(&e, 1, 2);
    getch();
    return 0;
}