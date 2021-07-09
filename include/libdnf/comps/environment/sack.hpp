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


#ifndef LIBDNF_COMPS_ENVIRONMENT_SACK_HPP
#define LIBDNF_COMPS_ENVIRONMENT_SACK_HPP

#include "libdnf/common/sack/sack.hpp"
#include "libdnf/common/weak_ptr.hpp"
#include "libdnf/transaction/transaction_item_reason.hpp"

#include <map>
#include <memory>
#include <string>


namespace libdnf::comps {


class Comps;

class Environment;

class EnvironmentQuery;

class EnvironmentSack;
using EnvironmentSackWeakPtr = WeakPtr<EnvironmentSack, false>;


class EnvironmentSack : public libdnf::sack::Sack<Environment> {
public:
    ~EnvironmentSack();

    /// Create WeakPtr to EnvironmentSack
    EnvironmentSackWeakPtr get_weak_ptr();

    const std::map<std::string, libdnf::transaction::TransactionItemReason> & get_reasons() const { return reasons; }

protected:
    explicit EnvironmentSack(Comps & comps);

private:
    Comps & comps;

    class Impl;
    std::unique_ptr<Impl> p_impl;

    /// @return Map of resolved reasons why environments were installed: ``{environment_id -> reason}``.
    ///         An environment can be installed due to multiple reasons, only the most significant is returned.
    /// @since 5.0
    std::map<std::string, libdnf::transaction::TransactionItemReason> reasons;

    friend Comps;
    friend EnvironmentQuery;
    friend Environment;
};


}  // namespace libdnf::comps


#endif  // LIBDNF_COMPS_ENVIRONMENT_SACK_HPP
