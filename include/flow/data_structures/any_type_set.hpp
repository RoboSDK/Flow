#pragma once

#include <any>
#include <functional>

namespace flow {
/**
 * The purpose of this data structure is in the scenario where you have at least two locations in code
 * where a user passes type information as an argument, but you don't want to pass this type around everywhere.
 *
 * This stores any type by its std::type_info
 *
 * example:
 * my_set.put(1.0); // stored a double
 */
class any_type_set {
  using typeinfo_ref = std::reference_wrapper<const std::type_info>;

public:
  template<typename T>
  bool contains() const
  {
    const auto info = typeinfo_ref(typeid(T));
    return m_items.find(info) != m_items.end();
  }

  template<typename T>
  void put(T&& t)
  {
    m_items[typeid(T)] = std::forward<T>(t);
  }

  template<typename T>
  auto& at()
  {
    return std::any_cast<T&>(m_items.at(typeid(T)));
  }

private:
  struct hasher {
    std::size_t operator()(typeinfo_ref code) const
    {
      return code.get().hash_code();
    }
  };

  struct compare_typeinfo {
    bool operator()(typeinfo_ref lhs, typeinfo_ref rhs) const
    {
      return lhs.get() == rhs.get();
    }
  };

  std::unordered_map<typeinfo_ref, std::any, hasher, compare_typeinfo> m_items{};
};
}// namespace flow
