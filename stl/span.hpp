#ifndef __COS_STL_SPAN_HPP
#define __COS_STL_SPAN_HPP

#include <types.hpp>

namespace std {

template <typename ViewedT>
struct span {
public:
	span(ViewedT *data, std::size_t count) : begin(data), end(data + count) {}

	[[nodiscard]] ViewedT &operator[](std::size_t index) noexcept { return *(begin + index); }

	[[nodiscard]] const ViewedT &operator[](std::size_t index) const noexcept { return *(begin + index); }

	[[nodiscard]] std::size_t size() const noexcept { return static_cast<std::size_t>(end - begin); }

	[[nodiscard]] std::size_t size_bytes() const noexcept { return size() * sizeof(ViewedT); }

	[[nodiscard]] ViewedT *data() const noexcept { return begin; }

private:
	ViewedT *begin;
	ViewedT *end;
};

}  // namespace std

#endif
