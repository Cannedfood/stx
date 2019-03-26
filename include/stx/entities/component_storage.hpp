#pragma once

#include <cstddef>
#include <vector>
#include <memory>
#include <vector>

namespace stx::ecs {

class component_storage_interface {
public:
	virtual ~component_storage_interface() {}


	virtual void* getTypeErased    (size_t index) = 0;
	virtual void  destroyTypeErased(size_t index) = 0;

	struct statistics_t {
		size_t total_slots  = 0;
		size_t used_slots   = 0;
		size_t total_memory = 0;
		size_t used_memory  = 0;
	};
	virtual statistics_t statistics() = 0;
};

template<class T, size_t ElementsPerBlock = 524288 / sizeof(T)>
class component_storage final : public component_storage_interface {
	constexpr size_t aligned_size() const noexcept {
		return sizeof(T); // TODO: make sure multiples of the size are still aligned
	}

	void* getTypeErased(size_t index)     override { return get(index); }
	void  destroyTypeErased(size_t index) override { return destroy(index); }
public:
	component_storage() {}
	~component_storage() {}

	T*   get  (size_t index) {
		size_t block   = index / ElementsPerBlock; // Optimizes to right shift (hopefully)
		size_t element = index % ElementsPerBlock; // Optimizes to bitwise-and
		return reinterpret_cast<T*>(m_blocks[block].get() + element * aligned_size());
	}

	template<class... Args>
	T*   create(size_t index, Args&&... args) {
		_increment_usage(index);
		return new(get(index)) T(std::forward<Args>(args)...);;
	}
	void destroy(size_t index) {
		get(index)->~T();
		_decrement_usage(index);
	}
	void moveToEmpty(size_t from, size_t to) {
		T* pTo   = get(to);
		T* pFrom = get(from);
		new(pTo) T(std::move(*pFrom));
		pFrom->~T();
	}
	void moveAndOverride(size_t from, size_t to) {
		destroy(to);
		moveToEmpty(from, to);
	}

	// Statistics
	statistics_t statistics() override {
		statistics_t result {};

		for(auto& b : m_blocks) {
			if(b) result.total_slots += ElementsPerBlock;
		}
		for(auto u : m_usages) result.used_slots += u;

		result.total_memory = result.total_slots * aligned_size();
		result.used_memory  = result.used_slots  * aligned_size();

		return result;
	}

private:
	uint8_t* _increment_usage(size_t index) {
		size_t block = index / ElementsPerBlock;

		if(m_usages.size() <= block) {
			m_usages.resize(block + 1);
			m_blocks.resize(block + 1);
		}

		uint32_t usages = ++m_usages[block];
		if(usages == 1) {
			m_blocks[block].reset(new uint8_t[ElementsPerBlock * aligned_size()]);
		}
		return m_blocks[block].get();
	}

	void _decrement_usage(size_t index) {
		size_t block = index / ElementsPerBlock;

		if(--m_usages[block] == 0) {
			m_blocks[block].reset();
		}
	}

	std::vector<std::unique_ptr<uint8_t[]>> m_blocks;
	std::vector<uint32_t>                   m_usages;
};

} // namespace stx::ecs