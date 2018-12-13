#include "catch.hpp"
#include <stx/system.hpp>

using namespace stx;

class test_system final : public system {
public:
	bool added = false, configured = false, enabled = false;

	void sysAdded    (system_configuration&) { added = true; }
	void sysConfigure(system_configuration&) { configured = true; }
	void sysEnable   (system_manager&) { enabled = true; }
	void sysUpdate   (system_manager&) {}
	void sysDisable  (system_manager&) { enabled = false; }
	void sysRemoved  () { added = false; configured = false; }
};

TEST_CASE("System manager", "[system]") {
	system_manager systems;

	SECTION("Group ids count up") {
		REQUIRE(systems.groupMask({}).none());
		REQUIRE(systems.groupMask({"a"}).any());
		REQUIRE(systems.groupId("a") != systems.groupId("not_a"));
		REQUIRE(systems.groupMask({"a"}) != systems.groupMask({"not_a"}));
	}

	SECTION("Enable and disable works") {
		auto a = std::make_shared<test_system>();

		systems.add("a", {"a"}, {"not_a"}, a);

		// TODO: disable by name

		CHECK(a->added);
		CHECK(a->configured);
		CHECK(!a->enabled);

		systems.enable("a");
		CHECK(a->enabled);

		systems.enable("not_a");
		CHECK(!a->enabled);

		systems.disable("not_a");
		CHECK(a->enabled);

		systems.disable("a");
		CHECK(!a->enabled);
	}
}
