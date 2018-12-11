#include "catch.hpp"

#include <stx/xml.hpp>

using namespace stx;
using namespace stx::xml;

TEST_CASE("Simple xml parser test", "[xml]") {
	const char* source = R"(
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
		</outer>
	)";

	node doc;
	arena_allocator alloc;
	doc.parse_document(alloc, source);

	node
		*outer,
		*commentA,
		*inner1,
		*inner2,
		*inner3,
		*inner4;

	outer = doc.children();
	CHECK(outer);
	CHECK(!outer->next());
	CHECK(!outer->prev());
	CHECK(outer->type() == node::regular);
	CHECK(!outer->attributes());

	commentA = outer->children();
	CHECK(commentA);
	inner1 = commentA->next();
	CHECK(inner1);
	inner2 = inner1->next();
	CHECK(inner2);
	inner3 = inner2->next();
	CHECK(inner3);
	inner4 = inner3->next();
	CHECK(inner4);
	CHECK(!inner4->next());

	node* content = inner4->children();
	REQUIRE(content);
	node* comment = content->next();
	REQUIRE(comment);

	CHECK(content->type() == node::content);
	CHECK(comment->type() == node::comment);
	CHECK(content->content_value() == "I am content");
	CHECK(comment->comment_value() == "I am a comment");
}
