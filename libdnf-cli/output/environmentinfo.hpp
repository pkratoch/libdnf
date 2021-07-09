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


#ifndef LIBDNF_CLI_OUTPUT_ENVIRONMENTINFO_HPP
#define LIBDNF_CLI_OUTPUT_ENVIRONMENTINFO_HPP

#include "libdnf-cli/utils/tty.hpp"

#include <libsmartcols/libsmartcols.h>

#include <string.h>


namespace libdnf::cli::output {


static void add_line_into_environmentinfo_table(
    struct libscols_table * table, const char * key, const char * value, const char * color) {
    struct libscols_line * ln = scols_table_new_line(table, NULL);
    scols_line_set_data(ln, 0, key);
    scols_line_set_data(ln, 1, value);

    if (color && strcmp(color, "") != 0) {
        auto cell_value = scols_line_get_cell(ln, 1);
        scols_cell_set_color(cell_value, color);
    }
}


static void add_line_into_environmentinfo_table(
    struct libscols_table * table, const char * key, const char * value) {
    add_line_into_environmentinfo_table(table, key, value, "");
}


static struct libscols_table * create_environmentinfo_table(libdnf::comps::Environment & environment) {
    struct libscols_table * table = scols_new_table();
    scols_table_enable_noheadings(table, 1);
    scols_table_set_column_separator(table, " : ");
    scols_table_new_column(table, "key", 5, SCOLS_FL_TREE);
    struct libscols_column * cl = scols_table_new_column(table, "value", 10, SCOLS_FL_WRAP);
    scols_column_set_safechars(cl, "\n");
    scols_column_set_wrapfunc(cl, scols_wrapnl_chunksize, scols_wrapnl_nextchunk, nullptr);
    if (libdnf::cli::utils::tty::is_interactive()) {
        scols_table_enable_colors(table, true);
    }
    auto sy = scols_new_symbols();
    scols_symbols_set_branch(sy, "  ");
    scols_symbols_set_right(sy, "  ");
    scols_symbols_set_vertical(sy, "");
    scols_table_set_symbols(table, sy);

    add_line_into_environmentinfo_table(table, "Environment id", environment.get_environmentid().c_str(), "bold");
    add_line_into_environmentinfo_table(table, "Name", environment.get_name().c_str());
    add_line_into_environmentinfo_table(table, "Description", environment.get_description().c_str());
    add_line_into_environmentinfo_table(table, "Order", environment.get_order().c_str());
    add_line_into_environmentinfo_table(table, "Installed", environment.get_installed() ? "True" : "False");

    auto repos = environment.get_repos();
    std::string joint_repolist = "";
    for (auto repo : repos) {
        if (joint_repolist != "") {
            joint_repolist.append(", ");
        }
        joint_repolist.append(repo);
    }
    add_line_into_environmentinfo_table(table, "Repositories", joint_repolist.c_str());

    struct libscols_line * ln_required = scols_table_new_line(table, NULL);
    scols_line_set_data(ln_required, 0, "Required groups");
    for (auto group : environment.get_groups()) {
        struct libscols_line * ln = scols_table_new_line(table, ln_required);
        scols_line_set_data(ln, 0, group.c_str());
    }

    struct libscols_line * ln_optional = scols_table_new_line(table, NULL);
    scols_line_set_data(ln_optional, 0, "Optional groups");
    for (auto group : environment.get_optional_groups()) {
        struct libscols_line * ln = scols_table_new_line(table, ln_optional);
        scols_line_set_data(ln, 0, group.c_str());
    }


    return table;
}

void print_environmentinfo_table(libdnf::comps::Environment & environment) {
    struct libscols_table * table = create_environmentinfo_table(environment);
    scols_print_table(table);
    scols_unref_table(table);
}


}  // namespace libdnf::cli::output

#endif  // LIBDNF_CLI_OUTPUT_ENVIRONMENTINFO_HPP
