#include "test.hpp"

#include <xwip/filesystem_monitor>

#include <cstdio>
#include <memory.h>
#include <fstream>
#include <thread>

using namespace stx;

void test_xfilesystem_monitor() {
	char name[L_tmpnam + 1];
	memset(name, 0, sizeof(name));

	if(!std::tmpnam(name)) test(!"Failed creating temporary file");

	printf("name: %s\n", name);

	std::ofstream(name) << "Hi" << std::endl;

	filesystem_monitor monitor;

	monitor.set_observer(name, filesystem_monitor::modified, [](unsigned mask) {
		printf("Something changed! %s\n", filesystem_monitor::to_string((filesystem_monitor::mask)mask));
	});

	std::ofstream(name) << "Hi" << std::endl;

	remove(name);

	monitor.poll_events();
}
