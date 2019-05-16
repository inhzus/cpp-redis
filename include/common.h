//
// Created by suun on 5/16/19.
//

#ifndef REDIS_COMMON_H
#define REDIS_COMMON_H
#include <memory>
#include <experimental/memory_resource>
// For std::variant
#include <variant>

namespace rd {
typedef size_t size_type;
template<class T>
using Allocator =
std::experimental::pmr::polymorphic_allocator<T>;
using Var = std::variant<int, float>;
}
#endif //REDIS_COMMON_H
