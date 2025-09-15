#ifndef __COS_STL_ARRAY_HPP
#define __COS_STL_ARRAY_HPP

#include <types.hpp>

namespace std {

template <typename T, std::size_t Count>
struct array {
private:
	T data[Count] = {};

public:
	[[nodiscard]] T& operator[](std::size_t index) noexcept { return data[index]; }

	[[nodiscard]] const T& operator[](std::size_t index) const noexcept { return data[index]; }
};

}  // namespace std

#endif
