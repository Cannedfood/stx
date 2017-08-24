#include <xwip/socket>

#include "test.hpp"

using namespace stx;

#include <string>
#include <vector>

#define PHRASE "Hallo ihr menschen"

#include <chrono>
#include <cstring>

using namespace std::chrono;
using namespace std::chrono_literals;

#define attempt(OPERATION)                             \
	try {                                              \
		OPERATION;                                     \
	} catch(std::runtime_error e) {                    \
		printf("Caught runtime_error at %s:%i:  %s\n", \
		       __FILE__,                               \
		       __LINE__,                               \
		       e.what());                              \
	}

void test_xsocket() {
	char buffer[] = PHRASE;

	tcp_server srv;
	puts("Server: Creating server...");
	attempt(srv.bind(8080, true));
	if(!srv) return;

	puts("Server: Listening...");
	auto thread = srv.listenAsync([&](tcp_connection&& c) {
		puts("Server: New connection!");
		attempt(c.send(buffer, sizeof(buffer)));
	});

	std::this_thread::sleep_for(200ms);

	tcp_connection c;
	puts("Client: Connecting to localhost:8080");
	attempt(c.connect("localhost", "8080", true));
	if(!c.connected()) return;

	puts("Client: Reading from connection...");
	attempt(test(c.recv_raw(buffer, sizeof(buffer)) == sizeof(buffer)));

	test(strcmp(buffer, PHRASE) == 0);

	srv.close();
	c.close();
}
