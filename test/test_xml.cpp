#include "test.hpp"
#include "../xml.hpp"

using namespace stx;
using namespace stx::xml;

static
void test_xml_simple() {
	parser p;
	node
		*outer,
		*inner1,
		*inner2,
		*inner3,
		*inner4;

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

	test(
		outer = p.parse(source)
	);

	node* n = outer;
	int depth = 0;
	while(n) {
		printf("%.*sN: %i %.*s\n", depth, "              ", n->type(), (int)n->name().size(), n->name().data());
		if(n->children()) {
			n = n->children();
			depth++;
		}
		else if(n->next()) {
			n = n->next();
		}
		else if(n->parent()) {
			n = n->parent()->next();
			depth--;
		}
	}

	test(outer);
	test(!outer->next());
	test(!outer->prev());
	test(outer->type() == node::regular_node);
	test(!outer->attributes());

	test(inner1 = outer->children());
	test(inner2 = inner1->next());
	test(inner3 = inner2->next());
	test(inner4 = inner3->next());
	test(!inner4->next());
}

void test_xml() {
	test_xml_simple();
}
