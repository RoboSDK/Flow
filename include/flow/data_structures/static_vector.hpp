#include <experimental/fixed_capacity_vector>

namespace flow {
template <typename T, std::size_t N>
using static_vector = std::experimental::fixed_capacity_vector<T, N>;
}
