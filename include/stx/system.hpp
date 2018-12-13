#pragma once

#include <vector>
#include <deque>
#include <bitset>
#include <unordered_map>
#include <chrono>

#include "shared_ptr.hpp"

namespace stx {

namespace options {
constexpr inline size_t MaxNumSystemGroups = 64;
} // namespace options

class system;
class system_manager;
class system_configuration;

/// Interface used by system::sysConfigure to gather data on how the system should be executed
class system_configuration {
public:
	virtual system_manager& manager() noexcept = 0;

	// Injection // TODO

	// Groups

	// Execution
	virtual void parallel(bool b = true)     noexcept = 0;
	virtual void asynchronous(bool b = true) noexcept = 0;
	virtual void threadsafe(bool b = true)   noexcept = 0;

	/// Sets update repetition
	virtual void target_dt(float seconds)    noexcept = 0;
};

class system {
public:
	virtual void sysAdded(system_configuration&) {}
	virtual void sysConfigure(system_configuration&) {}
	virtual void sysEnable   (system_manager&) {}
	virtual void sysUpdate   (system_manager&) {}
	virtual void sysDisable  (system_manager&) {}
	virtual void sysRemoved() {}
};

class system_manager {
public:
	using group_mask  = std::bitset<options::MaxNumSystemGroups>;
	using group_names = std::initializer_list<std::string_view>;

	system_manager();
	~system_manager();

	// Manage groups
	unsigned   groupId(std::string_view s);
	group_mask groupMask(group_names);

	// Enable/Disable
	void enable (std::string_view group);
	void disable(std::string_view group);
	void enable (group_names groups);
	void disable(group_names groups);
	void set    (group_names groups);

	void enable (unsigned group);
	void disable(unsigned group);
	void enable (group_mask groups);
	void disable(group_mask groups);
	void set    (group_mask groups);

	void push(); //<! Push enabled state
	void pop();  //<! Restore last pushed enabled state

	// Manage systems
	void add(
		std::string_view name,
		group_mask enabledIn,
		group_mask disabledIn,
		std::shared_ptr<system> sys);

	void add(
		std::string_view name,
		group_names enabledIn,
		group_names disabledIn,
		std::shared_ptr<system> sys);

	void add(
		std::string_view name,
		group_names enabledIn,
		std::shared_ptr<system> sys);

	void add(
		std::string_view name,
		std::shared_ptr<system> sys);

	group_mask enableOn       (std::string_view systemName) noexcept;
	void       enableOn       (std::string_view systemName, group_names) noexcept;
	void       enableOnAdd    (std::string_view systemName, group_names) noexcept;
	void       enableOnRemove (std::string_view systemName, group_names) noexcept;

	group_mask disableOn      (std::string_view systemName) noexcept;
	void       disableOn      (std::string_view systemName, group_names) noexcept;
	void       disableOnAdd   (std::string_view systemName, group_names) noexcept;
	void       disableOnRemove(std::string_view systemName, group_names) noexcept;

	// Debug
	void   debugRandomize(bool b); //!< Randomizes order to uncover bugs
	bool   debugRandomize();
	void   maxProfileMeasurements(size_t maxNumMeasurements); //<! Whether/How much performance data should be recorded
	size_t maxProfileMeasurements();

private:
	struct entry {
		std::string name;
		std::shared_ptr<system> sys;
		group_mask enabledBy, disabledBy;

		struct profiling_info {
			using clock = std::chrono::high_resolution_clock;

			enum class EventType {
				sysAdded, sysConfigure, sysEnable, sysUpdate, sysDisable, sysRemoved
			};

			struct data_point {
				EventType         type;
				int               thread;
				clock::time_point start;
				clock::time_point end;
			};

			std::deque<data_point> infos;
		};
		profiling_info profiling;
	};

	std::deque<entry> m_systems;
	group_mask         m_enabled_groups;

	std::unordered_map<std::string, unsigned> m_group_ids;

	std::vector<group_mask> m_state_stack;
};

} // namespace stx
