#include "catch.hpp"

#include <stx/config.hpp>

#include <sstream>

using namespace stx;

TEST_CASE("Test config", "[config]") {
	stx::config cfg;

	cfg.set("does exist", "42");

	std::string result = "nothing happened";

	SECTION("Get string optional") {
		CHECK(!cfg.get("doesn't exist", &result));
		CHECK(result == "nothing happened");

		CHECK(cfg.get("doesn't exist", "fallback") == "fallback");

		CHECK(cfg.get("does exist", &result));
		CHECK(result == "42");
	}

	SECTION("Get string required") {
		CHECK_THROWS(cfg.get("doesn't exist"));

		CHECK_NOTHROW(cfg.get("does exist"));
		CHECK(cfg.get("does exist") == "42");
	}

	SECTION("Get string fallback") {
		CHECK(cfg.get("doesn't exist", "fallback") == "fallback");
		CHECK(cfg.get("does exist", "fallback") == "42");
	}
}

TEST_CASE("Test config ini file loading", "[config]") {
	stx::config cfg;

	std::stringstream stream;

	stream << R"(
		thing=ding

		[NewSection]
		thing2=ding2
		thing3=ding3; This comment should be ignored.
	)";

	REQUIRE_NOTHROW(cfg.parseIni(stream));

	CHECK(cfg.get("thing") == "ding");
	CHECK(cfg.get("NewSection.thing2") == "ding2");
	CHECK(cfg.get("NewSection.thing3") == "ding3");

}

TEST_CASE("Test cmd argument parsing", "[config]") {
	stx::config cfg;

	const char* args[] = {
		"--meatballs",
		"--tasty=absolutely",
		"not_parsed"
	};

	REQUIRE_NOTHROW(cfg.parseCmd(std::size(args), args));

	CHECK(cfg.get("meatballs", false));
	CHECK(cfg.get("tasty") == "absolutely");
	CHECK(cfg.get("not_parsed", 1.1) == 1.1);
}