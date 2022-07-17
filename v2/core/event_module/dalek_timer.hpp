/**
 * @file dalek_timer.hpp
 * @author pink (996234439@qq.com)
 * @brief 
 * @version 0.1
 * @date 2022-07-13
 * @copyright Copyright (c) 2022
 * 
 */


#pragma once

// FIXME remove map, use pointer ?
#include <map>

#include <list>
#include <atomic>
#include <chrono>
#include <cassert>
#include <functional>

#include <sys/timerfd.h>

namespace dalek {
namespace timer {

using tid = std::size_t;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::minutes;
using std::chrono::hours;

inline constexpr std::size_t TVN_SIZE = 1 << 6; // 64
inline constexpr std::size_t TVR_SIZE = 1 << 8; // 256

template <typename T>
inline milliseconds cast_to_mill(std::size_t delay)
{
    static_cast(std::is_same<T, milliseconds> ||
                    std::is_same<T, seconds> ||
                    std::is_same<T, minutes> ||
                    std::is_same<T, hours>,"T should be a unit of std::chrono::[time unit]");

    return std::chrono::duration_cast<milliseconds>(T(delay));
}

/**
 * @brief Usage  milliseconds ret = delay_to_mill<seconds>(100)   ret = 100 * 1000 mills
 *
 * @tparam T
 * @param delay
 * @return milliseconds
 */
template <typename T>
inline milliseconds delay_to_mill(std::size_t delay) {
    return cast_to_mill<T>(delay) ;
}

template <typename T>
inline std::size_t delay_to_mill_count(std::size_t delay)
{
    return cast_to_mill<T>(delay).count();
}

class tid_util
{
private:
    static std::atomic<tid> id_;
public:

    /**
     * @param Return a unique tid
     * */
    static tid get()
    {
        id_++;
        return id_.load();
    }
};

class timer_util_list_node
{
private:
    std::function<void()> callback_;
    tid id_;
public:
    inline explicit timer_util_list_node(std::function<void()> cb , tid id):
        callback_(std::move(cb)),
        id_(id)
    {}

    inline timer_util_list_node(timer_util_list_node&& n): callback_(std::move(n.callback_)), id_(std::move(n.id_))
    {}

    inline timer_util_list_node& operator= (timer_util_list_node&& n)
    {
        callback_ = std::move(n.callback_);
        id_ = std::move(n.id_);
    }

    inline void tick()
    {
        if (callback_)
        {
            callback_();
        }
    }

    inline std::function<void()>& callback()
    {
        return callback_;
    }

    /**
     * @param cb Set callback
     * */
    inline void set_callback(std::function<void()> cb)
    {
        callback_ = std::move(cb);
    }

    /**
     * @param id Set frame's id
     * */
    inline void set_id(tid id)
    {
        id_ = id;
    }

    /**
     * @return Return frame's id
     * */
    inline tid get_tid() const { return id_; }
};

class timer_util_list
{
private:
    using node_list = std::list<timer_util_list_node>;
    node_list nodes_;
    std::map<tid, node_list::iterator> map_;
public:

    timer_util_list() = default;
    ~timer_util_list() = default;

    timer_util_list(timer_util_list&& t):
            nodes_(std::move(t.nodes_)),
            map_(std::move(t.map_))
    {}

    timer_util_list& operator= (timer_util_list&& t)
    {
        nodes_ = std::move(t.nodes_);
        map_ = std::move(t.map_);
    }

    inline node_list& nodes()
    {
        return nodes_;
    }

    inline std::map<tid, node_list::iterator> map()
    {
        return map_;
    }

    inline tid add(std::function<void()> cb)
    {
        tid id = tid_util::get();
        nodes_.emplace_front(cb, id);
        map_.emplace(id, nodes_.begin());
        return id;
    }

    inline bool remove(tid id)
    {
        auto iter = map_.find(id);
        if (iter == map_.end())
        {
            return false;
        }
        else
        {
            auto list_iter = iter->second;
            nodes_.erase(list_iter);
            map_.erase(iter);
            return true;
        }
        return false;
    }

    inline void tick()
    {
        for (auto& i : nodes_)
        {
            i.tick();
        }
        nodes_.clear();
        map_.clear();
    }
};


template <std::size_t TVNSIZE>
class timer_wheel
{
private:
    std::array<timer_util_list, TVNSIZE> frames_;
    std::map<tid, std::size_t> map_;
    std::size_t p_;
public:

    inline explicit timer_wheel(): p_(0) {}

    inline bool is_point_to_last()
    {
        return p_ == TVNSIZE - 1;
    }

    inline tid add(std::function<void()> cb, std::size_t expired)
    {
        assert(expired < TVNSIZE);
        auto p = (expired + p_) % TVNSIZE;
        auto id = frames_[p].add(cb);
        map_.emplace(id, p);
        return id;
    }

    inline bool remove(tid id)
    {
        auto iter = map_.find(id);
        if (iter == map_.end())
        {
            return false;
        }
        else
        {
            auto p = iter->second;
            auto ret = frames_[p].remove(id);
            map_.erase(iter);
            return ret;
        }
        return false;
    }

    /**
     * The 0 level wheel tick(root wheel)
     * */
    inline void tick()
    {
        frames_[p_].tick();
        p_ = (p_ + 1) % TVNSIZE;
    }

    /**
     * Remove tasks to the previous level timer_wheel, the high level wheel tick.
     * */
    inline timer_util_list remove_tasks()
    {
        timer_util_list list = std::move(frames_[p_]);
        p_ = (p_ + 1) % TVNSIZE;
        return list;
    }

    /**
     * Get tasks from the previous level timer_wheel
     * */
    inline void add_tasks(timer_util_list list)
    {
        auto p = (p_ + TVNSIZE - 1) % TVNSIZE;
        auto& nodes = list.nodes();
        auto& fnode = frames_[p].nodes();
        auto& fmap = frames_[p].map();
        for (auto& i : nodes)
        {
            fnode.emplace_front(std::move(i));
            fmap.emplace(fnode.front().get_tid(), fnode.begin());
        }
    }
};

/**
 * @brief 
 * 
 * @tparam num The number of wheel
 */
template <std::size_t num>
class multi_level_time_wheel
{
private:
    timer_wheel<TVR_SIZE> root_wheel_;
    std::array<timer_wheel<TVN_SIZE>, num> node_wheels_;
public:
    
    template <typename T>
    inline tid add(std::function<void()> cb, std::size_t delay)
    {
        auto expired = delay_to_mill_count<T>(delay);
        if (expired < TVR_SIZE) {
            return root_wheel_.add(std::move(cb), expired);
        } else {
            for (auto i = 1; i <= num; ++i)
            {
                // 2^8 ~ 2^14 - 1, 2^14 ~ 2^20 ^-1...
                if (expired < (1 << (TVR_SIZE + i * TVN_SIZE)))
                {
                    auto temp = 
                    return node_wheels_[i - 1].add(std::move(cb), expired);
                }
            }
        }
    }

    inline bool remove(tid id)
    {

    }

    inline void tick()
    {

    }
};

std::atomic<tid> tid_util::id_ = 0;

}
} // namespace dalek::timer
