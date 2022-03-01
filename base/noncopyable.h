#ifndef PINK_NONCOPY_HPP
#define PINK_NONCOPY_HPP

namespace pinkx {

// Delete copy
class noncopyable 
{
private:
    noncopyable(noncopyable&) {};
    noncopyable& operator= (noncopyable&) {};
public:
    noncopyable() = default;
    ~noncopyable() = default;
};

}
#endif