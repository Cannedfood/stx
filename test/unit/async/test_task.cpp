#include "../catch.hpp"

#include <stx/async/task.hpp>
#include <stx/async/task_queue.hpp>

TEST_CASE("Test task composition", "[task]") {
	stx::task_queue q1;
	stx::task_queue q2;

	enum State {
		A,B,C,D
	};

	State state = A;

	/*
	defer(q1, [&]() {
		state = B;
	})
	.then(q2, [&]() {
		state = C;
	})
	.then([&]() {
		state = D;
	});

	CHECK(state == A);
	q1.execute_tasks();
	CHECK(state == B);
	q2.execute_tasks(); // task C and D are both executed
	CHECK(state == D);
	*/
}