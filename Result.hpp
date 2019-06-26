/*
    This file is part of pqpp.

    Copyright (C) 2019 ReimuNotMoe

    This program is free software: you can redistribute it and/or modify
    it under the terms of the MIT License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef PQPP_RESULT_HPP
#define PQPP_RESULT_HPP

#include "CommonIncludes.hpp"
#include "Exception.hpp"

namespace pqpp {
	class Result {
	public:
		struct Field {
			std::string_view name;
		};

	private:
		std::vector<std::vector<std::string_view>> __rows;
		std::vector<Field> __fields;
		PGresult *__pgresult_ptr = nullptr;

	public:
		const std::vector<std::vector<std::string_view>>& rows;
		const std::vector<Field>& fields;

		Result() : rows(__rows), fields(__fields) {

		}

		virtual ~Result() {
			if (__pgresult_ptr)
				PQclear(__pgresult_ptr);
		}

		explicit Result(PGresult *__result_ptr) : Result() {
			assign(__result_ptr);
		}

		void assign(PGresult *__result_ptr);

		size_t row_count() {
			return rows.size();
		}

	};
}

#endif //PQPP_RESULT_HPP
