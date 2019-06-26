/*
    This file is part of pqpp.

    Copyright (C) 2019 ReimuNotMoe

    This program is free software: you can redistribute it and/or modify
    it under the terms of the MIT License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include "Client.hpp"

using namespace pqpp;

Client::Client(boost::asio::io_service &__io_svc, boost::asio::yield_context &__yield_ctx) : io_svc(__io_svc), yield_ctx(__yield_ctx), io_timeout_timer(__io_svc) {

}

Client::Client(boost::asio::io_service &__io_svc, boost::asio::yield_context &__yield_ctx,
	       struct client_config __config) : Client(__io_svc, __yield_ctx) {
	conf = std::move(__config);
}

Client::Client(boost::asio::io_service &__io_svc, boost::asio::yield_context &__yield_ctx,
	       const std::function<void(pqpp::client_config &)> &cfg_func) : Client(__io_svc, __yield_ctx) {
	cfg_func(conf);
}

Client::~Client() {
	if (conn)
		PQfinish(conn);
}

void Client::config_resolve_envvars() {
	auto &c = conf;

	if (!c.connection_string.empty())
		return;

	if (c.user.empty()) {
		char *p = getenv("PGUSER");
		if (p)
			c.user = p;
		else {
			throw pqpp::Exception("user not defined");
		}
	}

	if (c.password.empty()) {
		char *p = getenv("PGPASSWORD");
		if (p)
			c.password = p;
//		else
//			throw pqpp::Exception("password not defined");
	}

	if (c.database.empty()) {
		char *p = getenv("PGDATABASE");
		if (p)
			c.database = p;
//		else
//			throw pqpp::Exception("database not defined");
	}

	if (!c.port) {
		char *p = getenv("PGPORT");
		if (p)
			c.port = strtol(p, nullptr, 10);
//		else
//			throw pqpp::Exception("port not defined");
	}
}

void Client::do_connect() {
	config_resolve_envvars();

	if (conf.connection_string.empty()) {
		std::string portstr;
		if (conf.port)
			portstr = std::to_string(conf.port);

		const char *keys[6] = {"host", "user", "password", "database", "port", nullptr};
		const char *vals[6];
		vals[0] = conf.host.c_str();
		vals[1] = conf.user.c_str();
		vals[2] = conf.password.c_str();
		vals[3] = conf.database.c_str();
		vals[4] = portstr.c_str();
		vals[5] = nullptr;

		conn = PQconnectStartParams(keys, vals, 0);
	} else {
		conn = PQconnectStart(conf.connection_string.c_str());
	}

	if (!conn) {
		throw pqpp::Exception("can't allocate memory");
	}

	auto s = PQstatus(conn);

	if (s == CONNECTION_BAD) {
		throw pqpp::Exception(conn, "connection failed");
	}

	PQsetnonblocking(conn, 1);

	auto fd = PQsocket(conn);
	struct sockaddr sa;
	socklen_t len;
	if (getsockname(fd, &sa, &len))
		throw std::system_error(std::error_code(errno, std::system_category()), strerror(errno));

	if (sa.sa_family == AF_UNIX)
		unix_socket = std::make_unique<boost::asio::local::stream_protocol::socket>(
			io_svc, boost::asio::local::stream_protocol(), fd);
	else
		tcp_socket = std::make_unique<boost::asio::ip::tcp::socket>(
			io_svc, sa.sa_family == AF_INET ?
				boost::asio::ip::tcp::v4() : boost::asio::ip::tcp::v6(), fd);

	start_io_timer();

	while (true) {
		auto pstat = PQconnectPoll(conn);

		if (pstat == PGRES_POLLING_READING) {
			io_wait_readable();
		} else if (pstat == PGRES_POLLING_WRITING) {
			io_wait_writable();
		} else if (pstat == PGRES_POLLING_OK) {
			printf("done\n");
			break;
		} else if (pstat == PGRES_POLLING_FAILED) {
			throw pqpp::Exception(conn, "connection failed");
		}
	}
}

size_t Client::io_wait_readable() {
	printf("waiting read\n");
	io_timeout_timer.expires_from_now(io_timeout_duration);
	if (tcp_socket) {
		tcp_socket->async_read_some(boost::asio::null_buffers(), yield_ctx);
		return tcp_socket->available();
	} else {
		unix_socket->async_read_some(boost::asio::null_buffers(), yield_ctx);
		return unix_socket->available();
	}
}

void Client::io_wait_writable() {
	printf("waiting write\n");
	io_timeout_timer.expires_from_now(io_timeout_duration);
	if (tcp_socket) {
		tcp_socket->async_write_some(boost::asio::null_buffers(), yield_ctx);
	} else {
		unix_socket->async_write_some(boost::asio::null_buffers(), yield_ctx);
	}
}

void Client::do_query() {
	while (true) {
		auto rc = PQflush(conn);

		if (rc == 1) {
			io_wait_writable();
		} else if (rc == 0) {
			break;
		} else {
			throw pqpp::Exception(conn, "PQflush");
		}
	}

	while (true) {
		if (!PQconsumeInput(conn))
			throw pqpp::Exception(conn, "PQconsumeInput");

		if (PQisBusy(conn)) {
			io_wait_readable();
		} else {
			break;
		}

	}
}

void Client::config(const std::function<void(pqpp::client_config &)> &cfg_func) {
	cfg_func(conf);
}

void Client::config(struct client_config __config) {
	conf = std::move(__config);
}

void Client::connect(std::function<void(const std::exception *err)> __callback) {
	callback_connect = std::move(__callback);
	try {
		connect();
		callback_connect(nullptr);
	} catch (std::exception &e) {
		callback_connect(&e);
	}
}

void Client::connect() {
	do_connect();
}

Result Client::query(const std::string &__query_str) {
	if (!PQsendQuery(conn, __query_str.c_str())) {
		throw pqpp::Exception(conn, "PQsendQuery");
	}

	do_query();

	auto result = PQgetResult(conn);

	if (!result)
		throw pqpp::Exception(conn, "PQgetResult");

	auto status = PQresultStatus(result);

	if (status == PGRES_TUPLES_OK) {
		return Result(result);
	} else if (status == PGRES_COMMAND_OK) {
		PQclear(result);
	} else {
		PQclear(result);
		throw pqpp::Exception(conn, "PQresultStatus");
	}


	return {};
}

Result Client::query(const std::string &__query_str, const std::vector<std::string> &__values) {
	auto arr_size = __values.size();
	const char *params_str[arr_size];
	int params_len[arr_size];


	for (size_t i=0; i<arr_size; i++) {
		params_str[i] = __values[i].data();
		params_len[i] = (int)__values[i].size();
	}

	if (!PQsendQueryParams(conn, __query_str.c_str(), arr_size, nullptr, params_str, params_len,
			       nullptr, 0)) {
		throw pqpp::Exception(conn, "PQsendQuery");
	}

	do_query();

	auto result = PQgetResult(conn);

	if (!result)
		throw pqpp::Exception(conn, "PQgetResult");

	auto status = PQresultStatus(result);

	if (status == PGRES_TUPLES_OK) {
		return Result(result);
	} else if (status == PGRES_COMMAND_OK) {
		PQclear(result);
	} else {
		PQclear(result);
		throw pqpp::Exception(conn, "PQresultStatus");
	}


	return {};
}

void Client::query(const std::string &__query_str,
		   const std::function<void(const std::exception *err, const Result &res)> &__callback) {
	try {
		auto r = query(__query_str);
		__callback(nullptr, r);
	} catch (std::exception &e) {
		__callback(&e, {});
	}
}

void Client::query(const std::string &__query_str, const std::vector<std::string> &__values,
		   const std::function<void(const std::exception *err, const Result &res)> &__callback) {
	try {
		auto r = query(__query_str, __values);
		__callback(nullptr, r);
	} catch (std::exception &e) {
		__callback(&e, {});
	}
}



