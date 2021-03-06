#include "catch.hpp"

#include <stx/xml.hpp>

using namespace stx;
using namespace stx::xml;

TEST_CASE("Simple xml parser test", "[xml]") {
	const char* source = R"(
		<?xml version="1.0" encoding="UTF-8" standalone="no"?>
		<outer>
			<!-- comment -->
			<inner1/>
			<inner2>
			</inner2>
			<inner3 name='nice name' type="awesome type"/>
			<inner4 name='nice name' type="awesome type">
				I am content
				<!-- I am a comment -->
			</inner4>
			<inner5>I am squished content</inner5>
		</outer>
	)";

	node doc;
	arena_allocator alloc;
	try {
		doc.parse_document(alloc, source);
	}
	catch(stx::parsing::errors::parsing_error& e) {
		e.diagnose("c_str", source);
		throw;
	}

	node
		*outer,
		*commentA,
		*inner1,
		*inner2,
		*inner3,
		*inner4,
		*inner5;

	CHECK(doc.children()->type() == node::processing_instruction);

	outer = doc.child(node::regular);
	CHECK(outer);
	CHECK(!outer->next());
	CHECK(outer->type() == node::regular);
	CHECK(!outer->attributes());

	commentA = outer->children();
	CHECK(commentA);
	inner1 = commentA->next();
	CHECK(inner1);
	CHECK(inner1->name() == "inner1");
	inner2 = inner1->next();
	CHECK(inner2);
	CHECK(inner2->name() == "inner2");
	inner3 = inner2->next();
	CHECK(inner3);
	CHECK(inner3->name() == "inner3");
	CHECK(inner3->req_attrib("name").value() == "nice name");
	CHECK(inner3->req_attrib("type").value() == "awesome type");
	inner4 = inner3->next();
	CHECK(inner4);
	inner5 = inner4->next();
	CHECK(inner5);
	CHECK(inner5->child(node::content)->content_value() == "I am squished content");
	CHECK(!inner5->next());

	node* content = inner4->children();
	REQUIRE(content);
	node* comment = content->next();
	REQUIRE(comment);

	CHECK(content->type() == node::content);
	CHECK(comment->type() == node::comment);
	CHECK(content->content_value() == "I am content");
	CHECK(comment->comment_value() == "I am a comment");
}

TEST_CASE("Test nasty text in xml") {
	node document;
	arena_allocator alloc;

	REQUIRE_NOTHROW(
		document.parse_document(alloc, R"(
			<frag>
				in  float random;
				out vec3  color;

				void main() {
					if(random < .5) discard;
					color = vec3(1, 1, 1);
				}
			</frag>
		)")
	);

	REQUIRE(document.children()->name() == "frag");

	node* frag = document.child("frag");
	REQUIRE(frag);
	REQUIRE(frag->children());
	REQUIRE(!frag->children()->next());

	REQUIRE(frag->children()->type() == node::content);
	REQUIRE(frag->children()->content_value().find("random < .5") != std::string_view::npos);
}
