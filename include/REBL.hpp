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
#include <filesystem>
#include <memory>
#include <string_view>

namespace REBL
{

class RBD
{
  public:
    /**
     * Opens a new REBL RBD object with a SQLite Database at `rbd_db_path`. (If
     * you don't have one, spawn a template with `spawn_rbd_db_template()`.
     *
     * Initializes the calculation parameters with `mcs_settings`.
     *
     * Per default `parse_from_disk` is set to `true`, meaning that the entry
     * `rbd` in table `input_rbd` is read and parsed as the RBD to be executed
     * calculations on. If you do not wish to do that (e.g. for running in a
     * degree of a cached mode), set this to false. Then you have to supply
     * your own RBD-JSON string.
     */
    RBD(const std::string_view rbd_db_path,
        const MCSSettings &mcs_settings,
        bool parse_from_disk = true);
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
     * Changes the active RBD for the passed `rbd_json`. This assumes the
     * components are set up correctly and re-parses instantly without writing
     * the RBD to disk.
     */
    void change_cached_rbd(std::string_view rbd_json);
    /**
     * Runs the Minimal Cut Sets algorithm on the RBD and saves the results
     */
    Kochs::Object run_minimal_cut_sets();

  public:
    /**
     * Spawns an empty RBD SQLite Database template at `db_path` which can be
     * filled with an RBD and associated data.
     */
    static void spawn_rbd_db_template(std::filesystem::path db_path);

    /**
     * Merges all outputs from the `input_db_paths` into `output_db_path`. This
     * preserves all other tables from the input db at `ix_main_input_db` in the
     * `input_db_paths` vector. In turn, the resulting db is compatible with
     * REBL again.*/
    static void merge_output(
        const std::filesystem::path &output_db_path,
        const std::vector<std::filesystem::path> &input_db_paths,
        size_t ix_main_input_db = 0);

  private:
    /// PIMPL
    struct Core;
    std::unique_ptr<Core> core;
};

} // namespace REBL

#endif // _REBL_HPP