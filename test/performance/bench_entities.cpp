#include <stx/entities.hpp>
#include <stx/system.hpp>

#include <stx/random.hpp>
#include <stx/timer.hpp>

struct position {
	float x, y;
};

struct velocity {
	float x, y;
};

struct color {
	float r, g, b, a;
};

struct circle {
	float radius;
};

struct square {
	float size;
};

int main() {
	stx::timer t;
	stx::random rnd;

	stx::entities entities;

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
	printf("Creating took %s\n", t.to_string().c_str());


	double dt    = .1;
	for (size_t i = 0; i < 1000; i++) {
		for(auto [pos, vel] : stx::filter<position, velocity>(entities)) {
			pos.x += dt * vel.x;
			pos.y += dt * vel.y;
		}

		for(auto [pos, col, circ] : stx::filter<position, color, circle>(entities)) {
			circ.radius /= pos.x * col.r;
		}

		for(auto [pos, col, sqr] : stx::filter<position, color, square>(entities)) {
			sqr.size /= pos.x * col.r;
		}
	}

	printf("Iteration took %s\n", t.to_string().c_str());

	return 0;
}
