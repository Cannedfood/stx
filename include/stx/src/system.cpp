#include "../system.hpp"

namespace stx {

using group_mask = system_manager::group_mask;
using group_names = system_manager::group_names;

system_manager::system_manager() {
	std::terminate();
} // TODO
system_manager::~system_manager() {} // TODO

// Manage groups
unsigned system_manager::groupId(std::string_view s) {
	unsigned new_id     = m_group_ids.size();
	auto [iter, is_new] = m_group_ids.try_emplace(std::string(s), new_id);
	return is_new ? new_id : iter->second;
}
group_mask system_manager::groupMask(group_names names) {
	group_mask result;
	for(auto& name : names) result.set(groupId(name));
	return result;
}

// -- Enable/Disable ------------------------------------------------

// by name

void system_manager::disable(std::string_view group) { disable(groupId(group)); }
void system_manager::enable (std::string_view group) { enable(groupId(group)); }

void system_manager::enable (group_names groups) { enable(groupMask(groups)); }
void system_manager::disable(group_names groups) { disable(groupMask(groups)); }
void system_manager::set    (group_names groups) { set(groupMask(groups)); }

// by id

void system_manager::enable (unsigned group) {
	group_mask groups = m_enabled_groups;
	groups.set(group);
	set(groups);
}
void system_manager::disable(unsigned group) {
	group_mask groups = m_enabled_groups;
	groups.reset(group);
	set(groups);
}

void system_manager::enable (group_mask groups) { set(m_enabled_groups | groups); }
void system_manager::disable(group_mask groups) { set(m_enabled_groups & ~groups); }
void system_manager::set(group_mask groups) {
	// TODO:
}

void system_manager::push() {
	m_state_stack.emplace_back(m_enabled_groups);
}
void system_manager::pop() {
	group_mask enabled_groups = m_state_stack.back();
	m_state_stack.pop_back();
	set(enabled_groups);
}

// Manage systems
void system_manager::add(
	std::string_view name, group_mask enabledIn, group_mask disabledIn,
	std::shared_ptr<system> sys)
{
	m_systems.push_back({
		std::string(name), sys, enabledIn, disabledIn, {}
	});

	entry& e = m_systems.back();

	class add_configurator final : public system_configuration {
		system_manager& m_manager;
		entry&          m_entry;
	public:
		add_configurator(system_manager& manager, entry& e) :
			m_manager{manager},
			m_entry{e}
		{}

		system_manager& manager() noexcept override { return m_manager; }

		void parallel(bool b = true)     noexcept override { std::terminate(); }
		void asynchronous(bool b = true) noexcept override { std::terminate(); }
		void threadsafe(bool b = true)   noexcept override { std::terminate(); }

		void target_dt(float seconds)    noexcept override { std::terminate(); }
	};

	auto add_config = add_configurator{*this, e};
	sys->sysAdded(add_config);

	// TODO: create proper config
	sys->sysConfigure(add_config);
}

void system_manager::add(
	std::string_view name, group_names enabledIn, group_names disabledIn,
	std::shared_ptr<system> sys)
{
	 add(name, groupMask(enabledIn), groupMask(enabledIn), sys);
}

void system_manager::add(
	std::string_view name, group_names enabledIn, std::shared_ptr<system> sys)
{
	add(name, groupMask(enabledIn), {}, sys);
}

void system_manager::add(std::string_view name, std::shared_ptr<system> sys)
{
	add(name, {}, {}, sys);
}

// Debug
void   system_manager::debugRandomize(bool b) {} // TODO
bool   system_manager::debugRandomize() { return false; } // TODO
void   system_manager::maxProfileMeasurements(size_t maxNumMeasurements) {} // TODO
size_t system_manager::maxProfileMeasurements() { return 0; } // TODO

} // namespace stx
