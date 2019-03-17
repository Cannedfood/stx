#include "catch.hpp"

#include <stx/json.hpp>

using namespace stx;

enum json_event {
	ARRAY_START = 0,
	ARRAY_END   = 1,

	OBJECT_START = 2,
	KEY = 3,
	OBJECT_END = 4,

	NUMBER = 5,
	STRING = 6
};

struct test_document_handler : json::document_handler {
	std::vector<json_event> events;
	std::vector<std::string_view> strs;

    void v(std::string_view s) {
        strs.push_back(s);
        // printf(">%s<\n", std::string(s).c_str());
    }

	void beginObject()               override { events.push_back(OBJECT_START); }
	void key(std::string_view value) override { v(value); events.push_back(KEY); }
	void endObject()                 override { events.push_back(OBJECT_END); }

	void beginArray() override { events.push_back(ARRAY_START); }
	void endArray()   override { events.push_back(ARRAY_END); }

	void number(std::string_view value) override { v(value); events.push_back(NUMBER); }
	void string(std::string_view value) override { v(value); events.push_back(STRING); }
};

TEST_CASE("Parsing correct json works", "[json]") {
    const char* text = R"(
        {
            "things": {
                "brush": "grr",
                "pi": 3.1415,
                "numbers": [ 1, 2e7 ]
            }
        }
    )";

	test_document_handler expected;
	expected.beginObject();
		expected.key("things");
			expected.beginObject();
				expected.key("brush"); expected.string("grr");
				expected.key("pi"); expected.number("3.1415");
				expected.key("numbers");
					expected.beginArray();
						expected.number("1");
						expected.number("2e7");
					expected.endArray();
			expected.endObject();
	expected.endObject();

	test_document_handler actual;
	json::parse(actual, text);

	REQUIRE(actual.events == expected.events);
	REQUIRE(actual.strs   == expected.strs);
}
