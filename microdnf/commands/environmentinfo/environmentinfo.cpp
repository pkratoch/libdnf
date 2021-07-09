/*
Copyright (C) 2021 Red Hat, Inc.

This file is part of microdnf: https://github.com/rpm-software-management/libdnf/

Microdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Microdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with microdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "environmentinfo.hpp"

#include "../../context.hpp"

#include <libdnf/comps/comps.hpp>
#include <libdnf/comps/environment/environment.hpp>
#include <libdnf/comps/environment/query.hpp>
#include <libdnf/conf/option_string.hpp>
#include "libdnf-cli/output/environmentinfo.hpp"

#include <filesystem>
#include <fstream>
#include <set>


namespace microdnf {


using namespace libdnf::cli;


EnvironmentinfoCommand::EnvironmentinfoCommand(Command & parent) : Command(parent, "environmentinfo") {
    auto & ctx = static_cast<Context &>(get_session());
    auto & parser = ctx.get_argument_parser();
    auto & cmd = *get_argument_parser_command();

    available_option = dynamic_cast<libdnf::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf::OptionBool>(new libdnf::OptionBool(false))));

    installed_option = dynamic_cast<libdnf::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf::OptionBool>(new libdnf::OptionBool(false))));

    auto available = parser.add_new_named_arg("available");
    available->set_long_name("available");
    available->set_short_description("show only available environments");
    available->set_const_value("true");
    available->link_value(available_option);

    auto installed = parser.add_new_named_arg("installed");
    installed->set_long_name("installed");
    installed->set_short_description("show only installed environments");
    installed->set_const_value("true");
    installed->link_value(installed_option);

    patterns_to_show_options = parser.add_new_values();
    auto keys = parser.add_new_positional_arg(
        "environments_to_show",
        ArgumentParser::PositionalArg::UNLIMITED,
        parser.add_init_value(std::unique_ptr<libdnf::Option>(new libdnf::OptionString(nullptr))),
        patterns_to_show_options);
    keys->set_short_description("List of environments to show");

    auto conflict_args =
        parser.add_conflict_args_group(std::unique_ptr<std::vector<ArgumentParser::Argument *>>(
            new std::vector<ArgumentParser::Argument *>{available, installed}));

    available->set_conflict_arguments(conflict_args);
    installed->set_conflict_arguments(conflict_args);

    cmd.set_short_description("Print details about comps environments");
    cmd.set_parse_hook_func([this, &ctx](
                                [[maybe_unused]] ArgumentParser::Argument * arg,
                                [[maybe_unused]] const char * option,
                                [[maybe_unused]] int argc,
                                [[maybe_unused]] const char * const argv[]) {
        ctx.set_selected_command(this);
        return true;
    });

    cmd.register_named_arg(available);
    cmd.register_named_arg(installed);
    cmd.register_positional_arg(keys);
}


void EnvironmentinfoCommand::run() {
    std::vector<std::string> patterns_to_show;
    if (patterns_to_show_options->size() > 0) {
        patterns_to_show.reserve(patterns_to_show_options->size());
        for (auto & pattern : *patterns_to_show_options) {
            auto option = dynamic_cast<libdnf::OptionString *>(pattern.get());
            patterns_to_show.emplace_back(option->get_value());
        }
    }

    // Load environment sack
    // TODO(pkratoch): use comps from base and real repositories
    std::unique_ptr<libdnf::Base> base = std::make_unique<libdnf::Base>();
    libdnf::comps::Comps comps(*base.get());
    comps.load_installed();
    std::filesystem::path data_path = PROJECT_SOURCE_DIR "/test/libdnf/comps/data/";
    const char * reponame = "repo";
    comps.load_from_file(data_path / "core.xml", reponame);
    comps.load_from_file(data_path / "core.xml", "repo2");
    comps.load_from_file(data_path / "standard.xml", reponame);
    comps.load_from_file(data_path / "minimal-environment.xml", reponame);
    comps.load_from_file(data_path / "custom-environment.xml", reponame);

    libdnf::comps::EnvironmentQuery query(comps.get_environment_sack());

    // Filter by patterns if given
    if (patterns_to_show.size() > 0) {
        auto query_names = libdnf::comps::EnvironmentQuery(query);
        query.filter_environmentid(patterns_to_show, libdnf::sack::QueryCmp::IGLOB);
        query |= query_names.filter_name(patterns_to_show, libdnf::sack::QueryCmp::IGLOB);
    }

    std::set<libdnf::comps::Environment> environment_list;

    auto query_installed = libdnf::comps::EnvironmentQuery(query);
    query_installed.filter_installed(true);

    // --installed -> filter installed environments
    if (installed_option->get_value()) {
        environment_list = query_installed.list();
    // --available / all
    } else {
        // all -> first add installed environments to the list
        if (!available_option->get_value()) {
            for (auto environment: query_installed.list()) {
                environment_list.emplace(environment);
            }
        }
        // --available / all -> add available not-installed environments into the list
        auto query_available = libdnf::comps::EnvironmentQuery(query);
        query_available.filter_installed(false);
        std::set<std::string> installed_environmentids;
        for (auto environment: query_installed.list()) {
            installed_environmentids.insert(environment.get_environmentid());
        }
        for (auto environment: query_available.list()) {
            if (installed_environmentids.find(environment.get_environmentid()) == installed_environmentids.end()) {
                environment_list.emplace(environment);
            }
        }
    }

    for (auto environment : environment_list) {
        libdnf::cli::output::print_environmentinfo_table(environment);
        std::cout << '\n';
    }
}

}  // namespace microdnf
