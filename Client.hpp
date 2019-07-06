/*
    This file is part of pqpp.

    Copyright (C) 2019 ReimuNotMoe

    This program is free software: you can redistribute it and/or modify
    it under the terms of the MIT License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef PQPP_CLIENT_HPP
#define PQPP_CLIENT_HPP

#include "CommonIncludes.hpp"
#include "Exception.hpp"
#include "Result.hpp"

namespace pqpp {
	struct client_config {
		std::string host;
		std::string user;
		std::string password;
		std::string database;
		uint16_t port = 0;

		std::string connection_string;

		time_t io_timeout = -1;
		time_t idle_timeout = -1;
		time_t queue_timeout = -1;

		size_t pool_size = 0;
	};

	class pool_ctx;
	class Pool;
	class PoolExposed;

	class Client : public std::enable_shared_from_this<Client> {
	protected:

		std::shared_ptr<client_config> conf;

		boost::asio::io_service& io_service;
		boost::asio::system_timer io_timeout_timer;
		boost::asio::system_timer idle_timeout_timer;
		std::chrono::seconds io_timeout_duration;
		boost::asio::yield_context *yield_ctx = nullptr;

		std::unique_ptr<boost::asio::ip::tcp::socket> tcp_socket;
		std::unique_ptr<boost::asio::local::stream_protocol::socket> unix_socket;

		std::shared_ptr<pool_ctx> parent_pool_ctx;

		bool __connected = false;
		PGconn *conn = nullptr;

		std::function<void(const std::exception *)> callback_connect;

		void config_resolve_envvars();

		void do_connect();
		void do_query();
		void do_destruct();

		void setup_asio_sockets(int __fd);

		size_t io_wait_readable();
		void io_wait_writable();

		void start_io_timer();
		void start_idle_timer();

	public:
		Client(boost::asio::io_service& __io_svc, boost::asio::yield_context *__yield_ctx);

		Client(boost::asio::io_service& __io_svc, boost::asio::yield_context *__yield_ctx, std::shared_ptr<pool_ctx> __parent_pool_ctx);

		Client(boost::asio::io_service& __io_svc, boost::asio::yield_context *__yield_ctx, struct client_config __config);

		Client(boost::asio::io_service& __io_svc, boost::asio::yield_context *__yield_ctx, const std::function<void(pqpp::client_config&)>& cfg_func);

		virtual ~Client();

		void assign_yield_context(boost::asio::yield_context *__yield_ctx) {
			yield_ctx = __yield_ctx;
		}

		void config(std::shared_ptr<client_config>& __config);
		void config(struct client_config __config);
		void config(const std::function<void(pqpp::client_config&)>& cfg_func);

		void connect();
		void connect(std::function<void(const std::exception *err)> __callback);

		bool connected() const noexcept {
			return __connected;
		}


		void release();

		Result query(const std::string& __query_str);
		Result query(const std::string& __query_str, const std::vector<std::string>& __values);
		void query(const std::string& __query_str, const std::function<void(const std::exception *err, const Result &res)>& __callback);
		void query(const std::string& __query_str, const std::vector<std::string>& __values, const std::function<void(const std::exception *err, const Result &res)>& __callback);

	};
}

#endif //PQPP_CLIENT_HPP
