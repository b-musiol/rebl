/**
 * Core Implementation for REBL - Library for handling and analysis of
 * REliability BLock diagrams.
 *
 * Author: Bartek Musiol (bmusiol@proton.me)
 *
 * See LICENSE
 */

#ifndef _REBL_CORE_HPP
#define _REBL_CORE_HPP

#include "MCSSettings.hpp"
#include <map>
#define KNOKAN_DIRECTED_GRAPH
#define KNOKAN_ALGORITHM_DFS

#include "ComponentData.hpp"
#include "FC.hpp"
#include "RBD_DB.hpp"
#include "REBL.hpp"
#include <KnoKan.hpp>
#include <Litesaver.hpp>
#include <Ticket.hpp>
#include <string>


using namespace REBL;

struct RBD::Core
{
    std::unique_ptr<KnoKan::DirectedGraph<int, ComponentData, EmptyP>> rbd;

    std::string rbd_db_path;
    DB::Connection rbd_db;

    Ticket id_dropper;
    std::map<std::string, std::vector<int>> component_instance_map;
    FailureCombination::Machine fc_machine;

    int start_point_id;
    int end_point_id;

    Core(const std::string_view rbd_db_path, const MCSSettings &mcs_settings);
    ~Core();

    /**
     * Parses the rbd from the json in the rbd_db_path.
     */
    void parse_rbd();
    /**
     * Recursively parses the rbd from `rbd_json` and `rbd_components` between
     * pre-created `split_node` and `join_node`.
     */
    void parse_rbd_core(
        nlohmann::json &rbd_json,
        std::unordered_map<std::string, Kochs::Object> &rbd_components,
        int &split_node,
        int &join_node);
    /**
     * Finds the component name to its `component_id`. If none exists, returns a
     * `std::optional` that is indeed empty.
     */
    std::optional<std::string> find_component_name(int component_id);
};

#endif // _REBL_CORE_HPP