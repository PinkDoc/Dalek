#pragma once

#include "dalek_core.hpp"
// dalek as a upstream

namespace upstream
{

    class dalek_upstream
    {
    protected:
        core::Channel chan_;
    public:
        virtual int create_request();
        virtual int reinit_request();
        virtual int handle_header();
        virtual int stop_request();                          // client stop
        virtual int finalize_request();
        virtual int back_server_input();        // return the back server input
        virtual ~dalek_upstream();
    };




}