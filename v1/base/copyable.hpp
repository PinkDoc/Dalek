#ifndef PINK_COPY_H
#define PINK_COPY_H

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
