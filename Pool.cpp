/*
    This file is part of pqpp.

    Copyright (C) 2019 ReimuNotMoe

    This program is free software: you can redistribute it and/or modify
    it under the terms of the MIT License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include "Pool.hpp"

using namespace pqpp;

Pool::Pool(boost::asio::io_service &__io_svc) : io_svc(__io_svc) {

}

Pool::Pool(boost::asio::io_service &__io_svc, struct client_config __config) : Pool(__io_svc) {
	config(std::move(__config));
}

Pool::Pool(boost::asio::io_service &__io_svc, const std::function<void(pqpp::client_config &)> &cfg_func) : Pool(__io_svc) {
	config(cfg_func);
}


void Pool::config(std::shared_ptr<client_config>& __config) {
	conf = __config;
}

void Pool::config(const std::function<void(pqpp::client_config &)> &cfg_func) {
	if (!conf)
		conf = std::make_shared<client_config>();

	cfg_func(*conf);
}

void Pool::config(struct client_config __config) {
	if (!conf)
		conf = std::make_shared<client_config>(__config);

//	conf = std::move(__config);
}