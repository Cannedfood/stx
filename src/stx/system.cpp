#include "system.hpp"

#include <random>
#include <algorithm>
#include <cassert>

namespace stx {

using group_mask = system_manager::group_mask;
using group_names = system_manager::group_names;

system_manager::system_manager() noexcept : m_injector(nullptr) {}
system_manager::system_manager(stx::injector& inj) noexcept : m_injector(&inj) {}
system_manager::~system_manager() noexcept {
	for (auto& e : m_systems) {
		if(e.enabled) {
			e.sys->sysDisable(*this);
		}
	}

	while(!m_systems.empty()) {
		m_systems.front().sys->sysRemoved();
		m_systems.pop_front();
	}
}

// -- Update -------------------------------------------------------

void system_manager::update(float dt) {
	for(auto& system : m_systems) {
		if(system.enabled) {
			system.profiling.sysUpdate.measure([&]() {
				system.sys->sysUpdate(dt);
			});
		}
	}
}

// -- Manage groups --------------------------------------------------
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
void system_manager::toggle (std::string_view group) { toggle(groupId(group)); }

void system_manager::enable (group_names groups) { enable(groupMask(groups)); }
void system_manager::disable(group_names groups) { disable(groupMask(groups)); }
void system_manager::toggle (group_names groups) { toggle(groupMask(groups)); }

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
void system_manager::toggle(unsigned group) {
	group_mask groups = m_enabled_groups;
	groups[group] = !groups[group];
	set(groups);
}

void system_manager::enable (group_mask groups) { set(m_enabled_groups | groups); }
void system_manager::disable(group_mask groups) { set(m_enabled_groups & ~groups); }
void system_manager::toggle (group_mask groups) { set(m_enabled_groups ^ groups); }
void system_manager::set(group_mask groups) {
	m_enabled_groups = groups;
	for(auto& e : m_systems) {
		_updateEnabled(e);
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
	shared<system> sys)
{
	m_systems.push_back({});
	entry& e = m_systems.back();
	e.name       = name;
	e.sys        = sys;
	e.enabledBy  = enabledIn;
	e.disabledBy = disabledIn;

	class add_configurator final : public system_configuration {
		entry& m_entry;
	public:
		add_configurator(system_manager& manager, entry& e) noexcept
			: system_configuration(manager), m_entry(e)
		{}
		void enabledBy (std::initializer_list<std::string_view> groups) noexcept override { m_entry.enabledBy |= manager.groupMask(groups); }
		void disabledBy(std::initializer_list<std::string_view> groups) noexcept override { m_entry.disabledBy |= manager.groupMask(groups); }
	};

	auto add_config = add_configurator{*this, e};
	e.profiling.sysAdded    .measure([&]() { sys->sysAdded(add_config); });
	e.profiling.sysConfigure.measure([&]() { sys->sysConfigure(add_config); });

	_updateEnabled(e);
}

void system_manager::add(
	std::string_view name, group_names enabledIn, group_names disabledIn,
	shared<system> sys)
{
	add(name, groupMask(enabledIn), groupMask(disabledIn), sys);
}

void system_manager::add(
	std::string_view name, group_names enabledIn, shared<system> sys)
{
	add(name, groupMask(enabledIn), {}, sys);
}

void system_manager::add(std::string_view name, shared<system> sys)
{
	add(name, {}, {}, sys);
}

injector& system_manager::inject() {
	assert(m_injector && "No injector set in the system_manager's constructor");
	return *m_injector;
}

// Debug
void   system_manager::debugRandomize() {
	std::shuffle(m_systems.begin(), m_systems.end(), std::mt19937(std::random_device()()));
}
void   system_manager::maxProfileMeasurements(size_t maxNumMeasurements) {} // TODO
size_t system_manager::maxProfileMeasurements() { return 0; } // TODO

void system_manager::_updateEnabled(entry& e) {
	bool should_be_enabled =
		(e.enabledBy  & m_enabled_groups).any() &&
		(e.disabledBy & m_enabled_groups).none() &&
		!e.forceDisable;

	if(e.enabled != should_be_enabled) {
		e.enabled = should_be_enabled;
		if(e.enabled) {
			e.profiling.sysEnable.measure([&]() {
				e.sys->sysEnable(*this);
			});
		}
		else {
			e.profiling.sysEnable.measure([&]() {
				e.sys->sysDisable(*this);
			});
		}
	}
}

} // namespace stx
