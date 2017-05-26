#pragma once

#define xgetter(ACCESSOR, MEMBER) \
	inline auto const& ACCESSOR() const noexcept { return MEMBER; }

#define xsetter(ACCESSOR, MEMBER)                 \
	inline auto& ACCESSOR(decltype(MEMBER)& m) {  \
		MEMBER = m;                               \
		return *this;                             \
	}                                             \
                                                  \
	inline auto& ACCESSOR(decltype(MEMBER)&& m) { \
		MEMBER = std::move(m);                    \
		return *this;                             \
	}

#define xaccess(ACCESSOR, MEMBER) \
	xgetter(ACCESSOR, MEMBER) xsetter(ACCESSOR, MEMBER)
