/*
    This file is part of pqpp.

    Copyright (C) 2019 ReimuNotMoe

    This program is free software: you can redistribute it and/or modify
    it under the terms of the MIT License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include "Result.hpp"

using namespace pqpp;

void Result::assign(PGresult *__result_ptr) {
	__pgresult_ptr = __result_ptr;
	auto &r = __pgresult_ptr;

	int ncols = PQnfields(r);
	int nrows = PQntuples(r);

	for (int j = 0; j < ncols; j++) {
		Field f{
			.name = PQfname(r, j)
		};

		__fields.push_back(std::move(f));
	}


	for (int i = 0; i < nrows; i++) {
		std::vector<std::string_view> this_row;
		for (int j = 0; j < ncols; j++) {
			char *pdata = PQgetvalue(r, i, j);
			int size = PQgetlength(r, i, j);
			this_row.emplace_back(pdata, (size_t)size);
		}
		__rows.push_back(std::move(this_row));
	}
}
