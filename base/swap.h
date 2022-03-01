#ifndef PINK_SWAP_HPP
#define PINK_SWAP_HPP

namespace pinkx {
    
    template<typename T>
        inline void Pswap(T& t1, T& t2) {
            T&& tmp(std::move(t1));
            t1      = std::move(t2);
            t2      = std::move(tmp);
        }

}
#endif