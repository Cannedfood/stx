#include "catch.hpp"

#include "test_helpers/counted.hpp"

#include <stx/shared.hpp>

using namespace stx;

TEST_CASE("make_shared and simple destruct") {
	int count = 0;

	{
		shared<counted> ptr = make_shared<counted>(count);
		REQUIRE(ptr.refcount() == 1);
		REQUIRE(count == 1);
	}
	REQUIRE(count == 0);
}

TEST_CASE("Shared pointer copy", "[shared_ptr]") {
	int count = 0;

	{
		shared<counted> a = make_shared<counted>(count);
		CHECK(count == 1);
		REQUIRE(a.refcount() == 1);
		{
			shared<counted> b = make_shared<counted>(count);
			REQUIRE(count == 2);
			REQUIRE(b.refcount() == 1);
			b = a;
			CHECK(a.get() == b.get());
			CHECK(a.refcount() == 2);
			CHECK(b.refcount() == 2);
			CHECK(count == 1);
		}
		REQUIRE(a.refcount() == 1);
	}
	REQUIRE(count == 0);
}

TEST_CASE("Shared pointer move", "[shared_ptr]") {
	int count = 0;

	{
		shared<counted> a = make_shared<counted>(count);
		REQUIRE(count == 1);
		REQUIRE(a.refcount() == 1);
		{
			shared<counted> b = make_shared<counted>(count);
			shared<counted> c = a;
			REQUIRE(count == 2);
			REQUIRE(a.refcount() == 2);
			REQUIRE(b.refcount() == 1);

			{
				auto tmp = std::move(c);
				c = std::move(b);
				REQUIRE(!b.get());
				REQUIRE(!b);
				REQUIRE(c);
				REQUIRE(c.get());
				b = std::move(tmp);
			}
			REQUIRE(b.get() == a.get());
			REQUIRE(a.refcount() == 2);
			REQUIRE(c.refcount() == 1);
		}
		REQUIRE(a.refcount() == 1);
	}
	REQUIRE(count == 0);
}

TEST_CASE("Shared pointer override", "[shared_ptr]") {
	int count = 0;
	shared<counted> a = make_shared<counted>(count);
	shared<counted> b = make_shared<counted>(count);
	REQUIRE(count == 2);
	a = b;
	REQUIRE(count == 1);
	b = make_shared<counted>(count);
	REQUIRE(count == 2);
	a = std::move(b);
	REQUIRE(count == 1);
}

TEST_CASE("Shared pointer self-assign", "[shared_ptr]") {
	int count = 0;
	shared<counted> a = make_shared<counted>(count);
	REQUIRE(count == 1);
	a = a;
	REQUIRE(count == 1);
	a = std::move(a);
	REQUIRE(count == 1);
}

TEST_CASE("Weak pointer self-assign", "[shared_ptr]") {
	weak<int> w = make_shared<int>(0);
	REQUIRE(w.weak_refcount() == 1);
	w = w;
	REQUIRE(w.weak_refcount() == 0); // Shouldn't increment, no strong refs left
}

TEST_CASE("Basic weak pointer", "[shared_ptr]") {
	int count = 0;

	shared<counted> a = make_shared<counted>(count);
	REQUIRE(count == 1);
	{
		weak<counted> b = a;
		REQUIRE(a.refcount() == 1);
		REQUIRE(a.weak_refcount() == 1);

		weak<counted> c = b;
		REQUIRE(a.weak_refcount() == 2);

		shared<counted> d = c.lock();
		REQUIRE(a.refcount() == 2);
	}
}

TEST_CASE("Weak pointer destroyed after last shared", "[shared_ptr]") {
	int count = 0;
	{
		shared<counted> a = make_shared<counted>(count);
		{
			weak<counted> b = a;
			a.reset();
			REQUIRE(b.refcount() == 0);
			REQUIRE(b.weak_refcount() == 1);
		}
	}
}

TEST_CASE("Weak pointer locked after last shared was destroyed", "[shared_ptr]") {
	int count = 0;
	weak<counted> a;
	{
		shared<counted> b = make_shared<counted>(count);
		a = b;
	}
	REQUIRE(a.refcount() == 0);
	REQUIRE(a.weak_refcount() == 1);
	REQUIRE(a.lock() == nullptr);
}

struct Refcounted : public counted, public stx::enable_shared_from_this<Refcounted> {
	using counted::counted;
};

TEST_CASE("Test enable_shared_from_this", "[shared_ptr]") {
	int count = 0;

	shared<Refcounted> a = make_shared<Refcounted>(count);
	{
		shared<Refcounted> b = make_shared<Refcounted>(count);
	}
}

TEST_CASE("Test weak to self", "[shared_ptr]") {
	// If a weak pointer is destroyed while in the destructor the object, the shared_block may be deleted after we leave, this checks if this is prevented.
	struct FooBar {
		weak<FooBar> self;
	};

	shared<FooBar> bar = make_shared<FooBar>();
	bar->self = bar;

	bar.reset(); // This should SEGFAULT otherwise
}

TEST_CASE("Test enable_shared_from_this on the stack", "[shared_ptr]") {
	int count = 0;

	Refcounted a(count);
	{
		shared<Refcounted> r = a.shared_from_this();
		REQUIRE(r.get() == &a);
	}
	{
		shared<Refcounted> r = a.shared_from_this();
		REQUIRE(r.get() == &a);
		weak<Refcounted> w = a.weak_from_this();
	}
	{
		shared<Refcounted> r = a.shared_from_this();
		REQUIRE(r.get() == &a);
		weak<Refcounted> w = a.weak_from_this();
	}
}
