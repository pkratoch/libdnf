#include "libdnf/base/base.hpp"
#include "libdnf/comps/environment/query.hpp"
#include "libdnf/comps/environment/query_impl.hpp"
#include "libdnf/comps/environment/environment-private.hpp"
#include "libdnf/comps/environment/sack.hpp"
#include "libdnf/comps/comps.hpp"
#include "libdnf/comps/comps_impl.hpp"

extern "C" {
#include <solv/pool.h>
#include <solv/repo.h>
}

namespace libdnf::comps {


EnvironmentQueryWeakPtr EnvironmentQuery::get_weak_ptr() {
    return EnvironmentQueryWeakPtr(this, &p_impl->data_guard);
}

EnvironmentQuery::EnvironmentQuery(const EnvironmentSackWeakPtr & sack)
    : Query()
    , sack(sack)
    , p_impl{new Impl()}
{
    Pool * pool = sack->comps.p_impl->get_pool();
    std::map<std::string, std::vector<Id>> environment_map;
    Id solvable_id;
    std::string solvable_name;
    std::string environmentid;

    // Loop over all solvables
    FOR_POOL_SOLVABLES(solvable_id) {
        // Do not include solvables from disabled repositories
        // TODO(pkratoch): Test this works
        if (pool_id2solvable(pool, solvable_id)->repo->disabled) {
            continue;
        }
        // SOLVABLE_NAME is in a form "type:id"; include only solvables of type "environment"
        solvable_name = pool_lookup_str(pool, solvable_id, SOLVABLE_NAME);
        auto delimiter_position = solvable_name.find(":");
        if (solvable_name.substr(0, delimiter_position) != "environment") {
            continue;
        }
        // Map environmentids with list of corresponding solvable_ids
        // TODO(pkratoch): Sort solvable_ids for each environmentid according to something (repo priority / repo id / ?)
        environmentid = solvable_name.substr(delimiter_position, std::string::npos);
        if (strcmp(pool_id2solvable(pool, solvable_id)->repo->name, "@System")) {
            environmentid.append("_available");
        } else {
            environmentid.append("_installed");
        }
        if (environment_map.find(environmentid) == environment_map.end()) {
            std::vector<Id> solvable_ids;
            environment_map.emplace(environmentid, solvable_ids);
        }
        environment_map[environmentid].insert(environment_map[environmentid].begin(), solvable_id);
    }

    // Create environments based on the environment_map
    for (auto it = environment_map.begin(); it != environment_map.end(); it++) {
        Environment environment(this);
        add_solvable_ids(environment, it->second);
        add(environment);
    }
}

EnvironmentQuery::EnvironmentQuery(const BaseWeakPtr & base): EnvironmentQuery(base->get_comps()->get_environment_sack()) {}

EnvironmentQuery::EnvironmentQuery(Base & base): EnvironmentQuery(base.get_comps()->get_environment_sack()) {}

EnvironmentQuery::EnvironmentQuery(const EnvironmentQuery & query)
    : Query(query)
    , sack(query.sack)
    , p_impl{new Impl()}
{}


EnvironmentQuery::~EnvironmentQuery() {}


}  // namespace libdnf::comps

