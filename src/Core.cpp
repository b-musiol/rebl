/**
 * Core Implementation for REBL - Library for handling and analysis of
 * REliability BLock diagrams.
 *
 * Author: Bartek Musiol (bmusiol@proton.me)
 *
 * See LICENSE
 */

#include "../include_private/Core.hpp"
#include "algorithms/DFS.hpp"
#include "rbd_db_queries.hpp"
#include <Kochs.hpp>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <variant>
#include <vector>

using namespace REBL;

RBD::Core::Core(const std::string_view rbd_db_path,
                const MCSSettings &mcs_settings)
    : rbd(std::make_unique<
          KnoKan::DirectedGraph<int, ComponentData, EmptyP>>()),
      rbd_db_path(rbd_db_path), rbd_db(rbd_db_path), id_dropper(1),
      fc_machine(component_instance_map, mcs_settings, *rbd)
{
}

RBD::Core::~Core()
{
}

void RBD::Core::parse_rbd()
{
    // first get the data from the db
    auto rbd_json       = rbd_db.get_rbd();
    auto rbd_components = rbd_db.get_rbd_components();

    // Reset the inner graph.
    rbd->clear();

    // Define the outer start and end point
    start_point_id = id_dropper.pull();
    end_point_id   = id_dropper.pull();
    // Add these as nodes into the rbd
    rbd->add_node(start_point_id);
    rbd->add_node(end_point_id);
    // The start point remains constant, but the end point keeps being pushed
    // back.

    parse_rbd_core(rbd_json, rbd_components, start_point_id, end_point_id);
}

void RBD::Core::parse_rbd_core(
    nlohmann::json &rbd_json,
    std::unordered_map<std::string, Kochs::Object> &rbd_components,
    int &split_node,
    int &join_node)
{
    int curr_prev = split_node;
    int curr_id   = curr_prev;
    int curr_post = join_node;
    int curr_join, curr_split;

    bool prev_was_subblock = false;

    // run through the json, which is a list
    for (auto &block : rbd_json)
    {

        // There are three possibilities:
        // - string: Block instance
        // - null: glue block
        // - list: split with more blocks inside
        if (block.is_string())
        {
            curr_id = id_dropper.pull();
            // new block found
            std::string block_name = block.get<std::string>();
            // make new entry in the component instance map
            component_instance_map[block_name].push_back(curr_id);
            // Check if an entry exists
            auto rel_data_it = rbd_components.find(block_name);
            // Presume ideal
            ComponentData node_property(0, 0);
            if (rel_data_it != rbd_components.end())
            {
                // overwrite with data if it exists
                // This can be ideal, but we don't care about that.
                node_property = ComponentData(rel_data_it->second.H(),
                                              rel_data_it->second.T());
            }

            // add the node into the graph
            rbd->add_node(curr_id, node_property);

            // Connect to the previous node
            rbd->add_edge(curr_prev, curr_id);

            // Make the current node the previous node
            curr_prev = curr_id;

            // This is it, the block is integrated and catalogued!
            prev_was_subblock = false;
        }
        else if (block.is_null())
        {
            curr_id = id_dropper.pull();
            // glue block found

            // We don't catalogue glue blocks. They are just ideal blocks that
            // float around. But we still need a unique id.

            // add the node into the graph
            rbd->add_node(curr_id, ComponentData());

            // Connect to the previous node
            rbd->add_edge(curr_prev, curr_id);

            // Make the current node the previous node
            curr_prev = curr_id;

            // Done! The glue block is integrated!
            prev_was_subblock = false;
        }
        else if (block.is_array())
        {
            // split block found
            if (!prev_was_subblock)
            {
                // Provide a join node, but only if this is the first time a
                // split block is found, as these are shared between all
                // consecutive ones.
                curr_split = id_dropper.pull();
                curr_join = curr_id = id_dropper.pull();
                rbd->add_node(curr_join, ComponentData());
                rbd->add_node(curr_split, ComponentData());
                rbd->add_edge(curr_prev, curr_split);
                curr_prev = curr_id;
            }

            // This is just recursively calling this function
            parse_rbd_core(block, rbd_components, curr_split, curr_join);

            // Now the sub-block is integrated!
            // If this branch is called the next time, that just keeps adding
            // parallel blocks. If this branch is called after one of the two
            // other branches, then this will split new.
            prev_was_subblock = true;
        }
        else
        {
            // Illegal entry found
            throw std::runtime_error("Illegal entry in the rbd json found");
        }
    }

    // As a last step, connect the last node to the join node, which at the last
    // instance is just the end node of the entire rbd.
    // In the simplest (useless) case, this just connects the start node with
    // the end node, which is exactly what we want for a fully neutral rbd.
    // This is going to be called at every recursive sub-call, but because
    // add_edge() skips over re-adding edges, AND because edges are entirely
    // unweighted, this is fine.
    rbd->add_edge(curr_id, curr_post);
}

std::optional<std::string> RBD::Core::find_component_name(
    int component_instance)
{
    for (const auto &[name, vec] : component_instance_map)
    {
        if (std::find(vec.begin(), vec.end(), component_instance) != vec.end())
            return name;
    }

    return std::nullopt;
}

Kochs::Object RBD::Core::run_mcs_and_save()
{
    auto h_factor_raw = rbd_db.get_input_unique("rbd", "H_factor");
    auto t_factor_raw = rbd_db.get_input_unique("rbd", "T_factor");
    double h_factor;
    double t_factor;
    if (std::holds_alternative<double>(h_factor_raw) &&
        std::holds_alternative<double>(t_factor_raw))
    {
        h_factor = std::get<double>(h_factor_raw);
        t_factor = std::get<double>(t_factor_raw);
    }
    else
    {
        throw std::runtime_error(
            "REBL Error. Table 'input_rbd' has been filled incorrectly. "
            "Specifically H_Factor and T_Factor, which both need valFloat "
            "values.");
    }
    // Insert a summary with a "not ok" and empty (0) values flag.
    // This is then going to be adjusted when the calculation is done and
    // remains should something go wrong.
    auto current_settings = fc_machine.get_current_settings();
    unsigned int run_id   = get_next_run_id();
    SQLiteDB::Row params_insert_result_summary =
        make_params_insert_result_summary_row(
            run_id,
            rbd_db.get_rbd_raw_json(),
            false,
            0.0,
            0.0,
            current_settings.use_probability,
            current_settings.min_combination_size,
            current_settings.max_combination_size,
            current_settings.min_probability,
            current_settings.max_probability);
    rbd_db.direct_write_access(DB::Query::insert_into_output_result_summary,
                               params_insert_result_summary);

    std::vector<SQLiteDB::Row> v_params_result_fc;
    std::vector<SQLiteDB::Row> v_params_detail_fc;

    Kochs::Object system_reliability;
    // This method loops over all failure combinations (they are generated
    // live through the failure combination machine).
    // At each loop it first calculates the result, then saves the failure
    // combination result to the database along with the rbd.
    // After all results are calculated, it calculates the accumulated result
    // and writes that into the database as well.
    unsigned int fc_ctr = 0;
    for (auto fc = fc_machine.next(); !fc.empty(); fc = fc_machine.next())
    {
        fc_ctr++;
        // std::cout << "FC#" << fc_ctr << ": ";
        // for (auto &fc_entry : fc)
        // {
        //     std::cout << fc_entry << " ";
        // }
        // std::cout << "\n";

        Kochs::Object fc_reliability;
        // Extract the block instances in the current failure combination,
        // which only carries the actual components, that can have multiple
        // instances.
        std::unordered_set<int> fc_block_instances;
        for (auto &component : fc)
        {
            bool first = true;
            for (auto &instance_id : component_instance_map.at(component))
            {
                fc_block_instances.insert(instance_id);
                if (first)
                {
                    auto rel_data =
                        rbd->node_properties.at(instance_id).get_rel_data();
                    v_params_detail_fc.push_back(
                        make_params_insert_detail_fc_row(
                            fc_ctr,
                            run_id,
                            rel_data.H() / h_factor,
                            rel_data.T() / t_factor,
                            component));
                    first = false;
                }
            }
        }

        // check if the rbd is still traversable even if the failure
        // combination is applied
        bool system_is_functional =
            KnoKan::Algorithm::DFS::path_exists(*rbd,
                                                start_point_id,
                                                end_point_id,
                                                fc_block_instances);
        if (system_is_functional)
        {
            // If the system is functional, this failure combination is only
            // logged and not counted
            v_params_result_fc.push_back(
                make_params_insert_result_fc_row(fc_ctr, run_id, 0.0, 0.0));
        }
        else // if (!system_is_functional)
        {
            // If the system is not functional, this failure combination
            // contributes to the whole system failure values.
            bool first = true;
            for (auto &component : fc)
            {
                // We only need one of the instances of each component, because
                // the way we created them, all instances share the same
                // reliability data.
                const auto &instance_id =
                    component_instance_map.at(component).front();
                // This now holds the reliability data that we want to use.
                const auto &rel_data =
                    rbd->node_properties.at(instance_id).get_rel_data();
                if (first)
                {
                    // Since we want to combine all component data with AND,
                    // and the default is an ideal component, this would absorb
                    // all data. So we need to instantiate the first entry of
                    // the entire chain if AND combined objects with actual
                    // data.
                    fc_reliability = rel_data;
                    first          = false;
                }
                else
                {
                    // From the second component on we can just use operator
                    // overloading.
                    fc_reliability = fc_reliability & rel_data;
                }
            }

            v_params_result_fc.push_back(make_params_insert_result_fc_row(
                fc_ctr,
                run_id,
                fc_reliability.H() / h_factor,
                fc_reliability.T() / t_factor));
            // Now we just need to add it to the system reliability. That is the
            // OR operator. Here the operator overloading can be used right
            // away, because an ideal component does not absorb through an OR
            // operator.
            system_reliability = system_reliability | fc_reliability;
        }
    }

    // Write everything into the database
    rbd_db.direct_write_access(DB::Query::insert_into_output_result_fc,
                               v_params_result_fc);
    rbd_db.direct_write_access(DB::Query::insert_into_output_detail_fc,
                               v_params_detail_fc);
    rbd_db.direct_write_access(DB::Query::update_output_result_summary,
                               make_params_update_result_summary_row(
                                   run_id,
                                   true,
                                   system_reliability.H() / h_factor,
                                   system_reliability.T() / t_factor));

    // Here we return the system reliability, mostly for testing purposes. But
    // the application may also do something useful with this.
    return system_reliability;
}

unsigned int RBD::Core::get_next_run_id()
{
    auto run_id_raw =
        rbd_db.direct_read_access(DB::Query::select_largest_run_id);
    // Only accept positive run ids starting with 1
    if (run_id_raw.data.size() > 0 && run_id_raw.data.front().is_integer(0))
    {
        return std::max(run_id_raw.data.front().get_integer(0) + 1,
                        static_cast<std::int64_t>(1));
    }
    else
    {
        return 1;
    }
}

SQLiteDB::Row RBD::Core::make_params_insert_result_summary_row(
    unsigned int run_id,
    std::string rbd_json,
    bool ok,
    double H,
    double T,
    bool use_probability,
    unsigned int min_combination_size,
    unsigned int max_combination_size,
    double min_probability,
    double max_probability)
{
    // see REBL::DB::Query::insert_into_output_result_summary
    SQLiteDB::Row row;

    row.push_integer(run_id);
    row.push_text(rbd_json);
    row.push_integer(ok);
    row.push_real(H);
    row.push_real(T);
    row.push_integer(use_probability);
    row.push_integer(min_combination_size);
    row.push_integer(max_combination_size);
    row.push_real(min_probability);
    row.push_real(max_probability);

    return row;
}
SQLiteDB::Row RBD::Core::make_params_update_result_summary_row(
    unsigned int run_id,
    bool ok,
    double H,
    double T)
{
    // see REBL::DB::Query::update_output_result_summary
    SQLiteDB::Row row;

    row.push_integer(ok);
    row.push_real(H);
    row.push_real(T);
    row.push_integer(run_id);

    return row;
}
SQLiteDB::Row RBD::Core::make_params_insert_result_fc_row(unsigned int fc_id,
                                                          unsigned int run_id,
                                                          double H,
                                                          double T)
{
    // see REBL::DB::Query::insert_into_output_result_fc
    SQLiteDB::Row row;

    row.push_integer(fc_id);
    row.push_integer(run_id);
    row.push_real(H);
    row.push_real(T);

    return row;
}
SQLiteDB::Row RBD::Core::make_params_insert_detail_fc_row(unsigned int fc_id,
                                                          unsigned int run_id,
                                                          double H,
                                                          double T,
                                                          std::string component)
{
    // see REBL::DB::Query::insert_into_output_detail_fc
    SQLiteDB::Row row;

    row.push_integer(fc_id);
    row.push_integer(run_id);
    row.push_real(H);
    row.push_real(T);
    row.push_text(component);

    return row;
}

void RBD::Core::spawn_rbd_db_template(std::filesystem::path db_path)
{
    DB::Connection::spawn_rbd_db_template(db_path);
}