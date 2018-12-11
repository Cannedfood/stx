#include <stx/entities.hpp>
#include <stx/system.hpp>

struct transform {
	float x, y;
	float rotation;
};

int main() {
	stx::entities       entities;
	// stx::system_manager systems;

	entities.create();

	return 0;
}
