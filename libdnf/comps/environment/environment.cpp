#include <libdnf/comps/environment/environment.hpp>
#include <libdnf/comps/environment/query.hpp>
#include <libdnf/comps/environment/sack.hpp>
#include <libdnf/comps/group/group.hpp>
#include <libdnf/comps/comps.hpp>
#include <libdnf/comps/comps_impl.hpp>
#include <libdnf/utils/xml.hpp>

extern "C" {
#include <solv/knownid.h>
#include <solv/pool.h>
#include <solv/repo.h>
#include <solv/solvable.h>
#include <solv/dataiterator.h>
}

#include <string>
#include <iostream>
#include <libxml/tree.h>


namespace libdnf::comps {


Environment::~Environment() {}


Environment::Environment(EnvironmentQuery * query) : query(query->get_weak_ptr()) {}


void add_solvable_id(Environment & environment, Id solvable_id) {
    environment.add_environment_id(EnvironmentId(solvable_id));
}


void add_solvable_ids(Environment & environment, std::vector<Id> solvable_ids) {
    for (Id solvable_id : solvable_ids) {
        environment.add_environment_id(EnvironmentId(solvable_id));
    }
}


Environment & Environment::operator+=(const Environment & rhs) {
    this->environment_ids.insert(this->environment_ids.begin(), rhs.environment_ids.begin(), rhs.environment_ids.end());
    return *this;
}


std::string lookup_str(Pool * pool, std::vector<EnvironmentId> environment_ids, Id key) {
    for (EnvironmentId environment_id: environment_ids) {
        if (pool_lookup_str(pool, environment_id.id, key)) {
            return pool_lookup_str(pool, environment_id.id, key);
        }
    }
    return "";
}


std::string Environment::get_environmentid() const {
    std::string solvable_name(
        lookup_str(query->sack->comps.p_impl->get_pool(), environment_ids, SOLVABLE_NAME));
    if (solvable_name.find(":") == std::string::npos) {
        return "";
    }
    return solvable_name.substr(solvable_name.find(":") + 1);
}


std::string Environment::get_name() const {
    return lookup_str(query->sack->comps.p_impl->get_pool(), environment_ids, SOLVABLE_SUMMARY);
}


std::string Environment::get_description() const {
    return lookup_str(query->sack->comps.p_impl->get_pool(), environment_ids, SOLVABLE_DESCRIPTION);
}


std::string Environment::get_translated_name(const char * lang) const {
    std::string translation;
    Pool * pool = query->sack->comps.p_impl->get_pool();
    for (EnvironmentId environment_id: environment_ids) {
        Solvable * solvable = pool->solvables + environment_id.id;
        if (solvable_lookup_str_lang(solvable, SOLVABLE_SUMMARY, lang, 1)) {
            translation = solvable_lookup_str_lang(solvable, SOLVABLE_SUMMARY, lang, 1);
            // Return translation only if it's different from the untranslated string.
            if (translation != solvable_lookup_str(solvable, SOLVABLE_SUMMARY)) {
                return translation;
            }
        }
    }
    return this->get_name();
}


// TODO(pkratoch): Test this
std::string Environment::get_translated_name() const {
    std::string translation;
    Pool * pool = query->sack->comps.p_impl->get_pool();
    for (EnvironmentId environment_id: environment_ids) {
        Solvable * solvable = pool->solvables + environment_id.id;
        if (solvable_lookup_str_poollang(solvable, SOLVABLE_SUMMARY)) {
            translation = solvable_lookup_str_poollang(solvable, SOLVABLE_SUMMARY);
            // Return translation only if it's different from the untranslated string.
            if (translation != solvable_lookup_str(solvable, SOLVABLE_SUMMARY)) {
                return translation;
            }
        }
    }
    return this->get_name();
}


std::string Environment::get_translated_description(const char * lang) const {
    std::string translation;
    Pool * pool = query->sack->comps.p_impl->get_pool();
    for (EnvironmentId environment_id: environment_ids) {
        Solvable * solvable = pool->solvables + environment_id.id;
        if (solvable_lookup_str_lang(solvable, SOLVABLE_DESCRIPTION, lang, 1)) {
            translation = solvable_lookup_str_lang(solvable, SOLVABLE_DESCRIPTION, lang, 1);
            // Return translation only if it's different from the untranslated string.
            if (translation != solvable_lookup_str(solvable, SOLVABLE_DESCRIPTION)) {
                return translation;
            }
        }
    }
    return this->get_description();
}


std::string Environment::get_translated_description() const {
    std::string translation;
    Pool * pool = query->sack->comps.p_impl->get_pool();
    for (EnvironmentId environment_id: environment_ids) {
        Solvable * solvable = pool->solvables + environment_id.id;
        if (solvable_lookup_str_poollang(solvable, SOLVABLE_DESCRIPTION)) {
            translation = solvable_lookup_str_poollang(solvable, SOLVABLE_DESCRIPTION);
            // Return translation only if it's different from the untranslated string.
            if (translation != solvable_lookup_str(solvable, SOLVABLE_DESCRIPTION)) {
                return translation;
            }
        }
    }
    return this->get_description();
}


std::string Environment::get_order() const {
    return lookup_str(query->sack->comps.p_impl->get_pool(), environment_ids, SOLVABLE_ORDER);
}

std::vector<std::string> Environment::get_groups() {
    // Return groups if they are already loaded
    if (!groups.empty()) {
        return groups;
    }

    Pool * pool = query->sack->comps.p_impl->get_pool();
    // Use only the first (highest priority) solvable for group lists
    Solvable * solvable = pool_id2solvable(pool, environment_ids[0].id);
    std::string group_solvable_name;

    if (solvable->requires) {
        for (Id * r_id = solvable->repo->idarraydata + solvable->requires; *r_id; ++r_id) {
            std::string group_solvable_name = pool_id2str(pool, *r_id);
            groups.push_back(group_solvable_name.substr(group_solvable_name.find(":") + 1, std::string::npos));
        }
    }
    return groups;
}

std::vector<std::string> Environment::get_optional_groups() {
    // Return optional_groups if they are already loaded
    if (!optional_groups.empty()) {
        return optional_groups;
    }

    Pool * pool = query->sack->comps.p_impl->get_pool();
    // Use only the first (highest priority) solvable for group lists
    Solvable * solvable = pool_id2solvable(pool, environment_ids[0].id);
    std::string group_solvable_name;

    if (solvable->suggests) {
        for (Id * r_id = solvable->repo->idarraydata + solvable->suggests; *r_id; ++r_id) {
            std::string group_solvable_name = pool_id2str(pool, *r_id);
            optional_groups.push_back(group_solvable_name.substr(group_solvable_name.find(":") + 1, std::string::npos));
        }
    }
    return optional_groups;
}

std::set<std::string> Environment::get_repos() const {
    std::set<std::string> result;
    Pool * pool = query->sack->comps.p_impl->get_pool();
    for (EnvironmentId environment_id: environment_ids) {
        Solvable * solvable = pool_id2solvable(pool, environment_id.id);
        result.emplace(solvable->repo->name);
    }
    return result;
}


bool Environment::get_installed() const {
    auto repos = get_repos();
    return repos.find("@System") != repos.end();
}


void Environment::dump(const std::string & path) {
    // Create doc with root node "comps"
    xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
    xmlNodePtr node_comps = xmlNewNode(NULL, BAD_CAST "comps");
    xmlDocSetRootElement(doc, node_comps);

    // Create "environment" node
    xmlNodePtr node_environment = xmlNewNode(NULL, BAD_CAST "environment");
    xmlAddChild(node_comps, node_environment);

    // Add id, name, description, display_order
    utils::xml::add_subnode_with_text(node_environment, "id", get_environmentid());
    utils::xml::add_subnode_with_text(node_environment, "name", get_name());
    utils::xml::add_subnode_with_text(node_environment, "description", get_description());
    utils::xml::add_subnode_with_text(node_environment, "display_order", get_order());

    // Add translations
    std::set<std::string> name_langs;
    std::set<std::string> description_langs;
    std::string summary_prefix = "solvable:summary:";
    std::string description_prefix = "solvable:description:";
    std::string keyname;
    std::string lang;
    xmlNodePtr node;
    Pool * pool = query->sack->comps.p_impl->get_pool();
    for (auto environment_id : environment_ids) {
        Dataiterator di;
        dataiterator_init(&di, pool, 0, environment_id.id, 0, 0, 0);
        // Iterate over all data in the environment solvable
        while (dataiterator_step(&di) != 0) {
            // If the content is NULL, skip
            if (!di.kv.str) {
                continue;
            }
            keyname = pool_id2str(pool, di.key->name);
            // If keyname starts with "solvable:summary:", it's a name translation
            if (keyname.rfind(summary_prefix, 0) == 0) {
                lang = keyname.substr(summary_prefix.length());
                // Add the lang into the set
                // If it's succesful (wasn't already present), create an XML node for this translation
                if(name_langs.insert(lang).second) {
                    node = utils::xml::add_subnode_with_text(node_environment, "name", std::string(di.kv.str));
                    xmlNewProp(node, BAD_CAST "xml:lang", BAD_CAST lang.c_str());
                }
            }
            // If keyname starts with "solvable:description:", it's a description translation
            else if (keyname.rfind(description_prefix, 0) == 0) {
                lang = keyname.substr(description_prefix.length());
                // Add the lang into the set
                // If it's succesful (wasn't already present), create an XML node for this translation
                if(description_langs.insert(lang).second) {
                    node = utils::xml::add_subnode_with_text(node_environment, "description", std::string(di.kv.str));
                    xmlNewProp(node, BAD_CAST "xml:lang", BAD_CAST lang.c_str());
                }
            }
        }
        dataiterator_free(&di);
    }

    // Save the document
    if (xmlSaveFile(path.c_str(), doc) == -1) {
        throw std::runtime_error("failed to save xml document for comps");
    }

    // Memory free
    xmlFreeDoc(doc);
}

}  // namespace libdnf::comps
