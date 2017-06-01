#pragma once

#define xgetter(ACCESSOR, MEMBER) \
	auto const& ACCESSOR() const noexcept { return MEMBER; }

#define xsetter(ACCESSOR, MEMBER)                \
	auto& ACCESSOR(decltype(MEMBER) const & m) { \
		MEMBER = m;                              \
		return *this;                            \
	}                                            \
                                                 \
	auto& ACCESSOR(decltype(MEMBER)&& m) {       \
		MEMBER = std::move(m);                   \
		return *this;                            \
	}

#define xaccess(ACCESSOR, MEMBER) \
	xgetter(ACCESSOR, MEMBER) xsetter(ACCESSOR, MEMBER)
