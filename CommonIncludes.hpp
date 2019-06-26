/*
    This file is part of pqpp.

    Copyright (C) 2019 ReimuNotMoe

    This program is free software: you can redistribute it and/or modify
    it under the terms of the MIT License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef PQPP_COMMONINCLUDES_HPP
#define PQPP_COMMONINCLUDES_HPP

#include <string>
#include <vector>
#include <functional>

#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cstdint>
#include <cinttypes>

#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include <postgresql/libpq-fe.h>

// io_context is only supported by boost 1.67+, so we use the old API here
// At least this is supported by boost 1.65 - 1.70
#define BOOST_COROUTINES_NO_DEPRECATION_WARNING

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/system_timer.hpp>

#endif //PQPP_COMMONINCLUDES_HPP
