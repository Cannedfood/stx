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

// Virtual setter
#define xsetterv(ACCESSOR, MEMBER)                \
	virtual auto& ACCESSOR(decltype(MEMBER) const & m) { \
		MEMBER = m;                              \
		return *this;                            \
	}

#define xaccessv(ACCESSOR, MEMBER) \
	xgetter(ACCESSOR, MEMBER) xsetterv(ACCESSOR, MEMBER)

#define xproperty_r(TYPE, NAME) TYPE NAME();
#define xproperty_w(TYPE, NAME) void NAME(TYPE const&);
#define xproperty(TYPE, NAME) xproperty_r(TYPE, NAME); xproperty_w(TYPE, NAME)
