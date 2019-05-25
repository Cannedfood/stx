#include "shared.hpp"

#ifdef STX_SHARED_DEBUG

#include <set>
#include <iostream>
#include <mutex>
#include "type.hpp"

namespace stx {

struct ref {
	char const* from;
	char const* to;

	size_t length() const noexcept { return to - from; }

	operator void const*() const noexcept { return from; }
};

struct obj {
	char const*           start;
	char const*           end;
	std::type_info const* type;
	int                   refcount = 0;

	bool contains(char const* p) const noexcept { return p >= start && p < end; }
	bool overlaps(obj const& other) const noexcept { return start < other.end && other.start < end; }

	size_t length() const noexcept { return end - start; }
	operator void const*() const noexcept { return start; }
};

static std::mutex _lock;
static std::vector<obj> _obj;
static std::vector<ref> _refs;

static void _cleanup_data() {
	// Remove references to null
	_refs.erase(
		std::remove_if(_refs.begin(), _refs.end(),
			[](ref& r) { return r.from == nullptr; }
		),
		_refs.end()
	);

	// Remove objects without a reference
	for(auto& o : _obj) o.refcount = 0;
	for(auto& ref : _refs) {
		for(auto& o : _obj) {
			if(o.contains(ref.to)) {
				o.refcount++;
			}
		}
	}
	_obj.erase(
		std::remove_if(_obj.begin(), _obj.end(),
			[](obj& o) { return o.refcount > 0; }
		),
		_obj.end()
	);
}

static auto _add_ref(ref r) {
	auto ref_iter = std::lower_bound(_refs.begin(), _refs.end(), r);
	if(ref_iter != _refs.end() && ref_iter->from == r.from) {
		*ref_iter = r;
		return ref_iter;
	}
	else {
		return _refs.insert(ref_iter, r);
	}
}

static auto _add_obj(obj o) {
	for(auto& other : _obj) {
		if(o.overlaps(other)) {
			other.type  = (!other.type || (o.length() > other.length())) ? o.type : other.type;
			other.start = std::min(o.start, other.start);
			other.end   = std::max(o.end, other.end);
			return;
		}
	}
	_obj.push_back(o);
}

static obj* _obj_containing(char const* p) {
	for(auto& o : _obj) {
		if(o.contains(p)) return &o;
	}
	return nullptr;
}

void debug::trace_reference(void const* self, void const* ptr, size_t type_size, std::type_info const* type) noexcept {
	std::lock_guard guard{_lock};

	auto o = obj {
		.start = (const char*)ptr,
		.end   = ((const char*) ptr) + type_size,
		.type  = type
	};

	auto r = ref {
		.from = (const char*) self,
		.to   = o.start
	};

	_add_ref(r);
	_add_obj(o);
}

void debug::shared_debug_print(std::ostream& to) noexcept {
	_cleanup_data();

	to << "digraph managed_objects {\n";

	to << "\t// Settings\n";
	to << "\tgraph[\n"
		  "\t	layout=fdp,\n"
		  "\t	center=true,\n"
		  "\t	margin=0.2,\n"
		  "\t	nodesep=0.1,\n"
		  "\t	splines=true,\n"
		  "\t	overlap=false\n"
		  "\t];\n"
		  "\tnode[\n"
		  "\t	shape=none,\n"
		  "\t	width=2.5,\n"
		  "\t	height=.6,\n"
		  "\t	fixedsize=true\n"
		  "\t];\n\n";

	to << "\t// Objects\n";
	for(auto& o : _obj) {
		to << "\t\"" << (void*)o.start << "\" [";

		// Label
		to << "label=\"";
		if(o.type) {
			std::string name = demangle(o.type->name());
			if(name.find("std::__") == 0) {
				to << "STL Container\\n";
			}
			else {
				to << name << "\\n";
			}
		}
		to << o.length() << " Bytes\\n";
		to << (void*)o.start;
		to << '\"';

		to << "];\n";
	};
	to << '\n';

	to << "\t// References\n";
	for(auto r : _refs) {
		auto* obj_from = _obj_containing(r.from);
		auto* obj_to   = _obj_containing(r.to);

		void* ptr_from = (void*)(obj_from? obj_from->start : r.from);
		void* ptr_to   = (void*)(obj_to?   obj_to->start   : r.to);

		if(ptr_to == nullptr) continue;

		to << "\t\"" << ptr_from << "\" -> \"" << ptr_to << "\";\n";
	}

	to << "}" << std::endl;
}

} // namespace stx

#endif
