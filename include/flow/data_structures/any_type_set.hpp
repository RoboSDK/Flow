#pragma once

#include <any>
#include <functional>

namespace flow {
class any_type_set {
  using typeinfo_ref = std::reference_wrapper<const std::type_info>;

public:
  template<typename T>
  bool contains(typeinfo_ref info)
  {
    return m_items.find(info) != m_items.end();
  }

  template<typename T>
  auto& put()
  {
    return std::any_cast<T&>(m_items[typeid(T)]);
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
