#include "catch.hpp"
#include <stx/system.hpp>

using namespace stx;

class test_system : public system {
public:
	bool added = false, configured = false, enabled = false;

	void sysAdded    (system_configuration&) { added = true; }
	void sysConfigure(system_configuration&) { configured = true; }
	void sysEnable   (system_manager&) { enabled = true; }
	void sysUpdate   () {}
	void sysDisable  (system_manager&) { enabled = false; }
	void sysRemoved  () { added = false; configured = false; }
};

TEST_CASE("System manager", "[system]") {
	SECTION("Group ids count up") {
		system_manager systems;
		REQUIRE(systems.groupMask({}).none());
		REQUIRE(systems.groupMask({"a"}).any());
		REQUIRE(systems.groupId("a") != systems.groupId("not_a"));
		REQUIRE(systems.groupMask({"a"}) != systems.groupMask({"not_a"}));
	}

	SECTION("Enable and disable works") {
		system_manager systems;
		auto a = stx::make_shared<test_system>();

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

	SECTION("System manager disables systems and calls sysRemove") {
		class check_remove_system : public test_system {
		public:
			int& count;

			check_remove_system(int& c) : count(c) {
				count++;
			}

			~check_remove_system() {
				count--;
				REQUIRE(!enabled);
				REQUIRE(!added);
			}
		};

		int count = 0;
		{
			system_manager systems;
			systems.add("Thing", {"a"}, stx::make_shared<check_remove_system>(count));
			REQUIRE(count == 1);
			systems.enable("a");
		}
		REQUIRE(count == 0);
	}
}
