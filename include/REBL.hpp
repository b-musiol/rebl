/**
 * Public Header for REBL - Library for handling and analysis of REliability
 * BLock diagrams.
 *
 * Author: Bartek Musiol (bmusiol@proton.me)
 *
 * See LICENSE
 */

#ifndef _REBL_HPP
#define _REBL_HPP

#include "ComponentData.hpp"
#include "Kochs.hpp"
#include "MCSSettings.hpp"
#include "graphs/DirectedGraph.hpp"
#include <memory>
#include <string_view>


namespace REBL
{

class RBD
{
  public:
    RBD(const std::string_view rbd_db_path, const MCSSettings &mcs_settings);
    ~RBD();

  public:
    /**
     * Gets a copy of the underlying directed graph.
     */
    KnoKan::DirectedGraph<int,
                          ComponentData,
                          KnoKan::Property::Predefined::NoWeight>
    get_pure_graph();
    /**
     * Gets the string that describes the underlying graph. At its base, it just
     * shows the adjacency list with the IDs. `with_rel_data` also adds a
     * reliability data (H/T/P) string after each node). `with_component_names`
     * adds the names of the components with the ids for maximum verbosity. This
     * is only useful for debugging, although all graph data can be exported
     * that way.
     */
    std::string get_graph_adjacency_string(bool with_rel_data,
                                           bool with_component_names);

  public:
    /**
     * Runs the Minimal Cut Sets algorithm on the RBD and saves the
     */
    Kochs::Object run_minimal_cut_sets();

  private:
    /// PIMPL
    struct Core;
    std::unique_ptr<Core> core;
};

} // namespace REBL

#endif // _REBL_HPP