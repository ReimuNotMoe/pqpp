/*
    This file is part of pqpp.

    Copyright (C) 2019 ReimuNotMoe

    This program is free software: you can redistribute it and/or modify
    it under the terms of the MIT License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <iostream>

#include "pqpp.hpp"

int main() {

	boost::asio::io_service iosvc;

	pqpp::Pool pool(iosvc);

	pool.config([](auto &cfg){
		cfg.user = "root";
		cfg.io_timeout = 100;
		cfg.queue_timeout = 2;
		cfg.pool_size = 1;
	});

	// Coroutine 1
	boost::asio::spawn(iosvc, [&](boost::asio::yield_context yield_ctx){
		auto cl = pool.client(yield_ctx);

		std::cout << "coroutine1: attempting to get client from pool\n";

		pqpp::ClientContainer c;
		try {
			c = cl.get();
		} catch (std::exception &e) {
			std::cout << "coroutine1: whoa! " << e.what() << "\n";
			return;
		}

		std::cout << "coroutine1: got connected client!\n";

		try {
			auto res = c.client->query("SELECT * FROM public.test;");

			std::cout << "coroutine1: There are " << res.rows.size() << " rows.\n\n";

			for (auto &it_r : res.rows) {
				std::cout << "coroutine1: ";
				for (auto &it_c : it_r) {
					std::cout << it_c << " ";
				}
				std::cout << "\n";
			}
		} catch (std::exception &e) {
			std::cout << "coroutine1: whoa! " << e.what() << "\n";
		}

	});

	// Coroutine 2, callback style
	boost::asio::spawn(iosvc, [&](boost::asio::yield_context yield_ctx){
		auto cl = pool.client(yield_ctx);

		std::cout << "coroutine2: attempting to get client from pool\n";
		cl.get([](auto err, auto client){
			if (err) {
				std::cout << "coroutine2: whoa! " << err->what() << "\n";
				return;
			}

			std::cout << "coroutine2: got connected client!\n";

			client->query("SELECT * FROM public.test;", [](auto err, auto &res){
				if (err) {
					std::cout << "coroutine2: whoa! " << err->what() << "\n";
					return;
				}

				std::cout << "coroutine2: There are " << res.rows.size() << " rows.\n\n";

				for (auto &it_r : res.rows) {
					std::cout << "coroutine2: ";
					for (auto &it_c : it_r) {
						std::cout << it_c << " ";
					}
					std::cout << "\n";
				}
			});


		});
	});

	// Coroutine 3, direct query
	boost::asio::spawn(iosvc, [&](boost::asio::yield_context yield_ctx){
		auto cl = pool.client(yield_ctx);

		std::cout << "coroutine3: attempting to get client from pool\n";
		cl.get([](auto err, auto client){
			if (err) {
				std::cout << "coroutine3: whoa! " << err->what() << "\n";
				return;
			}

			std::cout << "coroutine3: got connected client!\n";

			client->query("SELECT * FROM public.test;", [](auto err, auto &res){
				if (err) {
					std::cout << "coroutine3: whoa! " << err->what() << "\n";
					return;
				}

				std::cout << "coroutine3: There are " << res.rows.size() << " rows.\n\n";

				for (auto &it_r : res.rows) {
					std::cout << "coroutine3: ";
					for (auto &it_c : it_r) {
						std::cout << it_c << " ";
					}
					std::cout << "\n";
				}
			});


		});
	});

	iosvc.run();

}