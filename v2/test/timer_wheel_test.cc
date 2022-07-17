#include "../core/event_module/dalek_timer.hpp"
#include <set>
#include <cassert>
#include <iostream>

using namespace dalek::timer;

int cream = 0;

void call()
{
    cream++;
}

void test1()
{
    timer_wheel<1024> wheel;
    tid ids;

    for (auto i = 0; i < 5; ++i) {
        auto id = wheel.add([] {
            call();
        },i);
        if (i == 3) ids = id;
    }

    assert(wheel.remove(ids));

    wheel.tick();
    assert(cream == 1);

    wheel.tick();
    assert(cream == 2);

    for (auto i = 0; i < 1024; ++i)
    {
        wheel.tick();
    }

    assert(cream == 4);
}

int main()
{
    test1();
}