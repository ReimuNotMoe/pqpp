/*
    This file is part of pqpp.

    Copyright (C) 2019 ReimuNotMoe

    This program is free software: you can redistribute it and/or modify
    it under the terms of the MIT License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef PQPP_EXCEPTION_HPP
#define PQPP_EXCEPTION_HPP

#include "CommonIncludes.hpp"

namespace pqpp {
	class Exception : public std::exception {
	protected:
		std::string desc;
	public:
		explicit Exception(const std::string &__desc) : std::exception() {
			desc = __desc;
		}

		Exception(PGconn *__conn, const std::string &__desc) : std::exception() {
			desc = __desc;

			if (__conn) {
				desc += ": ";
				desc += PQerrorMessage(__conn);
			}
		}

		const char *what() const noexcept override {
			return desc.c_str();
		}
	};
}

#endif //PQPP_EXCEPTION_HPP
