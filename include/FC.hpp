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
    /**
     * Gets the current settings.
     */
    MCSSettings get_current_settings();

  private:
    enum class ActionState : size_t
    {
        BEGIN   = 1,
        CORE_1  = 2,
        CORE_2  = 3,
        UPWARDS = 4,
        EXIT    = 5
    };

    // fc: failure combination
  private:
    std::vector<std::string> copy_failure_combination();
    bool fc_is_too_improbable();
    bool fc_is_too_probable();
    Kochs::Object read_rel_data(const std::string &fc_component);
    Kochs::Object accumulate_rel_data_in_fc();

  private:
    std::vector<std::map<std::string, std::vector<int>>::iterator>
        failure_combination;
    bool begin;

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