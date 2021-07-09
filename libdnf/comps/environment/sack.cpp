#include "libdnf/comps/environment/sack.hpp"
#include "libdnf/comps/environment/sack_impl.hpp"
#include "libdnf/comps/environment/environment.hpp"
#include "libdnf/comps/environment/environment-private.hpp"
#include "libdnf/comps/environment/query.hpp"
#include "libdnf/comps/comps.hpp"
#include "libdnf/comps/comps_impl.hpp"

extern "C" {
#include <solv/knownid.h>
#include <solv/pool.h>
#include <solv/repo.h>
#include <solv/solvable.h>
}

#include <map>


namespace libdnf::comps {


EnvironmentSackWeakPtr EnvironmentSack::get_weak_ptr() {
    return EnvironmentSackWeakPtr(this, &p_impl->sack_guard);
}


EnvironmentSack::EnvironmentSack(Comps & comps)
    : Sack()
    , comps{comps}
    , p_impl{new Impl()}
{}


EnvironmentSack::~EnvironmentSack() {}


}  // namespace libdnf::comps
