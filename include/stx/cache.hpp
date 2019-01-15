#ifndef STX_CACHE_HPP_INCLUDED
#define STX_CACHE_HPP_INCLUDED

#pragma once

#include <string>
#include <map>
#include <memory>

#include "shared.hpp"

namespace stx {

template<class Value, class LoadInfo = std::string>
class cache {
	std::map<LoadInfo, stx::weak<Value>> mCache;
public:
	template<class Loader>
	stx::shared<Value> getOrLoad(LoadInfo const& li, Loader&& l) {
		auto& entry = mCache[li];
		stx::shared<Value> result = entry.lock();
		if(!result) {
			result = l(li);
			entry  = result;
		}
		return result;
	}

	void overload(LoadInfo const& li, stx::shared<Value> v) {
		mCache[li] = v;
	}

	stx::shared<Value> get(LoadInfo const& li) {
		return mCache[li].lock();
	}
};

} // namespace stx

#endif // header guard STX_CACHE_HPP_INCLUDED
