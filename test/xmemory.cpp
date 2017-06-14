#include <xmemory>

#include "test.hpp"

struct InstanceCounter {
	int* m_count;

	InstanceCounter(int* count) :
		m_count(count)
	{
		++*m_count;
	}
	~InstanceCounter() {
		--*m_count;
	}
};

using namespace stx;

static
void test_owned() {
	int count = 0;

	{
		owned<InstanceCounter> a(new InstanceCounter(&count));
	}
	test(count == 0);

	count = 0;
	{
		owned<InstanceCounter> a(new InstanceCounter(&count));
		auto b = std::move(a);
		test(!a);
		test(b);
	}
	test(count == 0);

	count = 0;
	{
		auto o = own(new InstanceCounter(&count));
		test(count == 1);
		auto o2 = new_owned<InstanceCounter>(&count);
		test(count == 2);
	}
	test(count == 0);
}

static
void test_shared() {
	int count = 0;
	{
		auto s = share(new InstanceCounter(&count));
		test(s);
		test(count == 1);
	}
	test(count == 0);

	count = 0;
	{
		auto s = new_shared<InstanceCounter>(&count);
		test(s);
		test(count == 1);
	}
	test(count == 0);

	count = 0;
	{
		auto s = new_shared<InstanceCounter>(&count);
		auto s2 = s;
		test(s);
		test(s2);
		test(s.get_block()->shared_refs() == 2);
	}

	count = 0;
	{
		auto s = new_shared<InstanceCounter>(&count);
		auto s2 = std::move(s);
		test(!s);
		test(s2);
		test(s2.get_block());
		test(s2.get_block()->shared_refs() == 1);
	}
}

static
void test_weak_shared() {
	int count = 0;

	{
		weak<InstanceCounter> w;
		test(!w);
		{
			auto s = share(new InstanceCounter(&count));
			w = s;
			test(w);
			test(w.lock() == s.get());
			test(s.get_block()->weak_refs() == 1);
		}
		test(!w);
		test(w.lock() == nullptr);
	}

	{
		weak<InstanceCounter> w;
		test(!w);
		{
			auto s = share(new InstanceCounter(&count));
			w = s;
			test(w);
			test(w.lock() == s.get());
		}
		test(!w);
		test(w.lock() == nullptr);
	}
}

void test_xmemory() {
	test_owned();
	test_shared();
	test_weak_shared();
}
