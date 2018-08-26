#include "catch.hpp"

#include "../shared_ptr.hpp"

using namespace stx;

struct Counted {
	int& counter;
	Counted(int& c) : counter(c) { ++counter; }
	~Counted() { --counter; }
};

TEST_CASE("make_shared and simple destruct") {
	int count = 0;

	{
		shared<Counted> ptr = make_shared<Counted>(count);
		REQUIRE(ptr.refcount() == 1);
		REQUIRE(count == 1);
	}
	REQUIRE(count == 0);
}

TEST_CASE("Shared pointer copy", "[shared_ptr]") {
	int count = 0;

	{
		shared<Counted> a = make_shared<Counted>(count);
		CHECK(count == 1);
		REQUIRE(a.refcount() == 1);
		{
			shared<Counted> b = make_shared<Counted>(count);
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
		shared<Counted> a = make_shared<Counted>(count);
		REQUIRE(count == 1);
		REQUIRE(a.refcount() == 1);
		{
			shared<Counted> b = make_shared<Counted>(count);
			shared<Counted> c = a;
			REQUIRE(count == 2);
			REQUIRE(a.refcount() == 2);
			REQUIRE(b.refcount() == 1);

			{
				auto tmp = std::move(c);
				c = std::move(b);
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

TEST_CASE("Basic weak pointer", "[shared_ptr]") {
	int count = 0;
	{
		shared<Counted> a = make_shared<Counted>(count);
		REQUIRE(count == 1);
		{
			weak<Counted> b = a;
			REQUIRE(a.refcount() == 1);
			REQUIRE(a.weak_refcount() == 1);

			weak<Counted> c = b;
			REQUIRE(a.weak_refcount() == 2);

			shared<Counted> d = c.lock();
			REQUIRE(a.refcount() == 2);
		}
	}
}

struct Refcounted : public Counted, public stx::enable_shared_from_this<Refcounted> {
	using Counted::Counted;
};

TEST_CASE("Test enable_shared_from_this", "[shared_ptr]") {
	/*
	int count = 0;

	shared<Refcounted> a = make_shared<Refcounted>(count);
	{
		shared<Refcounted> b = make_shared<Refcounted>(count);
	}
	*/
}

TEST_CASE("Test enable_shared_from_this on the stack", "[shared_ptr]") {
	int count = 0;

	Refcounted a(count);
	{
		shared<Refcounted> r = a.shared_from_this();
		REQUIRE(r.get() == &a);
		REQUIRE(r.refcount() == 2);
	}
	{
		shared<Refcounted> r = a.shared_from_this();
		REQUIRE(r.get() == &a);
		REQUIRE(r.refcount() == 2);
		weak<Refcounted> w = a.weak_from_this();
		REQUIRE(r.weak_refcount() == 1);
	}
	{
		shared<Refcounted> r = a.shared_from_this();
		REQUIRE(r.get() == &a);
		REQUIRE(r.refcount() == 2);
		weak<Refcounted> w = a.weak_from_this();
		REQUIRE(r.weak_refcount() == 1);
	}
}
