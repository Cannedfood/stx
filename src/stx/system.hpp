#pragma once

#include <deque>
#include <bitset>
#include <chrono>

#include "injector.hpp"
#include "shared.hpp"

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
	system_manager& manager;

	system_configuration(system_manager& manager) noexcept : manager(manager) {}

	// Injection
	injector& inject();
	template<class T> stx::shared<T> inject() { return inject().get<T>(); }
	template<class Arg1, class... ArgN> void inject(Arg1&&, ArgN&&...);

	// Groups
	virtual void enabledBy(std::initializer_list<std::string_view> groups) noexcept = 0;
	virtual void disabledBy(std::initializer_list<std::string_view> groups) noexcept = 0;
};

class system : public stx::enable_shared_from_this<system> {
public:
	virtual ~system() {}
	virtual void sysAdded(system_configuration&) {}
	virtual void sysConfigure(system_configuration&) {}
	virtual void sysEnable(system_manager&) {}
	virtual void sysUpdate(float dt) noexcept {}
	virtual void sysDisable(system_manager&) noexcept {}
	virtual void sysRemoved() noexcept {}
};

class system_manager {
public:
	using group_mask  = std::bitset<options::MaxNumSystemGroups>;
	using group_names = std::initializer_list<std::string_view>;

	system_manager() noexcept;
	system_manager(stx::injector&) noexcept;
	~system_manager() noexcept;

	// Updating
	void update(float dt);

	// Manage groups
	unsigned   groupId(std::string_view s) noexcept;
	group_mask groupMask(group_names) noexcept;

	// Enable/Disable
	void enable (std::string_view group);
	void disable(std::string_view group);
	void toggle (std::string_view group);

	void enable (group_names groups);
	void disable(group_names groups);
	void toggle (group_names groups);

	void set    (group_names groups);

	void enable (unsigned group);
	void disable(unsigned group);
	void toggle (unsigned group);

	void enable (group_mask groups);
	void disable(group_mask groups);
	void toggle (group_mask groups);

	void set    (group_mask groups);

	void push() noexcept; //<! Push enabled state
	void pop();  //<! Restore last pushed enabled state

	// Manage systems
	void add(
		std::string_view name,
		group_mask enabledIn,
		group_mask disabledIn,
		shared<system> sys);

	void add(
		std::string_view name,
		group_names enabledIn,
		group_names disabledIn,
		shared<system> sys);

	void add(
		std::string_view name,
		group_names enabledIn,
		shared<system> sys);

	void add(
		std::string_view name,
		shared<system> sys);

	group_mask enableOn       (std::string_view systemName) noexcept;
	void       enableOn       (std::string_view systemName, group_names) noexcept;
	void       enableOnAdd    (std::string_view systemName, group_names) noexcept;
	void       enableOnRemove (std::string_view systemName, group_names) noexcept;

	group_mask disableOn      (std::string_view systemName) noexcept;
	void       disableOn      (std::string_view systemName, group_names) noexcept;
	void       disableOnAdd   (std::string_view systemName, group_names) noexcept;
	void       disableOnRemove(std::string_view systemName, group_names) noexcept;

	// Debug
	void   debugRandomize(); //!< Randomizes order to uncover bugs
	void   maxProfileMeasurements(size_t maxNumMeasurements); //<! Whether/How much performance data should be recorded
	size_t maxProfileMeasurements();

	injector& inject();

	template<class T> stx::shared<T> inject() { return inject().get<T>(); }
	template<class... Args> void inject(Args&&... args) { inject().get(std::forward<Args>(args)...); }
private:
	struct entry {
		std::string name;
		shared<system> sys;
		group_mask enabledBy, disabledBy;
		bool forceDisable = false;
		bool enabled = false;

		struct profiling_info {
			using clock = std::chrono::high_resolution_clock;

			enum class EventType {
				sysAdded, sysConfigure, sysEnable, sysUpdate, sysDisable, sysRemoved
			};

			struct data_point {
				EventType         type;
				int               thread;
				clock::time_point start, end;
			};

			std::deque<data_point> infos;
		};
		profiling_info profiling;
	};

	std::deque<entry> m_systems;
	group_mask        m_enabled_groups;

	std::unordered_map<std::string, unsigned> m_group_ids;

	std::vector<group_mask> m_state_stack;

	stx::injector* m_injector;

	void _updateEnabled(entry& e);
};

} // namespace stx

// =============================================================
// == Inline implementation =====================================
// =============================================================

namespace stx {

inline injector& system_configuration::inject() { return manager.inject(); }

template<class Arg1, class... ArgN>
inline void system_configuration::inject(Arg1&& arg1, ArgN&&... argN) { inject()(std::forward<Arg1>(arg1), std::forward<ArgN>(argN)...); }

} // namespace stx