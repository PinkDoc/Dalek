#include "../core/event_module/dalek_timer.hpp"
#include <set>
#include <cassert>
#include <iostream>
using namespace dalek::timer;

int cream = 0;

void print()
{
    cream++;
}

void test1()
{
    timer_util_list list;
    std::set<int> id_ok;

    for (auto i = 0; i < 20000; ++i)
    {
        auto id = list.add([]{
            print();
        });

        assert(id_ok.find(id) == id_ok.end());
    }

    for (auto i = 65; i < 1065; ++i)
    {
        assert(list.remove(i));
    }

    for (auto i = 25000; i < 30000; ++i)
    {
        assert(!list.remove(i));
    }

    list.tick();

    assert(cream == 19000);
    std::cout << "[Test Pass]" << std::endl;
}

int main()
{
    test1();
}