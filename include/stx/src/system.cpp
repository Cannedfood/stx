#include "../system.hpp"

#include <iostream>

namespace stx {

using group_mask = system_manager::group_mask;
using group_names = system_manager::group_names;

system_manager::system_manager() noexcept {}
system_manager::~system_manager() noexcept {}

// Manage groups
unsigned system_manager::groupId(std::string_view s) noexcept {
	unsigned new_id     = m_group_ids.size();
	auto [iter, is_new] = m_group_ids.try_emplace(std::string(s), new_id);
	return is_new ? new_id : iter->second;
}
group_mask system_manager::groupMask(group_names names) noexcept {
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
	std::cout << "Set " << groups << std::endl;

	m_enabled_groups = groups;
	for(auto& e : m_systems) {
		bool should_be_enabled =
			(e.enabledBy  & m_enabled_groups).any() &&
			(e.disabledBy & m_enabled_groups).none() &&
			!e.forceDisable;

		std::cout << e.name << " " << should_be_enabled << " " << e.enabledBy << "  " << e.disabledBy << std::endl;

		// printf("%s: %i| %s %s\n", e.name.c_str(), should_be_enabled);

		if(e.enabled != should_be_enabled) {
			e.enabled = should_be_enabled;
			if(e.enabled) {
				e.sys->sysEnable(*this);
			}
			else {
				e.sys->sysDisable(*this);
			}
		}
	}
}

void system_manager::push() noexcept {
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
	m_systems.push_back({});
	entry& e = m_systems.back();
	e.name       = name;
	e.sys        = sys;
	e.enabledBy  = enabledIn;
	e.disabledBy = disabledIn;

	class add_configurator final : public system_configuration {
		system_manager& m_manager;
		entry&          m_entry;
	public:
		add_configurator(system_manager& manager, entry& e)
			: m_manager(manager), m_entry(e)
		{}
		system_manager& manager() noexcept override { return m_manager; }
		void enabledBy(std::initializer_list<std::string_view> groups) noexcept override {
			m_entry.enabledBy |= m_manager.groupMask(groups);
		}
		void disabledBy(std::initializer_list<std::string_view> groups) noexcept override {
			m_entry.disabledBy |= m_manager.groupMask(groups);
		}
	};

	std::cout << "Add '" << e.name << "' {"<<e.enabledBy<<"} | {"<<e.disabledBy<<"}" << std::endl;

	auto add_config = add_configurator{*this, e};
	sys->sysAdded(add_config);
	sys->sysConfigure(add_config);
}

void system_manager::add(
	std::string_view name, group_names enabledIn, group_names disabledIn,
	std::shared_ptr<system> sys)
{
	 add(name, groupMask(enabledIn), groupMask(disabledIn), sys);
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
