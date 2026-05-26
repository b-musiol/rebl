/**
 * Core Implementation for REBL - Library for handling and analysis of
 * REliability BLock diagrams.
 *
 * Author: Bartek Musiol (bmusiol@proton.me)
 *
 * See LICENSE
 */

#include "../include/Core.hpp"
#include "algorithms/DFS.hpp"
#include <Kochs.hpp>
#include <memory>
#include <stdexcept>

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

std::optional<std::string> RBD::Core::find_component_name(int value)
{
    for (const auto &[name, vec] : component_instance_map)
    {
        if (std::find(vec.begin(), vec.end(), value) != vec.end())
            return name;
    }

    return std::nullopt;
}

Kochs::Object RBD::Core::run_mcs_and_save()
{
    Kochs::Object system_reliability;
    // This method loops over all failure combinations (they are generated
    // live through the failure combination machine).
    // At each loop it first calculates the result, then saves the failure
    // combination result to the database along with the rbd.
    // After all results are calculated, it calculates the accumulated result
    // and writes that into the database as well.
    for (auto fc = fc_machine.next(); !fc.empty(); fc = fc_machine.next())
    {
        Kochs::Object fc_reliability;
        // Extract the block instances in the current failure combination,
        // which only carries the actual components, that can have multiple
        // instances.
        std::unordered_set<int> fc_block_instances;
        for (auto &component : fc)
        {
            for (auto &instance_id : component_instance_map.at(component))
            {
                fc_block_instances.insert(instance_id);
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
            // Now we just need to add it to the system reliability. That is the
            // OR operator. Here the operator overloading can be used right
            // away, because an ideal component does not absorb through an OR
            // operator.
            system_reliability = system_reliability | fc_reliability;
        }
    }

    // Here we return the system reliability, mostly for testing purposes. But
    // the application may also do something useful with this.
    return system_reliability;
}