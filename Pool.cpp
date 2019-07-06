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

Pool::Pool(boost::asio::io_service &__io_svc) : io_service(__io_svc), queue_timer(io_service) {
	ctx = std::make_shared<pool_ctx>(conf, queue, queued_size, queue_timeout, queue_timer);
}

Pool::Pool(boost::asio::io_service &__io_svc, struct client_config __config) : Pool(__io_svc) {
	config(std::move(__config));
}

Pool::Pool(boost::asio::io_service &__io_svc, const std::function<void(pqpp::client_config &)> &cfg_func) : Pool(__io_svc) {
	config(cfg_func);
}


void Pool::config(std::shared_ptr<client_config>& __config) {
	conf = __config;

	setup_queue_timer();
}

void Pool::config(const std::function<void(pqpp::client_config &)> &cfg_func) {
	if (!conf)
		conf = std::make_shared<client_config>();

	cfg_func(*conf);

	setup_queue_timer();
}

void Pool::config(struct client_config __config) {
	if (!conf)
		conf = std::make_shared<client_config>(__config);

	setup_queue_timer();
}

void Pool::setup_queue_timer() {
	if (conf->queue_timeout != -1) {
		queue_timeout = std::chrono::seconds(conf->queue_timeout);
	} else {
		queue_timeout = std::chrono::seconds(INT32_MAX);
	}
}

PoolClient Pool::client(boost::asio::yield_context &__yield_ctx) {
	return PoolClient(*this, __yield_ctx);
}

void Pool::give_back(std::shared_ptr<Client> __client) {
	queue.push_back(std::move(__client));
	queued_size--;
	queue_timer.cancel_one();
}





PoolClient::PoolClient(Pool &__parent_pool, boost::asio::yield_context &__yield_ctx) :
	parent_pool(static_cast<PoolExposed &>(__parent_pool)), io_service(parent_pool.io_service),
	yield_ctx(__yield_ctx), queue_timer(parent_pool.queue_timer), queue_timeout(parent_pool.queue_timeout) {

}

ClientContainer PoolClient::get() {
	ClientContainer ret;

	auto &queue = parent_pool.queue;
	auto &queued_size = parent_pool.queued_size;
	auto &conf = parent_pool.conf;
	auto &io_svc = parent_pool.io_service;

	if (conf->pool_size > 0) {
		while (queued_size >= conf->pool_size) {
			if (conf->queue_timeout) {
				boost::system::error_code ec;

				puts("waiting pool");
				queue_timer.expires_from_now(queue_timeout);
				queue_timer.async_wait(yield_ctx[ec]);

				if (ec == boost::asio::error::operation_aborted) {
					break;
				} else {
					throw std::system_error(EBUSY, std::generic_category(), "pool is full");
				}
			} else {
				throw std::system_error(EBUSY, std::generic_category(), "pool is full");
			}
		}
	}

	puts("got pool");

	queued_size++;


	if (queue.empty()) {
		ret.client = std::make_shared<Client>(io_svc, nullptr, parent_pool.ctx);
	} else {
		ret.client = queue.front();
		queue.pop_front();
	}

	ret.client->assign_yield_context(&yield_ctx);

	if (!ret.client->connected())
		ret.client->connect();

	return ret;
}

void PoolClient::get(const std::function<void(const std::exception *, Client *)> &__callback) {
	try {
		__callback(nullptr, get().client.get());
	} catch (std::exception &e) {
		__callback(&e, nullptr);
	}

}

Result PoolClient::query(const std::string &__query_str) {
	return get().client->query(__query_str);
}

Result PoolClient::query(const std::string &__query_str, const std::vector<std::string> &__values) {
	return get().client->query(__query_str, __values);
}

void PoolClient::query(const std::string &__query_str,
		       const std::function<void(const std::exception *, const pqpp::Result &)> &__callback) {
	try {
		get().client->query(__query_str, __callback);
	} catch (std::exception &e) {
		__callback(&e, {});
	}
}

void PoolClient::query(const std::string &__query_str, const std::vector<std::string> &__values,
		       const std::function<void(const std::exception *, const pqpp::Result &)> &__callback) {
	try {
		get().client->query(__query_str, __values, __callback);
	} catch (std::exception &e) {
		__callback(&e, {});
	}
}

pool_ctx::pool_ctx(std::shared_ptr<client_config> &__conf, std::deque<std::shared_ptr<Client>> &__queue,
		   size_t &__queued_size, std::chrono::seconds &__queue_timeout,
		   boost::asio::system_timer &__queue_timer) : conf(__conf),
							       queue(__queue), queued_size(__queued_size), queue_timeout(__queue_timeout), queue_timer(__queue_timer) {

}

void pool_ctx::give_back(std::shared_ptr<Client> __client) {
	queue.push_back(std::move(__client));
	queued_size--;
	queue_timer.cancel_one();
	puts("released pool");
}