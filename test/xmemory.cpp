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
}

static
void test_shared() {

}

static
void test_weak_shared() {

}

void test_xmemory() {
	test_owned();
	test_shared();
	test_weak_shared();
}
