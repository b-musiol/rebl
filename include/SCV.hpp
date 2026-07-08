/**
 * Public Header for REBL-SCV - Library for handling and analysis of
 * REliability BLock diagrams.
 * Provides methods to build the REBL reliability block diagram as a JSON
 * through code.
 *
 * "Reportin' for duty."
 *
 * Author: Bartek Musiol (bmusiol@proton.me)
 *
 * See LICENSE
 */

#ifndef _REBL_SCV_HPP
#define _REBL_SCV_HPP

#include <memory>
#include <string>

namespace REBL
{

class SCV
{
  public:
    SCV();
    ~SCV();

  public:
    /// PIMPL
    struct Core;
    std::unique_ptr<Core> m_core;

  public:
    /**
     * Gets the internal JSON as a string.
     */
    std::string get_json_string();
    /**
     * Saves the internal JSON to the `json_path` given.
     */
    void save_to_file(const std::string json_path);

  public:
    /**
     * Deletes the currently internally saved RBD and begins building mode with
     * the initial node.
     */
    void init();
    /**
     * Places a block with a `block_name` at the current node. Blocks with the
     * same name can appear multiple time in the RBD and will be the exact same
     * component, just multiple instances of it (and also fail simultaneously).
     */
    void place(const std::string_view block_name);
    /**
     * Increases the fork counter and saves the current node to the internal
     * fork stack. This affects `rewind()` and `join()`. Effectively, this
     * creates a new fork node.
     */
    void fork();
    /**
     * Joins all strands from the last `fork()` called (and only the last fork,
     * not recursively!). Places a NULL block right after. Decreases the fork
     * counter and pops the last node from the internal fork stack. If fork is
     * called with an empty fork stack, this is effectively a no-op.
     */
    void join();
    /**
     * Finalizes the current branch and returns to the last forked node to
     * start a new branch. It is discouraged to first `rewind()` and then
     * `join()`, because that just creates a short circuit around all other
     * branches.
     */
    void rewind();
};

} // namespace REBL

#endif // _REBL_SCV_HPP