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


#ifndef LIBDNF_COMPS_ENVIRONMENT_ENVIRONMENT_HPP
#define LIBDNF_COMPS_ENVIRONMENT_ENVIRONMENT_HPP

#include "libdnf/common/weak_ptr.hpp"
#include "libdnf/comps/group/group.hpp"
#include "libdnf/transaction/transaction_item_reason.hpp"

#include <set>
#include <string>
#include <vector>


namespace libdnf::comps {


struct EnvironmentId {
public:
    EnvironmentId() = default;
    explicit EnvironmentId(int id);

    bool operator==(const EnvironmentId & other) const noexcept { return id == other.id; };
    bool operator!=(const EnvironmentId & other) const noexcept { return id != other.id; };

    // Corresponds to solvable id
    int id{0};
};


inline EnvironmentId::EnvironmentId(int id) : id(id) {}


class EnvironmentQuery;
using EnvironmentQueryWeakPtr = WeakPtr<EnvironmentQuery, false>;


/// @replaces dnf:dnf/comps.py:class:Environment
class Environment {
public:
    ~Environment();

    const std::vector<EnvironmentId> & get_environment_ids() const { return environment_ids; }
    void add_environment_id(EnvironmentId environment_id) { environment_ids.push_back(environment_id); }

    /// Get environment id
    std::string get_environmentid() const;

    /// Get environment name
    std::string get_name() const;

    /// Get environment description
    std::string get_description() const;

    /// Get translated name of a environment based on current locales.
    /// If a translation is not found, return untranslated name.
    ///
    /// @replaces dnf:dnf/comps.py:attribute:Environment.ui_name
    std::string get_translated_name(const char * lang) const;
    std::string get_translated_name() const;

    /// Get translated description of a environment based on current locales.
    /// If a translation is not found, return untranslated description.
    ///
    /// @replaces dnf:dnf/comps.py:attribute:Environment.ui_description
    std::string get_translated_description(const char * lang) const;
    std::string get_translated_description() const;

    /// Get environment order
    std::string get_order() const;

    std::vector<std::string> get_groups();
    std::vector<std::string> get_optional_groups();

    std::set<std::string> get_repos() const;

    /// Determine if environment is installed
    /// If it belongs to the @System repo, return true
    bool get_installed() const;

    /// @return Resolved reason why an environment was installed.
    ///         An environment can be installed due to multiple reasons, only the most significant is returned.
    /// @since 5.0
    //
    // TODO(dmach): return actual value from data in EnvironmentSack
    // TODO(dmach): throw an exception when getting a reason for an available package (it should work only for installed)
    libdnf::transaction::TransactionItemReason get_reason() const { return libdnf::transaction::TransactionItemReason::UNKNOWN; }

    /// Merge a comps environment with another one
    Environment & operator+=(const Environment & rhs);

    bool operator<(const Environment & rhs) const {
        return get_environmentid() < rhs.get_environmentid() or (get_installed() and !rhs.get_installed());
    }

    void dump(const std::string & path);

protected:
    explicit Environment(EnvironmentQuery * query);

private:
    EnvironmentQueryWeakPtr query;

    // Corresponds to solvable ids for this environment (libsolv doesn't merge environments,
    // so there are multiple solvables for one environmentid)
    std::vector<EnvironmentId> environment_ids;

    std::vector<std::string> groups;
    std::vector<std::string> optional_groups;

    friend class EnvironmentSack;
    friend class EnvironmentQuery;
};


}  // namespace libdnf::comps


#endif  // LIBDNF_COMPS_ENVIRONMENT_ENVIRONMENT_HPP
