#include "test.hpp"

#include "../database.hpp"
#include "../random.hpp"

using namespace stx;

void test_xdatabase_sqlite() {
	std::string db_name = "/tmp/stx_db_" + std::to_string(stx::rand<uint16_t>());

	database db = stx::database::sqlite();
	test(db.open(db_name, db_exceptions));
	if(!db.is_open()) return;
}
