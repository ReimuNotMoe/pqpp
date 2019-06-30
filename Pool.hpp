/*
    This file is part of pqpp.

    Copyright (C) 2019 ReimuNotMoe

    This program is free software: you can redistribute it and/or modify
    it under the terms of the MIT License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef PQPP_POOL_HPP
#define PQPP_POOL_HPP

#include "CommonIncludes.hpp"
#include "Client.hpp"

namespace pqpp {
	class Pool : public std::enable_shared_from_this<Pool> {
	protected:
		std::shared_ptr<client_config> conf;
		std::vector<std::shared_ptr<Client>> pool;
		boost::asio::io_service& io_svc;
		std::deque<std::shared_ptr<Client>> queue;
		size_t queued_size = 0;

	public:
		explicit Pool(boost::asio::io_service& __io_svc);
		Pool(boost::asio::io_service& __io_svc, struct client_config __config);
		Pool(boost::asio::io_service& __io_svc, const std::function<void(pqpp::client_config&)>& cfg_func);

		void config(std::shared_ptr<client_config>& __config);
		void config(struct client_config __config);
		void config(const std::function<void(pqpp::client_config&)>& cfg_func);

		std::shared_ptr<Client> get() {
			boost::asio::system_timer timer(io_svc);

			if (queued_size >= conf->pool_size) {
				// TODO: wait
			} else {

				if (queue.empty()) {
//					auto ret = std::make_shared<Client>(io_svc, )
				} else {
					auto ret = queue.front();
					queue.pop_front();
					return ret;
				}
			}
		}

	};
}

#endif //PQPP_POOL_HPP
