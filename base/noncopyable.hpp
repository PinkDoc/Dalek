#ifndef PINK_NONCOPY_H
#define PINK_NONCOPY_H

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