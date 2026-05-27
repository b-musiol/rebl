/**
 * REBL - Library for handling and analysis of REliability BLock diagrams.
 *
 * Author: Bartek Musiol (bmusiol@proton.me)
 *
 * See LICENSE
 */
#include "../include/REBL.hpp"
#include "../include/Core.hpp"
#include <Litesaver.hpp>
#include <sstream>


using namespace REBL;

RBD::RBD(const std::string_view rbd_db_path, const MCSSettings &mcs_settings)
    : core(std::make_unique<Core>(rbd_db_path, mcs_settings))
{
    core->parse_rbd();
}

RBD::~RBD()
{
}

KnoKan::
    DirectedGraph<int, ComponentData, KnoKan::Property::Predefined::NoWeight>
    RBD::get_pure_graph()
{
    return *(core->rbd);
}

std::string RBD::get_graph_adjacency_string(bool with_rel_data,
                                            bool with_component_names)
{
    std::stringstream out;

    for (auto &[node, neighbors] : core->rbd->adjacency_map)
    {

        out << node;
        if (with_component_names)
        {
            auto node_name = core->find_component_name(node);
            out << " (";
            if (node_name.has_value())
            {
                out << *node_name;
            }
            else
            {
                out << "§virtual§";
            }
            out << ")";
        }
        out << " -> [ ";
        bool first = true;
        for (auto &neighbor : neighbors)
        {
            if (first)
            {
                first = false;
            }
            else
            {
                out << ", ";
            }
            out << neighbor;
            if (with_component_names)
            {
                auto node_name = core->find_component_name(neighbor);
                out << " (";
                if (node_name.has_value())
                {
                    out << *node_name;
                }
                else
                {
                    out << "§virtual§";
                }
                out << ")";
            }
        }
        out << "]";
        if (with_rel_data)
        {
            out << " (";

            auto &rel_data = core->rbd->node_properties.at(node);

            out << "H: " << rel_data.get_rel_data().H() << "; ";
            out << "T: " << rel_data.get_rel_data().T() << "; ";
            out << "P: " << rel_data.get_weight() << "";

            out << ")";
        }

        out << "\n";
    }

    return out.str();
}

Kochs::Object RBD::run_minimal_cut_sets()
{
    return core->run_mcs_and_save();
}

void RBD::spawn_rbd_db_template(std::filesystem::path db_path)
{
    RBD::Core::spawn_rbd_db_template(db_path);
}