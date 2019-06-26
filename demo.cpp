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

#include "Client.hpp"

int main() {

	boost::asio::io_service iosvc;

	// Coroutine 1
	boost::asio::spawn(iosvc, [&](boost::asio::yield_context yield_ctx){
		pqpp::Client client(iosvc, yield_ctx);

		client.config({
				      .user = "root"
			      });

		try {
			client.connect();
		} catch (std::exception &e) {
			std::cout << "coroutine1: whoa! " << e.what() << "\n";
			return;
		}

		std::cout << "coroutine1: connected!\n";

		auto res = client.query("SELECT * FROM public.test;");

		std::cout << "coroutine1: There are " << res.rows.size() << " rows.\n\n";

		for (auto &it_r : res.rows) {
			std::cout << "coroutine1: ";
			for (auto &it_c : it_r) {
				std::cout << it_c << " ";
			}
			std::cout << "\n";
		}
	});

	// Coroutine 2, callback style
	boost::asio::spawn(iosvc, [&](boost::asio::yield_context yield_ctx){
		pqpp::Client client(iosvc, yield_ctx);

		client.config([](auto &cfg){
			cfg.user = "root";
			cfg.statement_timeout = 100;
		});

		client.connect([&](auto e) {
			if (e) {
				std::cout << "coroutine2: whoa! " << e->what() << "\n";
				return;
			} else
				std::cout << "coroutine2: connected!\n";

			client.query("SELECT * FROM public.test;", [](auto err, auto &res){
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

	iosvc.run();

}