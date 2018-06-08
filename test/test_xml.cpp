#include "test.hpp"
#include "../xml.hpp"

using namespace stx;
using namespace stx::xml;

static
void test_xml_simple() {
	const char* source = R"(
		<outer>
			<!-- comment -->
			<inner1/>
			<inner2>
			</inner2>
			<inner3 name='nice name' type="awesome type"/>
			<inner4 name='nice name' type="awesome type">
			</inner4>
		</outer>
	)";

	node doc;
	arena_allocator alloc;
	doc.parse_document(alloc, source);

	node
		*outer,
		*comment,
		*inner1,
		*inner2,
		*inner3,
		*inner4;

	test(outer = doc.children());
	test(!outer->next());
	test(!outer->prev());
	test(outer->type() == node::regular_node);
	test(!outer->attributes());

	test(comment = outer->children());
	test(inner1 = comment->next());
	test(inner2 = inner1->next());
	test(inner3 = inner2->next());
	test(inner4 = inner3->next());
	test(!inner4->next());
}

void test_xml() {
	test_xml_simple();
}
