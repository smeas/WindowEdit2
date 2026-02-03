#pragma once
#include <utility>

namespace sk::defer_
{

template <typename T>
struct defer_helper_
{
	T lambda;
	defer_helper_(T&& lambda) : lambda(std::move(lambda)) { }
	~defer_helper_() { lambda(); }
};

template <typename T>
defer_helper_<T> create_defer_helper_(T&& lambda) { return defer_helper_<T>{std::forward<T>(lambda)}; }

}

#define SK_DEFER_TOKENPASTE_(x, y) x ## y
#define SK_DEFER_TOKENPASTE2_(x, y) SK_DEFER_TOKENPASTE_(x, y)

#define SK_DEFER(...) auto SK_DEFER_TOKENPASTE2_(_defer_helper_var_, __COUNTER__) = ::sk::defer_::create_defer_helper_([&](){__VA_ARGS__;})
