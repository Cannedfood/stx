#include <xwip/socket>

#include "test.hpp"

using namespace stx;
using namespace std::chrono;
using namespace std::chrono_literals;

#include <thread>

#include <string>
#include <vector>

#define PORT 27175
#define PORTSTR "27175"

struct HttpResponse {
	std::vector<std::string> headers;
	std::string              content;

	std::string to_string() {
		std::string total = "HTTP/1.1 200 OK\n";

		headers.emplace_back("Content-length: " + std::to_string(content.size() + 2));

		size_t ntotal = 0;
		for(auto& s : headers) ntotal += s.size() + 1;
		ntotal += 2 + content.size();

		total.reserve(ntotal);

		for(auto& s : headers) {
			total.append(s);
			total.push_back('\n');
		}
		total.append("\n\n");
		total.append(content);

		return total;
	}
};

void test_xsocket() {
	socket s;
	connection c;

	test(s.open(ip4, tcp).bind(PORT));

	// bool accepting = false;
	std::thread([&] {
		try {
			c = s.accept(1);
			test(c);
			if(c) {
				HttpResponse http;
				http.content = "Hello there\n";
				http.headers.emplace_back("Connection: close");
				c.write_s(http.to_string().c_str());
			}
			c.close();
		}
		catch(socket::failed_operation& e) {
			puts(e.what());
			test(!"Socket threw an error");
		}
		catch(socket::invalid_operation& e) {
			puts(e.what());
			test(!"Socket thinks there is an error in our code");
		}
		s.close();
	}).detach();

	std::this_thread::sleep_for(100ms);
	system("curl localhost:" PORTSTR);

	return;
}
