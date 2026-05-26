/**
 * Failure Combinations for REBL - Library for handling and analysis of
 * REliability BLock diagrams.
 *
 * Author: Bartek Musiol (bmusiol@proton.me)
 *
 * See LICENSE
 */

#ifndef _REBL_FC_HPP
#define _REBL_FC_HPP

#include "ComponentData.hpp"
#include "MCSSettings.hpp"
#include "graphs/DirectedGraph.hpp"

#include <map>
#include <string>
#include <vector>

namespace REBL
{

namespace FailureCombination
{

class Machine
{
  public:
    /**
     * Initiates a new Failure Combination machine which guarantees to always
     * present a valid Failure Combination to analyze.
     */
    Machine(std::map<std::string, std::vector<int>> &component_instance_map,
            MCSSettings mcs_settings,
            KnoKan::DirectedGraph<int, ComponentData, EmptyP> &rbd);
    Machine(Machine &&)      = default;
    Machine(const Machine &) = default;
    ~Machine();

  public:
    /**
     * Gets the next failure combination. The most canonical way to use this is
     * probably.
     * `for(auto fc = fc_machine.next(); !fc.empty(); fc = fc_machine.next()){}`
     */
    std::vector<std::string> next();
    /**
     * Resets the internal state machine keeping the settings.
     */
    void reset();
    /**
     * Resets the internal state machine and allows to pass new
     * `mcs_settings` to have a different run through the exact same rbd.
     */
    void reset(MCSSettings mcs_settings);

  private:
    enum class ActionState : size_t
    {
        INIT,
        RESET_AND_DONE,
        VERT_INCREMENT,
        VERT_DECREMENT_AND_HORZ_INCREMENT,
        HORZ_INCREMENT,
        DONE
    };

    enum class PreState : size_t
    {
        NEW_FC         = 0,
        TOO_PROBABLE   = 1,
        TOO_IMPROBABLE = 2,
        OK             = 3
    };

    enum class ThisState : size_t
    {
        TOO_PROBABLE   = 1,
        TOO_IMPROBABLE = 2,
        OK             = 3
    };

    struct State
    {
        /**
         * State of the internal FC in last round.
         * NEW_FC if this is the first round.
         */
        PreState pre_state;
        /**
         * State of the internal FC right now
         */
        ThisState this_state;
        /**
         * Is incrementation (horizontal or vertical) possible?
         */
        bool incr_possible;
        /**
         * Is vertical decrementation possible?
         */
        bool vert_decr_possible;
    };

    // fc: failure combination
  private:
    void set_fc_gen_this_state(ThisState &this_state);
    bool is_incr_possible();
    bool is_vert_decr_possible();
    ActionState evaluate_fc_state_machine(State &state);
    bool fc_is_too_improbable();
    bool fc_is_too_probable();
    bool fc_increment_vertical();
    bool fc_decrement_vertical();
    bool fc_increment_horizontal();
    Kochs::Object read_rel_data(const std::string &fc_component);
    Kochs::Object accumulate_rel_data_in_fc();

  private:
    std::vector<std::string> failure_combination;

  private:
    std::map<std::string, std::vector<int>> &component_instance_map;
    std::unordered_map<int, ComponentData> &node_properties;

  private:
    bool use_probability;
    unsigned int min_combination_size;
    unsigned int max_combination_size;
    double min_probability;
    double max_probability;
};

} // namespace FailureCombination

} // namespace REBL

#endif // _REBL_FC_HPP