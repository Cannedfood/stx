#include "gc.hpp"

#include <array>
#include <map>
#include <vector>
#include <cassert>
#include <algorithm>

#include <iostream>
#include <fstream>

namespace stx {

/*
struct gc_out_refs : public std::array<void*, 16> {
	void add(void* p) {
		for(auto slot : *this) {
			if(slot == p) return;
		}

		for(auto& slot : *this) {
			if(!slot) {
				slot = p;
				return;
			}
		}
		puts("Out of reference slots");
		std::terminate();
	}

	void remove(void* p) {
		for(auto& slot : *this) {
			if(slot == p) {
				slot = nullptr;
				return;
			}
		}
		puts("Couldn't find out ref");
		std::terminate();
	}
};
*/

struct gc_object_entry {
	void* end;

	void(*pfnDelete)(void* p, void* user);
	void* user;

	bool marked = true; // Makes sure mark() was run before this object is deleted

	size_t timestamp;
};

struct gc_ref_entry {
	void*            to;
	gc_object_entry* from_object = nullptr;
	gc_object_entry* to_object = nullptr;
};

static size_t _gc_timestamp = 0;
static std::map<void*, gc_ref_entry>    _gc_refs;
static std::map<void*, gc_object_entry> _gc_objects;

static std::vector<std::pair<void*, gc_object_entry>> _to_sweep;

static auto _object_containing(void* ptr) {
	/*
	auto iter = _gc_objects.lower_bound(ptr);
	if(iter == _gc_objects.end()) return iter;
	if(iter->first      > ptr)  return _gc_objects.end();
	if(iter->second.end <= ptr) return _gc_objects.end();
	*/
	// return iter;
	auto iter = _gc_objects.begin();
	while(iter != _gc_objects.end()) {
		if(iter->first <= ptr && iter->second.end > ptr)
			return iter;
		++iter;
	}
	return iter;
}
static gc_object_entry& _ref_target(gc_ref_entry& ref) {
	if(ref.to_object) return *ref.to_object;

	ref.to_object = &_object_containing(ref.to)->second;

	return *ref.to_object;
}

void garbage_collector::register_object(void* p, size_t size, void(*pfnDelete)(void* p, void* user), void* user) noexcept {
	assert(_gc_objects.count(p) == 0);

	// printf("obj+ %p - %p\n", p, ((char*) p) + size);

	_gc_objects.emplace(p, gc_object_entry {
		.end = ((char*)p) + size,
		.pfnDelete = pfnDelete,
		.user = user,
		.timestamp = ++_gc_timestamp
	});
}

void garbage_collector::create_ref(void* from, void* to) noexcept {
	assert(_object_containing(to) != _gc_objects.end());
	_gc_refs.emplace(from, gc_ref_entry { .to = to });

	// printf("ref+ %p -> %p (%p)\n", from, _object_containing(to)->first, to);
}
void garbage_collector::remove_ref(void* from) noexcept {
	// auto iter = _object_containing(_gc_refs.at(from).to);
	// printf("ref- %p -> %p (%p)\n", from, iter == _gc_objects.end() ? (void*)(0xC011EC7ED) : iter->first, _gc_refs.at(from).to);

	size_t erased = _gc_refs.erase(from);
	assert(erased != 0);
}


void garbage_collector::mark_and_sweep() noexcept {
	// Unmark
	for(auto& [obj, obj_entry] : _gc_objects) {
		obj_entry.marked = false;
	}

	// Mark root level
	for(auto& [ref, ref_entry] : _gc_refs) {
		if(ref_entry.from_object) continue;

		auto obj_iter = _object_containing(ref);
		ref_entry.from_object = obj_iter != _gc_objects.end() ? &obj_iter->second : nullptr;

		if(!ref_entry.from_object) {
			// There's a reference to it, but the reference doesn't come from a managed object
			//  -> treat as root reference
			_ref_target(ref_entry).marked = true;
		}
	}

	// Mark indirect
	// TODO: optimize
	bool anyMarked;
	do {
		anyMarked = false;
		for(auto& [ref, ref_entry] : _gc_refs) {
			if(ref_entry.from_object && ref_entry.from_object->marked) {
				auto& target_obj = _ref_target(ref_entry);
				if(!target_obj.marked) {
					anyMarked = true;
					target_obj.marked = true;
				}
			}
		}
	} while(anyMarked);

	// Sweep
	using obj_iter = std::map<void*, gc_object_entry>::iterator;

	std::vector<obj_iter> to_sweep;
	for(auto iter = _gc_objects.begin(); iter != _gc_objects.end(); ++iter) {
		if(!iter->second.marked) {
			to_sweep.emplace_back(iter);
		}
	}
	std::sort(to_sweep.begin(), to_sweep.end(),
		[](obj_iter const& a, obj_iter const& b) {
			return a->second.timestamp < b->second.timestamp;
		}
	);
	for(auto to_delete : to_sweep) {
		auto& [ptr, entry] = *to_delete;
		entry.pfnDelete(ptr, entry.user);
		_gc_objects.erase(to_delete);
	}
}

size_t garbage_collector::refcount(void* p) noexcept {
	auto obj_iter = _object_containing(p);
	if(obj_iter == _gc_objects.end()) {
		puts("Objects end");
		return 0;
	}

	size_t count = 0;
	for(auto& [from, entry] : _gc_refs) {
		if(entry.to >= obj_iter->first && entry.to < obj_iter->second.end) {
			count++;
		}
	}
	return count;
}

size_t garbage_collector::outrefcount(void* p) noexcept {
	auto obj_iter = _object_containing(p);
	if(obj_iter == _gc_objects.end()) {
		puts("Objects end");
		return 0;
	}

	size_t count = 0;
	for(auto& [from, entry] : _gc_refs) {
		if(from >= obj_iter->first && from < obj_iter->second.end) {
			count++;
		}
	}
	return count;
}

size_t garbage_collector::total_ref_count() noexcept { return _gc_refs.size(); }
size_t garbage_collector::total_obj_count() noexcept { return _gc_objects.size(); }

void garbage_collector::LEAK_ALL() noexcept {
	_gc_refs.clear();
	mark_and_sweep();
}

void garbage_collector::printDotGraph(const char* path) {
	std::ofstream stream(path, std::ios::binary);
	printDotGraph(stream);
}
void garbage_collector::printDotGraph(std::ostream& out) {
	out << "digraph Objects {\n";
	for(auto& [ref, ref_entry] : _gc_refs) {
		auto from = _object_containing(ref);
		auto to   = _object_containing(ref_entry.to);

		// auto* pfrom = from == _gc_objects.end() ? nullptr : from->first;

		// out << '"'<<ref<<'(' << pfrom << ')' << '"';
		// out << " -> ";
		// out << '"'<<ref_entry.to<<'(' << to->first << ')' << "\";\n";
		out << "\t\"";
		if(from != _gc_objects.end())
			out << from->first;
		else
			out << "Extern -> " << ref;
		out << "\"" << " -> \"" << to->first << "\";\n";
	}
	out << "}" << std::endl;
}

} // namespace stx
