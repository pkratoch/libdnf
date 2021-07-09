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

#include "test_environment.hpp"

#include "libdnf/comps/comps.hpp"
#include "libdnf/comps/environment/query.hpp"

extern "C" {
#include <solv/repo.h>
}

#include <filesystem>
#include <fstream>


CPPUNIT_TEST_SUITE_REGISTRATION(CompsEnvironmentTest);


void CompsEnvironmentTest::setUp() {
    base = std::make_unique<libdnf::Base>();
}


void CompsEnvironmentTest::tearDown() {}


void CompsEnvironmentTest::test_load() {
    std::filesystem::path data_path = PROJECT_SOURCE_DIR "/test/libdnf/comps/data/";
    const char * reponame = "repo";
    libdnf::comps::Comps comps(*base.get());

    comps.load_from_file(data_path / "minimal-environment.xml", reponame);
    libdnf::comps::EnvironmentQuery q_minimal_env(comps.get_environment_sack());
    q_minimal_env.filter_installed(false);
    q_minimal_env.filter_environmentid("minimal-environment");
    auto minimal_env = q_minimal_env.get();
    CPPUNIT_ASSERT_EQUAL(std::string("minimal-environment"), minimal_env.get_environmentid());
    CPPUNIT_ASSERT_EQUAL(std::string("Minimal Install"), minimal_env.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("Minimale Installation"), minimal_env.get_translated_name("de"));
    CPPUNIT_ASSERT_EQUAL(std::string("Basic functionality."), minimal_env.get_description());
    CPPUNIT_ASSERT_EQUAL(std::string("Grundlegende Funktionalität."), minimal_env.get_translated_description("de"));
    CPPUNIT_ASSERT_EQUAL(std::string("3"), minimal_env.get_order());
    CPPUNIT_ASSERT_EQUAL(false, minimal_env.get_installed());
}


void CompsEnvironmentTest::test_load_defaults() {
    std::filesystem::path data_path = PROJECT_SOURCE_DIR "/test/libdnf/comps/data/";
    const char * reponame = "repo";
    libdnf::comps::Comps comps(*base.get());

    comps.load_from_file(data_path / "minimal-environment-empty.xml", reponame);
    libdnf::comps::EnvironmentQuery q_minimal_empty(comps.get_environment_sack());
    q_minimal_empty.filter_environmentid("minimal-environment");
    auto minimal_empty = q_minimal_empty.get();
    CPPUNIT_ASSERT_EQUAL(std::string("minimal-environment"), minimal_empty.get_environmentid());
    CPPUNIT_ASSERT_EQUAL(std::string(""), minimal_empty.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string(""), minimal_empty.get_translated_name("ja"));
    CPPUNIT_ASSERT_EQUAL(std::string(""), minimal_empty.get_description());
    CPPUNIT_ASSERT_EQUAL(std::string(""), minimal_empty.get_translated_description("ja"));
    CPPUNIT_ASSERT_EQUAL(std::string(""), minimal_empty.get_order());
    CPPUNIT_ASSERT_EQUAL(false, minimal_empty.get_installed());
}

void CompsEnvironmentTest::test_merge() {
    std::filesystem::path data_path = PROJECT_SOURCE_DIR "/test/libdnf/comps/data/";
    const char * reponame = "repo";
    libdnf::comps::Comps comps(*base.get());

    comps.load_from_file(data_path / "minimal-environment.xml", reponame);
    comps.load_from_file(data_path / "custom-environment.xml", reponame);
    // load another definiton of the minimal-environment that changes all attributes
    comps.load_from_file(data_path / "minimal-environment-v2.xml", reponame);

    libdnf::comps::EnvironmentQuery q_minimal_env(comps.get_environment_sack());
    q_minimal_env.filter_environmentid("minimal-environment");
    auto minimal_env = q_minimal_env.get();
    CPPUNIT_ASSERT_EQUAL(std::string("minimal-environment"), minimal_env.get_environmentid());
    CPPUNIT_ASSERT_EQUAL(std::string("Minimal Install v2"), minimal_env.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("Minimale Installation v2"), minimal_env.get_translated_name("de"));
    CPPUNIT_ASSERT_EQUAL(std::string("Basic functionality v2."), minimal_env.get_description());
    CPPUNIT_ASSERT_EQUAL(std::string("Grundlegende Funktionalität v2."), minimal_env.get_translated_description("de"));
    CPPUNIT_ASSERT_EQUAL(std::string("4"), minimal_env.get_order());
    CPPUNIT_ASSERT_EQUAL(false, minimal_env.get_installed());
}


void CompsEnvironmentTest::test_merge_with_empty() {
    std::filesystem::path data_path = PROJECT_SOURCE_DIR "/test/libdnf/comps/data/";
    const char * reponame = "repo";
    libdnf::comps::Comps comps(*base.get());

    comps.load_from_file(data_path / "minimal-environment.xml", reponame);
    comps.load_from_file(data_path / "custom-environment.xml", reponame);
    // load another definiton of the minimal-environment that has all attributes empty
    comps.load_from_file(data_path / "minimal-environment-empty.xml", reponame);

    libdnf::comps::EnvironmentQuery q_minimal_empty(comps.get_environment_sack());
    q_minimal_empty.filter_environmentid("minimal-environment");
    auto minimal_empty = q_minimal_empty.get();
    CPPUNIT_ASSERT_EQUAL(std::string("minimal-environment"), minimal_empty.get_environmentid());
    // attributes are missing in minimal-environment-empty.xml -> original values are kept
    CPPUNIT_ASSERT_EQUAL(std::string("Minimal Install"), minimal_empty.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("Minimale Installation"), minimal_empty.get_translated_name("de"));
    CPPUNIT_ASSERT_EQUAL(std::string("Basic functionality."), minimal_empty.get_description());
    CPPUNIT_ASSERT_EQUAL(std::string("Grundlegende Funktionalität."), minimal_empty.get_translated_description("de"));
    CPPUNIT_ASSERT_EQUAL(std::string("3"), minimal_empty.get_order());
    CPPUNIT_ASSERT_EQUAL(false, minimal_empty.get_installed());
}


void CompsEnvironmentTest::test_merge_empty_with_nonempty() {
    std::filesystem::path data_path = PROJECT_SOURCE_DIR "/test/libdnf/comps/data/";
    const char * reponame = "repo";
    libdnf::comps::Comps comps(*base.get());

    // load definiton of the minimal-environment that has all attributes empty
    comps.load_from_file(data_path / "minimal-environment-empty.xml", reponame);
    comps.load_from_file(data_path / "custom-environment.xml", reponame);
    // load another definiton of the minimal-environment that has all attributes filled
    comps.load_from_file(data_path / "minimal-environment.xml", reponame);

    libdnf::comps::EnvironmentQuery q_minimal_env(comps.get_environment_sack());
    q_minimal_env.filter_installed(false);
    q_minimal_env.filter_environmentid("minimal-environment");
    auto minimal_env = q_minimal_env.get();
    CPPUNIT_ASSERT_EQUAL(std::string("minimal-environment"), minimal_env.get_environmentid());
    CPPUNIT_ASSERT_EQUAL(std::string("Minimal Install"), minimal_env.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("Minimale Installation"), minimal_env.get_translated_name("de"));
    CPPUNIT_ASSERT_EQUAL(std::string("Basic functionality."), minimal_env.get_description());
    CPPUNIT_ASSERT_EQUAL(std::string("Grundlegende Funktionalität."), minimal_env.get_translated_description("de"));
    CPPUNIT_ASSERT_EQUAL(std::string("3"), minimal_env.get_order());
    CPPUNIT_ASSERT_EQUAL(false, minimal_env.get_installed());
}


void CompsEnvironmentTest::test_dump_and_load() {
    std::filesystem::path data_path = PROJECT_SOURCE_DIR "/test/libdnf/comps/data/";
    const char * reponame = "repo";
    libdnf::comps::Comps comps(*base.get());

    comps.load_from_file(data_path / "minimal-environment.xml", reponame);
    libdnf::comps::EnvironmentQuery q_minimal_env(comps.get_environment_sack());
    q_minimal_env.filter_environmentid("minimal-environment");
    auto minimal_env = q_minimal_env.get();

    auto dumped_minimal_path = std::filesystem::temp_directory_path() / "dumped-minimal-env.xml";
    minimal_env.dump(dumped_minimal_path);
    libdnf::comps::Comps comps2(*base.get());
    comps2.load_from_file(dumped_minimal_path, reponame);

    libdnf::comps::EnvironmentQuery q_dumped_minimal_env(comps2.get_environment_sack());
    q_dumped_minimal_env.filter_environmentid("minimal-environment");
    auto dumped_minimal_env = q_dumped_minimal_env.get();

    CPPUNIT_ASSERT_EQUAL(std::string("minimal-environment"), dumped_minimal_env.get_environmentid());
    CPPUNIT_ASSERT_EQUAL(std::string("Minimal Install"), dumped_minimal_env.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("Minimale Installation"), dumped_minimal_env.get_translated_name("de"));
    CPPUNIT_ASSERT_EQUAL(std::string("Basic functionality."), dumped_minimal_env.get_description());
    CPPUNIT_ASSERT_EQUAL(std::string("Grundlegende Funktionalität."), dumped_minimal_env.get_translated_description("de"));
    CPPUNIT_ASSERT_EQUAL(std::string("3"), dumped_minimal_env.get_order());
    CPPUNIT_ASSERT_EQUAL(false, dumped_minimal_env.get_installed());
}
