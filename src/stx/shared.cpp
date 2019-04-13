#include "shared.hpp"

#ifdef STX_SHARED_DEBUG

#include <set>
#include <iostream>
#include <mutex>
#include "type.hpp"

namespace stx {

struct shared_debug_entry {
	void* shared;

	mutable void*  ptr;
	mutable size_t size;

	mutable std::type_info const* type;

	operator void*() const noexcept { return this->shared; }

	void* begin() const noexcept { return ptr; }
	void* end()   const noexcept { return ((char*)ptr) + size; }

	bool same_object_as(shared_debug_entry const& other) const noexcept {
		return begin() <= other.begin() && end() >= other.end();
	}

	void merge_object(shared_debug_entry const& other) const noexcept {
		void* min                  = std::min(begin(), other.begin());
		void* max                  = std::max(end(), other.end());
		std::type_info const* type = (begin() < other.begin()) ? this->type : other.type;

		this->ptr  = other.ptr  = min;
		this->size = other.size = ((char*)max) - ((char*) min);
		this->type = other.type = type;
	}

	bool contains(void* ptr) const noexcept { return begin() <= ptr && ptr < end(); }
};

static std::mutex _lock;
static std::set<shared_debug_entry> _entries;

void debug::shared_created  (void* shared, void* ptr, size_t type_size, std::type_info const* type) noexcept {
	std::lock_guard guard(_lock);

	auto entry = shared_debug_entry{
		shared,
		ptr, type_size,
		type
	};

	for(auto& other : _entries) {
		if(other.same_object_as(entry)) {
			other.merge_object(entry);
		}
	}

	for(auto& other : _entries) {
		if(entry.same_object_as(other)) {
			entry.merge_object(other);
		}
	}

	_entries.insert(std::lower_bound(_entries.begin(), _entries.end(), entry), entry);
}
void debug::shared_destroyed(void* sptr) noexcept {
	std::lock_guard guard(_lock);

	size_t success = _entries.erase(shared_debug_entry {
		sptr,
		nullptr, 0,
		nullptr
	});

	if(success == 0) {
		std::cerr << "Unknown pointer destroyed" << std::endl;
	}
}
void debug::shared_debug_print(bool all) noexcept {
	std::lock_guard guard(_lock);

	auto print = [](shared_debug_entry const& e) {
		std::cout << "shared<";
		if(e.type) std::cout << demangle(e.type->name());
		else       std::cout << "??";
		std::cout << ">(" << e.ptr << ")";
	};

	std::cout << "All shared pointers:" << std::endl;
	for (auto& entry : _entries) {
		bool printed = false;
		for(auto& other : _entries) {
			if(entry.contains(other.shared)) {
				if(!printed) {
					print(entry);
					std::cout << "\n";
				}

				std::cout << "\t";
				print(other);
				std::cout << "\n";
			}
		}
		if(all && !printed) {
			print(entry);
			std::cout << "\n";
		}
	}
	std::cout.flush();
}

} // namespace stx

#endif
