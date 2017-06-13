#include <xhandle>

#include "test.hpp"

using namespace stx;

static void test_pointer_pair() {
	struct twin : public pointer_pair<twin> {};

	twin a;
	twin c;

	test(!a.get());

	{
		twin b;
		a.reset(&b);
		test(b.get() == &a);
		test(a.get() == &b);
		test(c.get() == nullptr);
	}

	test(a.get() == nullptr);

	{
		twin b;
		a.reset(&b);
		c = std::move(a);
		test(a.get() == nullptr);
		test(c.get() == &b);
		test(b.get() == &c);
	}

	{
		twin b;
		a.reset(&b);
		c = std::move(b);
		test(b.get() == nullptr);
		test(c.get() == &a);
		test(a.get() == &c);
	}
}

void test_handles() {
	struct ressource : public handle_socket {
		int& e;
		bool destroyed;

		ressource(int& ee) : e(ee), destroyed(false) { ++e; }

		~ressource() { on_handle_destroyed(); }

		void on_handle_destroyed() noexcept override {
			if(!destroyed) {
				destroyed = true;
				--e;
			}
		}
	};

	int counter = 0;

	handle h1;

	{
		ressource r(counter);
		test(counter == 1);

		h1.reset(&r);
		test(r.get() == &h1);
		test(h1.get() == &r);
	}

	test(h1.get() == nullptr);
	test(counter == 0);

	counter = 0;
	{
		ressource r(counter);
		test(counter == 1);

		{
			handle h2(&r);
			test(h2.get() == &r);
			test(counter == 1);
		}

		test(counter == 0);
	}
	test(counter == 0);
}

void test_xhandle() {
	test_pointer_pair();
	test_handles();
}
