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
	class PoolClient;

	class pool_ctx {
	public:
		std::shared_ptr<client_config>& conf;

		std::deque<std::shared_ptr<Client>>& queue;
		size_t& queued_size;

		std::chrono::seconds& queue_timeout;
		boost::asio::system_timer& queue_timer;

		pool_ctx(std::shared_ptr<client_config>& __conf, std::deque<std::shared_ptr<Client>>& __queue, size_t& __queued_size,
			std::chrono::seconds& __queue_timeout, boost::asio::system_timer& __queue_timer);

		void give_back(std::shared_ptr<Client> __client);
	};

	class Pool {
	protected:
		std::shared_ptr<client_config> conf;

		boost::asio::io_service& io_service;

		std::shared_ptr<pool_ctx> ctx;

		std::deque<std::shared_ptr<Client>> queue;
		size_t queued_size = 0;

		std::chrono::seconds queue_timeout;
		boost::asio::system_timer queue_timer;

		void setup_queue_timer();
		void give_back(std::shared_ptr<Client> __client);

	public:
		explicit Pool(boost::asio::io_service& __io_svc);
		Pool(boost::asio::io_service& __io_svc, struct client_config __config);
		Pool(boost::asio::io_service& __io_svc, const std::function<void(pqpp::client_config&)>& cfg_func);

		void config(std::shared_ptr<client_config>& __config);
		void config(struct client_config __config);
		void config(const std::function<void(pqpp::client_config&)>& cfg_func);

		PoolClient client(boost::asio::yield_context& __yield_ctx);

	};

	class PoolExposed : public Pool {
	public:
		using Pool::ctx;

		using Pool::conf;

		using Pool::queue;
		using Pool::queued_size;

		using Pool::io_service;

		using Pool::queue_timer;
		using Pool::queue_timeout;

		using Pool::setup_queue_timer;
		using Pool::give_back;

		explicit PoolExposed(boost::asio::io_service& __io_svc) : Pool(__io_svc) {

		}
	};

	class ClientContainer {
	public:
		std::shared_ptr<Client> client;

		~ClientContainer() {
			if (client)
				client->release();
		}
	};

	class PoolClient {
	protected:
		PoolExposed& parent_pool;

		boost::asio::io_service& io_service;
		boost::asio::yield_context& yield_ctx;

		boost::asio::system_timer& queue_timer;
		std::chrono::seconds& queue_timeout;

	public:
		PoolClient(Pool& __parent_pool, boost::asio::yield_context& __yield_ctx);

		ClientContainer get();
		void get(const std::function<void(const std::exception *, Client *)>& __callback);

		Result query(const std::string& __query_str);
		Result query(const std::string& __query_str, const std::vector<std::string>& __values);
		void query(const std::string& __query_str, const std::function<void(const std::exception *err, const Result &res)>& __callback);
		void query(const std::string& __query_str, const std::vector<std::string>& __values, const std::function<void(const std::exception *err, const Result &res)>& __callback);

	};
}

#endif //PQPP_POOL_HPP
