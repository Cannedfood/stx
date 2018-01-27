#ifndef STX_CACHE_HPP_INCLUDED
#define STX_CACHE_HPP_INCLUDED

#pragma once

#include <string>
#include <map>
#include <memory>

namespace stx {

template<class Value, class LoadInfo = std::string>
class cache {
	std::map<LoadInfo, std::weak_ptr<Value>> mCache;
public:
	template<class Loader>
	std::shared_ptr<Value> getOrLoad(LoadInfo const& li, Loader&& l) {
		auto& entry = mCache[li];
		std::shared_ptr<Value> result = entry.lock();
		if(!result) {
			result = l(li);
			entry  = result;
		}
		return result;
	}
};

} // namespace stx

#endif // header guard STX_CACHE_HPP_INCLUDED
