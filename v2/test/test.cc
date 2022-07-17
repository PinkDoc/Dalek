#include <array>
#include <cassert>

class tester
{
public:
    int val;
    tester(): val(996234439) {}
};

int main()
{
    std::array<tester, 1024> a;
    for (auto& i : a)
    {
        assert(i.val == 996234439);
    }
}