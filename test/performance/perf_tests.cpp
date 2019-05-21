#include <stx/entities.hpp>
#include <stx/system.hpp>

#include <stx/random.hpp>
#include <stx/timer.hpp>

#include <stx/gc.hpp>

void perfEntities() {
	struct position { float x, y; };
	struct velocity { float x, y; };
	struct color    { float r, g, b, a; };
	struct circle   { float radius; };
	struct square   { float size; };

	stx::timer t;
	stx::random rnd;

	stx::entities entities;

	puts("Entities performance:");

	t.reset();
	for(size_t i = 0; i < 100000; i++) {
		position pos = { rnd.get(0.f, 100.f), rnd.get(0.f, 100.f) };
		color    clr = { rnd.get(1.f), rnd.get(1.f), rnd.get(1.f), rnd.get(1.f) };
		velocity vel = { .1f, .1f };

		if(rnd.get(0, 1) == 0) {
			entities.create(pos, vel, clr, square { rnd.get(1.f) });
		}
		else {
			entities.create(pos, vel, clr, square { rnd.get(1.f) });
		}
	}
	printf("\tCreating took %s\n", t.to_string().c_str());


	double dt    = .1;
	for (size_t i = 0; i < 1000; i++) {
		for(auto [pos, vel] : entities.filter<position, velocity>()) {
			pos.x += dt * vel.x;
			pos.y += dt * vel.y;
		}

		for(auto [pos, col, circ] : entities.filter<position, color, circle>()) {
			circ.radius /= pos.x * col.r;
		}

		for(auto [pos, col, sqr] : entities.filter<position, color, square>()) {
			sqr.size /= pos.x * col.r;
		}
	}

	printf("\tIteration took %s\n", t.to_string().c_str());
	puts("");
}

void perfGC() {
	using namespace stx;

	struct thing { gc<thing> value; };

	puts("GC performance:");

	stx::random rnd;
	stx::timer t;

	constexpr size_t N = 20000;

	for (size_t i = 1; i <= 3; i++)
	{
		garbage_collector::LEAK_ALL();

		printf("\tRound %zu\n", i);

		std::vector<gc<thing>> things(N);

		t.reset();
		for(auto& e : things) e = make_gc<thing>();
		printf("\t\tTook %s to create %zu elements\n", t.to_string().c_str(), N);

		t.reset();
		for(auto& e : things) e->value = rnd.get(things.data(), things.size());
		printf("\t\tTook %s to randomly link %zu elements\n", t.to_string().c_str(), N);

		t.reset();
		things.clear();
		printf("\t\tTook %s to remove %zu references\n", t.to_string().c_str(), N);

		t.reset();
		garbage_collector::mark_and_sweep();
		printf("\t\tTook %s to collect graph with %zu objects with each a random ref to another\n", t.to_string().c_str(), N);

		puts("");
	}
}

int main() {
	perfEntities();
	perfGC();
	return 0;
}
