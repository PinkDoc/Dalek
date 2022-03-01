#ifndef PINK_COPY_HPP
#define PINK_COPY_HPP

namespace pinkx {


class copyable {
protected:
    copyable()          = default;
    ~copyable()         = default;
    copyable(copyable&) = default;
    copyable& operator=(copyable&) = default;
};




}
#endif
