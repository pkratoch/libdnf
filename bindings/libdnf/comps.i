#if defined(SWIGPYTHON)
%module(package="libdnf") comps
#elif defined(SWIGPERL)
%module "libdnf::comps"
#elif defined(SWIGRUBY)
%module "libdnf/comps"
#endif

%include <exception.i>
%include <std_string.i>
%include <std_vector.i>

%import "common.i"

%{
    #include "libdnf/comps/group/package.hpp"
    #include "libdnf/comps/group/group.hpp"
    #include "libdnf/comps/group/query.hpp"
    #include "libdnf/comps/group/sack.hpp"
    #include "libdnf/comps/environment/environment.hpp"
    #include "libdnf/comps/environment/query.hpp"
    #include "libdnf/comps/environment/sack.hpp"
    #include "libdnf/comps/comps.hpp"
%}

#define CV __perl_CV

%template(SackQueryGroup) libdnf::sack::Query<libdnf::comps::Group>;
%template(SackQueryEnvironment) libdnf::sack::Query<libdnf::comps::Environment>;

%include "libdnf/comps/group/package.hpp"
%include "libdnf/comps/group/group.hpp"
%include "libdnf/comps/group/query.hpp"
%include "libdnf/comps/group/sack.hpp"
%include "libdnf/comps/environment/environment.hpp"
%include "libdnf/comps/environment/query.hpp"
%include "libdnf/comps/environment/sack.hpp"
%include "libdnf/comps/comps.hpp"
