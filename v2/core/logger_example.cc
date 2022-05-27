#include "dalek_logger.hpp"
#include <iostream>
#include <functional>

void one_thread(int i)
{
    for (int j = 0; j <10; ++j)
        DLOG_WARN << "test error" << i << " " << j;
}

int main()
{
    Dalek_LoggerInit("fuck");
    std::vector<std::thread> threads;
    for (int i = 0; i < 1; ++i)
    {
        threads.emplace_back(std::bind(one_thread, i));
    }

    for (auto& i : threads) i.join();

}