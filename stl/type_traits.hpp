#ifndef STL_TYPE_TRAITS_HPP
#define STL_TYPE_TRAITS_HPP

namespace std {
template <class T, T v>
struct integral_constant {
	static constexpr T value = v;
	using value_type = T;
	using type = integral_constant;
	constexpr operator value_type() const noexcept { return value; }
	constexpr value_type operator()() const noexcept { return value; }
};

using false_type = integral_constant<bool, false>;
using true_type = integral_constant<bool, true>;

template <class T, class U>
struct is_same : false_type {};

template <class T>
struct is_same<T, T> : true_type {};

}  // namespace std

#endif
