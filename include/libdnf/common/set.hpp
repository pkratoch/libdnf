/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef LIBDNF_UTILS_SET_HPP
#define LIBDNF_UTILS_SET_HPP

#include <algorithm>
#include <set>


namespace libdnf {

/// Set represents set of objects (e.g. repositories, or groups)
/// and implements set operations such as unions or differences.
template <typename T>
class Set {
public:
    Set() = default;
    Set(const Set<T> & other) : data(other.data) {}
    Set(Set<T> && other) : data(std::move(other.data)) {}
    Set(std::initializer_list<T> ilist) : data(ilist) {}
    ~Set() = default;

    // GENERIC OPERATIONS
    bool empty() const noexcept { return data.empty(); };
    std::size_t size() const noexcept { return data.size(); }
    void clear() noexcept { data.clear(); }

    // ITEM OPERATIONS
    void add(const T & obj) { data.insert(obj); }
    void add(T && obj) { data.insert(std::move(obj)); }
    void remove(const T & obj) { data.erase(obj); }
    bool contains(const T & obj) const { return data.find(obj) != data.end(); }

    // SET OPERATIONS
    Set<T> & operator=(const Set<T> & other);
    Set<T> & operator=(Set<T> && other) noexcept;
    Set<T> & operator=(std::initializer_list<T> ilist);
    Set<T> & operator|=(const Set<T> & other);
    Set<T> & operator&=(const Set<T> & other);
    Set<T> & operator-=(const Set<T> & other);
    Set<T> & operator^=(const Set<T> & other);

    // update == union
    void update(const Set<T> & other) { *this |= other; }
    void intersection(const Set<T> & other) { *this &= other; }
    void difference(const Set<T> & other) { *this -= other; }
    void symmetric_difference(const Set<T> & other) { *this ^= other; }
    bool is_subset(const Set<T> & other) const;
    bool is_superset(const Set<T> & other) const;

    void swap(Set<T> & other) noexcept { data.swap(other.data); }

    // TODO(jrohel): Temporary solution. Implement iterator and other stuff and then remove this hack.
    /// Return reference to underlying std::set
    const std::set<T> & get_data() const noexcept { return data; }
    std::set<T> & get_data() noexcept { return data; }

private:
    friend bool operator==(const Set<T> & lhs, const Set<T> & rhs) { return lhs.data == rhs.data; }

    // TODO(jrohel): Performance? May be changed to std::unordered_set in future. Or container as template parameter.
    std::set<T> data;
};

template <typename T>
inline Set<T> & Set<T>::operator=(const Set<T> & other) {
    data = other.data;
    return *this;
}

template <typename T>
inline Set<T> & Set<T>::operator=(Set<T> && other) noexcept {
    data = std::move(other.data);
    return *this;
}

template <typename T>
inline Set<T> & Set<T>::operator=(std::initializer_list<T> ilist) {
    data = ilist;
    return *this;
}

template <typename T>
inline Set<T> & Set<T>::operator|=(const Set<T> & other) {
    std::set<T> result;
    std::set_union(
        data.begin(), data.end(), other.data.begin(), other.data.end(), std::inserter(result, result.begin()));
    data = result;
    return *this;
}

template <typename T>
inline Set<T> & Set<T>::operator&=(const Set<T> & other) {
    std::set<T> result;
    std::set_intersection(
        data.begin(), data.end(), other.data.begin(), other.data.end(), std::inserter(result, result.begin()));
    data = std::move(result);
    return *this;
}

template <typename T>
inline Set<T> & Set<T>::operator-=(const Set<T> & other) {
    std::set<T> result;
    std::set_difference(
        data.begin(), data.end(), other.data.begin(), other.data.end(), std::inserter(result, result.begin()));
    data = result;
    return *this;
}

template <typename T>
inline Set<T> & Set<T>::operator^=(const Set<T> & other) {
    std::set<T> result;
    std::set_symmetric_difference(
        data.begin(), data.end(), other.data.begin(), other.data.end(), std::inserter(result, result.begin()));
    data = result;
    return *this;
}

template <typename T>
inline bool Set<T>::is_subset(const Set<T> & other) const {
    return std::includes(other.data.begin(), other.data.end(), data.begin(), data.end());
}

template <typename T>
inline bool Set<T>::is_superset(const Set<T> & other) const {
    return std::includes(data.begin(), data.end(), other.data.begin(), other.data.end());
}

template <typename T>
inline Set<T> operator|(const Set<T> & lhs, const Set<T> & rhs) {
    Set<T> ret(lhs);
    return ret |= rhs;
}

template <typename T>
inline Set<T> operator&(const Set<T> & lhs, const Set<T> & rhs) {
    Set<T> ret(lhs);
    return ret &= rhs;
}

template <typename T>
inline Set<T> operator-(const Set<T> & lhs, const Set<T> & rhs) {
    Set<T> ret(lhs);
    return ret -= rhs;
}

template <typename T>
inline Set<T> operator^(const Set<T> & lhs, const Set<T> & rhs) {
    Set<T> ret(lhs);
    return ret ^= rhs;
}

}  // namespace libdnf

#endif
