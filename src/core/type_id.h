#pragma once
#include <cstddef>

using TypeId = size_t;

namespace detail
{
inline size_t nextTypeId()
{
    static size_t s_counter = 0;
    return s_counter++;
}
}  // namespace detail

/**
 * @brief Returns a unique, stable id for type T.
 * @tparam T Type to get an id for.
 * @return Same value on every call for this T; different from every
 * other T's id.
 *
 * Per-type id without RTTI: no typeid, no std::type_index, no
 * dynamic_cast anywhere in the project. Each template instantiation
 * gets its own function-local static, initialized exactly once
 * (Meyers-singleton style), so the counter never hands out the same
 * id twice.
 */
template <typename T>
TypeId getTypeId()
{
    static const TypeId s_id = detail::nextTypeId();
    return s_id;
}
