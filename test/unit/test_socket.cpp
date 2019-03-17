#include <stx/socket.hpp>

#include "catch.hpp"

#include <thread>
#include <chrono>
using namespace std::chrono_literals;

TEST_CASE("Test socket", "[socket]") {
	std::string_view msg = "Hello there\n";
	uint16_t port = 31415;

	stx::socket client;
	stx::socket server;

	volatile bool server_started = false;

	std::thread server_thread([&]() {
		REQUIRE(server.open(stx::domain::ipv4, stx::socktype::tcp));
		REQUIRE(server.option(stx::sockopt::reuse_port, true));

		REQUIRE(server.bind(stx::ipv4(port)));
		server_started = true;
		REQUIRE(server.listen(1));

		stx::socket client_connection = server.accept();
		REQUIRE(client_connection);

		char recvd_buffer[64] = {'\0'};
		int  recvd_length = client_connection.recv(recvd_buffer, sizeof(recvd_buffer));
		REQUIRE(recvd_length == msg.size());
		REQUIRE(recvd_buffer == msg);
	});

	while(!server_started) std::this_thread::sleep_for(5ms);

	std::thread client_thread([&]() {
		REQUIRE(client.open(stx::domain::ipv4, stx::socktype::tcp));
		REQUIRE(client.connect(stx::ipv4(127,0,0,1,  port)));
		client.send(msg.data(), msg.size());
	});

	client_thread.join();
	server_thread.join();
}
