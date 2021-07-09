/*
Copyright (C) 2021 Red Hat, Inc.

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


#ifndef LIBDNF_COMPS_ENVIRONMENT_QUERY_HPP
#define LIBDNF_COMPS_ENVIRONMENT_QUERY_HPP

#include "libdnf/base/base_weak.hpp"
#include "libdnf/common/sack/query.hpp"
#include "libdnf/common/weak_ptr.hpp"
#include "libdnf/comps/environment/environment.hpp"

#include <memory>

namespace libdnf {


}  // namespace libdnf


namespace libdnf::comps {


class EnvironmentQuery;
using EnvironmentQueryWeakPtr = WeakPtr<EnvironmentQuery, false>;

class EnvironmentSack;
using EnvironmentSackWeakPtr = WeakPtr<EnvironmentSack, false>;


class EnvironmentQuery : public libdnf::sack::Query<Environment> {
public:
    // Create new query with newly composed environments (using only solvables from currently enabled repositories)
    explicit EnvironmentQuery(const EnvironmentSackWeakPtr & sack);
    explicit EnvironmentQuery(const libdnf::BaseWeakPtr & base);
    explicit EnvironmentQuery(libdnf::Base & base);

    EnvironmentQuery(const EnvironmentQuery & query);
    ~EnvironmentQuery();

    EnvironmentQuery & filter_environmentid(const std::string & pattern, sack::QueryCmp cmp = libdnf::sack::QueryCmp::EQ);
    EnvironmentQuery & filter_environmentid(
        const std::vector<std::string> & patterns, sack::QueryCmp cmp = libdnf::sack::QueryCmp::EQ);
    EnvironmentQuery & filter_name(const std::string & pattern, sack::QueryCmp cmp = libdnf::sack::QueryCmp::EQ);
    EnvironmentQuery & filter_name(
        const std::vector<std::string> & patterns, sack::QueryCmp cmp = libdnf::sack::QueryCmp::EQ);
    EnvironmentQuery & filter_installed(bool value);

    /// Create WeakPtr to EnvironmentQuery
    EnvironmentQueryWeakPtr get_weak_ptr();

protected:
    EnvironmentQuery(EnvironmentQuery && query);

private:
    struct F {
        static std::string environmentid(const Environment & obj) { return obj.get_environmentid(); }
        static std::string name(const Environment & obj) { return obj.get_name(); }
        static bool is_installed(const Environment & obj) { return obj.get_installed(); }
    };

    EnvironmentSackWeakPtr sack;

    class Impl;
    std::unique_ptr<Impl> p_impl;

    friend Environment;
    friend EnvironmentSack;
};


inline EnvironmentQuery & EnvironmentQuery::filter_environmentid(const std::string & pattern, sack::QueryCmp cmp) {
    filter(F::environmentid, pattern, cmp);
    return *this;
}


inline EnvironmentQuery & EnvironmentQuery::filter_environmentid(const std::vector<std::string> & patterns, sack::QueryCmp cmp) {
    filter(F::environmentid, patterns, cmp);
    return *this;
}


inline EnvironmentQuery & EnvironmentQuery::filter_name(const std::string & pattern, sack::QueryCmp cmp) {
    filter(F::name, pattern, cmp);
    return *this;
}


inline EnvironmentQuery & EnvironmentQuery::filter_name(const std::vector<std::string> & patterns, sack::QueryCmp cmp) {
    filter(F::name, patterns, cmp);
    return *this;
}


inline EnvironmentQuery & EnvironmentQuery::filter_installed(bool value) {
    filter(F::is_installed, value, sack::QueryCmp::EQ);
    return *this;
}


}  // namespace libdnf::comps

#endif  // LIBDNF_COMPS_ENVIRONMENT_QUERY_HPP
