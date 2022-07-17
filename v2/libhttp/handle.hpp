#pragma once

#include "request.hpp"
#include "response.hpp"

namespace lighthttp
{
using request_handler = std::function<void(request &, response &)>;
} // namespace lighthttp
