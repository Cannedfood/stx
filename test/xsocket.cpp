#include <xsocket>

#include "test.hpp"

using namespace stx;


void test_xsocket() {
	server srv;

	srv.open(ipv4, tcp);
	srv.close();
}
