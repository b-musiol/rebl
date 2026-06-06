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

#include "../include/MCSSettings.hpp"
#include <map>

#include "../include/ComponentData.hpp"
#include "../include/REBL.hpp"
#include "FC.hpp"
#include "RBD_DB.hpp"
#include "graphs/DirectedGraph.hpp"
#include <Kochs.hpp>
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

    /**
     * Runs the enumerative minimal cut sets algorithm to calculate the rbd.
     */
    Kochs::Object run_mcs_and_save();

    /**
     * Spawns a RBD DB Template at `db_path`.
     */
    static void spawn_rbd_db_template(std::filesystem::path db_path);

  private:
    unsigned int get_next_run_id();
    SQLiteDB::Row make_params_insert_result_summary_row(
        unsigned int run_id,
        bool ok,
        double H,
        double T,
        bool use_probability,
        unsigned int min_combination_size,
        unsigned int max_combination_size,
        double min_probability,
        double max_probability);
    SQLiteDB::Row make_params_update_result_summary_row(unsigned int run_id,
                                                        bool ok,
                                                        double H,
                                                        double T);
    SQLiteDB::Row make_params_insert_result_fc_row(unsigned int fc_id,
                                                   unsigned int run_id,
                                                   double H,
                                                   double T);
    SQLiteDB::Row make_params_insert_detail_fc_row(unsigned int fc_id,
                                                   unsigned int run_id,
                                                   double H,
                                                   double T,
                                                   std::string component);
};

#endif // _REBL_CORE_HPP