#include "gc.hpp"

#include <vector>
#include <algorithm>
#include <mutex>
#include <cassert>
#include <fstream>

#include "type.hpp"

namespace stx {

using garbage_collector::Deleter;

struct object {
	void* begin;
	void* end;

	bool marked;

	size_t timestamp;

	Deleter deleter;
	void*   deleter_data;

	std::type_info const* type = nullptr;

	bool contains(void* p) const noexcept { return p >= begin && p < end; }

	constexpr bool operator<(object const& other) const noexcept { return end < other.end; }
};

struct reference {
	void* from;
	void* to;

	bool from_stack  = false;
	bool from_object = false;
	size_t timestamp;
};

static size_t _obj_timestamp = 0;
static size_t _ref_timestamp = 0;

static std::vector<object>    _obj_hints;
static std::vector<object>    _obj;
static std::vector<reference> _refs;

static std::mutex _lock;

// Utilities
static object* _object_containing(void* p) noexcept {
	for(auto& o : _obj) {
		if(o.contains(p)) {
			return &o;
		}
	}
	return nullptr;
}

static object* _hint_containing(void* p) noexcept {
	for(auto& o : _obj_hints) {
		if(o.contains(p)) {
			return &o;
		}
	}
	return nullptr;
}

template<class C> static
void _each_ref_from(object const& o, C&& c) noexcept {
	for(auto& ref : _refs) {
		if(o.contains(ref.from)) {
			c(ref);
		}
	}
}

template<class C> static
void _each_ref_to(object const& o, C&& c) noexcept {
	for(auto& ref : _refs) {
		if(o.contains(ref.to)) {
			c(ref);
		}
	}
}

// Events
static void _add_object(object o) noexcept {
	_obj.insert(std::lower_bound(_obj.begin(), _obj.end(), o), o);
	// _obj.push_back(o);
}
static void _add_ref(reference ref) noexcept {
	ref.timestamp = _ref_timestamp++;
	_refs.push_back(ref);
}
static void _remove_ref(reference ref) noexcept {
	for (size_t i = 0; i < _refs.size(); i++) {
		if(_refs[i].from == ref.from && _refs[i].to == ref.to) {
			_refs.erase(_refs.begin() + i);
			break;
		}
	}
}

void garbage_collector::add_object(void* obj, size_t size, Deleter del, void* del_data, std::type_info const* type) noexcept {
	std::scoped_lock guard(_lock);

	_add_object({
		.begin = obj,
		.end   = (char*) obj + size,
		.timestamp = _obj_timestamp++,
		.deleter = del,
		.deleter_data = del_data,
		.type = type
	});
}

void garbage_collector::hint_external_object(void* obj, size_t size, std::type_info const* type) noexcept {
	std::scoped_lock guard(_lock);

	_obj_hints.push_back({
		.begin = obj,
		.end = (char*) obj + size,
		.type = type
	});
}

void garbage_collector::unhint_external_object(void* obj) noexcept {
	std::scoped_lock guard(_lock);

	for (size_t i = 0; i < _obj_hints.size(); i++)
	{
		if(_obj_hints[i].contains(obj)) {
			_obj_hints.erase(_obj_hints.begin() + i);
			return;
		}
	}
}

// static int counter = 0;

void garbage_collector::reference_added(void* from, void* to) noexcept {
	std::scoped_lock guard(_lock);

	// if(!_object_containing(from)) {
	// 	counter++;
	// 	printf("Reference from unmanaged memory %p (#%d)\n", from, counter);
	// 	switch (counter)
	// 	{
	// 	default: break;
	// 	}
	// }

	// if(!_object_containing(to)) {
	// 	puts("Reference to unmanaged memory!");
	// }

	_add_ref({ .from = from, .to = to, .from_stack = from < &from });
}

void garbage_collector::reference_removed(void* from, void* to) noexcept {
	std::scoped_lock guard(_lock);
	_remove_ref({ .from = from, .to = to });
}

static std::vector<object*> _waiting_for_children_to_be_marked; // Keeping around to avoid allocations
static std::vector<object>  _to_sweep;  // Keeping around to avoid allocations
static std::mutex           _sweep_lock;

void garbage_collector::mark() noexcept {
	std::scoped_lock guard(_lock);

	for(auto& o : _obj) o.marked = false;

	  ////////////
	 /// Mark ///
	////////////

	// Mark references by unmanaged memory
	for(auto& ref : _refs) {
		if(ref.from_object) continue;
		if(_object_containing(ref.from)) continue;

		if(auto* to = _object_containing(ref.to)) {
			to->marked = true;
			_waiting_for_children_to_be_marked.push_back(to);
		}
	}

	// Mark references recursively
	while(!_waiting_for_children_to_be_marked.empty()) {
		object* o = _waiting_for_children_to_be_marked.back();
		_waiting_for_children_to_be_marked.pop_back();

		_each_ref_from(*o, [&](reference const& r) {
			auto* to = _object_containing(r.to);
			assert(to&&"Reference to unmanaged memory");
			if(!to->marked) {
				to->marked = true;
				_waiting_for_children_to_be_marked.push_back(to);
			}
		});
	}
	// waiting_for_children_to_be_marked.clear(); -> already empty

	std::scoped_lock guard2(_sweep_lock);
	for(size_t i = 0; i < _obj.size(); i++) {
		if(!_obj[i].marked) {
			_to_sweep.emplace_back(std::move(_obj[i]));
			_obj.erase(_obj.begin() + i);
			i--;
		}
	}
}
void garbage_collector::collect() noexcept {
	std::scoped_lock guard(_sweep_lock); // TODO: reenable

	// Find things with no references
	for(auto& o : _to_sweep) {
		if(o.deleter) {
			size_t refs = 0;
			_each_ref_to(o, [&](auto&) { ++refs; });
			if(refs == 0) {
				o.deleter(o.begin, o.deleter_data);
				o.deleter = nullptr;
			}
		}
	}

	std::sort(_obj.begin(), _obj.end(), [](object const& a, object const& b) { return a.timestamp < b.timestamp; });
	for(auto& o : _to_sweep) {
		// printf("Destroy %s %p\n", (!o.type)?"":demangle(o.type->name()).c_str(), o.begin);
		if(o.deleter) {
			o.deleter(o.begin, o.deleter_data);
		}
	}
	_to_sweep.clear();
}

static int marking_and_sweeping = 0;
void garbage_collector::mark_and_sweep() noexcept {
	if(marking_and_sweeping == 0) {
		marking_and_sweeping++;
		mark();
		collect();
		marking_and_sweeping--;
	}
}

size_t garbage_collector::total_ref_count() noexcept {
	std::scoped_lock guard(_lock);
	return _refs.size();
}
size_t garbage_collector::total_obj_count() noexcept {
	std::scoped_lock guard(_lock);
	return _obj.size();
}

size_t garbage_collector::refcount(void* p) noexcept {
	std::scoped_lock guard(_lock);
	object* o = _object_containing(p);
	if(!o) return 0;

	size_t n = 0;
	_each_ref_to(*o, [&](auto) { ++n; });
	return n;
}

size_t garbage_collector::outrefcount(void* p) noexcept {
	std::scoped_lock guard(_lock);
	object* o = _object_containing(p);
	if(!o) return 0;

	size_t n = 0;
	_each_ref_from(*o, [&](auto) { ++n; });
	return n;
}

size_t garbage_collector::timestamp_of(void* obj) noexcept {
	std::scoped_lock guard(_lock);
	auto* o = _object_containing(obj);
	assert(o);
	return o->timestamp;
}

bool garbage_collector::is_valid(void* obj, size_t timestamp) noexcept {
	std::scoped_lock guard(_lock);
	auto* o = _object_containing(obj);
	return o && o->timestamp == timestamp;
}

void garbage_collector::writeDotFile(std::ostream& to, bool externalReferences) {
	std::scoped_lock guard(_lock);
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
	auto writeObj = [&](stx::object const& o, bool isHint) {
		to << "\t\"" << o.begin << "\" [";

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
		to << ((char*)o.end - (char*)o.begin) << " Bytes\\n";
		to << o.begin;
		to << '\"';

		if(isHint) {
			to << ",color=green";
		}

		to << "];\n";
	};
	for(auto o : _obj)       { writeObj(o, false); }
	for(auto o : _obj_hints) { writeObj(o, true); }
	to << '\n';

	to << "\t// References\n";
	for(auto r : _refs) {
		auto* from_obj = _object_containing(r.from);
		auto* to_obj   = _object_containing(r.to);

		if(!externalReferences && !from_obj) continue; // Skip external references

		if(!from_obj) from_obj = _hint_containing(r.from);

		auto* from_ptr = from_obj ? from_obj->begin : r.from;
		auto* to_ptr   = to_obj   ? to_obj->begin   : r.to;

		to << "\t\"" << from_ptr << "\" -> \"" << to_ptr << "\";\n";
	}

	to << "}" << std::endl;
}
void garbage_collector::writeDotFile(std::string const& path, bool externalReferences) {
	std::ofstream file(path);
	if(!file) throw std::runtime_error("Failed opening '"+path+"'");
	writeDotFile(file, externalReferences);
}

void garbage_collector::LEAK_ALL() noexcept {
	std::scoped_lock guard(_lock);
	_refs.clear();
	_obj.clear();
}

} // namespace stx
