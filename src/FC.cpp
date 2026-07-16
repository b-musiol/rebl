/**
 * Failure Combinations for REBL - Library for handling and analysis of
 * REliability BLock diagrams.
 *
 * Author: Bartek Musiol (bmusiol@proton.me)
 *
 * See LICENSE
 */

#include "../include_private/FC.hpp"
#include "Kochs.hpp"

using namespace REBL::FailureCombination;

Machine::Machine(
    std::map<std::string, std::vector<int>> &component_instance_map,
    MCSSettings mcs_settings,
    KnoKan::DirectedGraph<int, ComponentData, EmptyP> &rbd)
    : component_instance_map(component_instance_map),
      node_properties(rbd.node_properties), begin(true)
{ reset(mcs_settings); }

Machine::~Machine() {}

std::vector<std::string> Machine::next()
{
    // set new_fc
    bool new_fc = true;
    // unset accept_fc
    bool accept_fc = false;

    // init action state
    ActionState action_state = ActionState::BEGIN;

    // execute state machine
    while (action_state != ActionState::EXIT)
    {
        switch (action_state)
        {

        case ActionState::BEGIN:
            // is fc empty?
            if (failure_combination.empty())
            {
                // is begin flag set?
                if (begin)
                {
                    // unset begin flag
                    begin = false;
                    // push iterator of the smallest component onto the fc
                    failure_combination.push_back(
                        component_instance_map.begin());
                    // unset new_fc
                    new_fc = false;
                    // Retain state
                    // action_state = ActionState::BEGIN;
                }
                else // if (!begin)
                {
                    // set begin flag
                    begin = true;
                    // Swap state
                    action_state = ActionState::EXIT;
                }
            }
            else // if (!failure_combination.empty())
            {
                // Swap state
                action_state = ActionState::CORE_1;
            }
            break;

        case ActionState::CORE_1:
            // is accept_fc set?
            if (accept_fc)
            {
                // Swap state
                action_state = ActionState::EXIT;
            }
            else // if (!accept_fc)
            {
                // is last component ideal?
                if (read_rel_data(failure_combination.back()->first).is_ideal())
                {
                    // advance last component iterator
                    failure_combination.back()++;
                    // Swap state
                    action_state = ActionState::UPWARDS;
                }
                else // if(!read_rel_data(failure_combination.back()->first).is_ideal())
                {
                    // is fc too probable?
                    if (fc_is_too_probable())
                    {
                        // swap state
                        action_state = ActionState::CORE_2;
                    }
                    // is fc too improbable?
                    else if (fc_is_too_improbable())
                    {
                        // pop from fc
                        failure_combination.pop_back();
                        // advance last component iterator
                        failure_combination.back()++;
                    }
                    // is new_fc set
                    else if (new_fc)
                    {
                        // swap state
                        action_state = ActionState::CORE_2;
                    }
                    else
                    {
                        // set accept_fc
                        accept_fc = true;
                        // swap state
                        action_state = ActionState::UPWARDS;
                    }
                }
            }
            break;

        case ActionState::CORE_2:
            // unset new_fc
            new_fc = false;
            // duplicate last fc iterator by pushing new
            failure_combination.push_back(failure_combination.back());
            // advance last component iterator
            failure_combination.back()++;
            // swap state
            action_state = ActionState::UPWARDS;
            break;

        case ActionState::UPWARDS:
            // is last component iterator exhausted?
            if (failure_combination.back() == component_instance_map.end())
            {
                // pop from fc
                failure_combination.pop_back();
                // is fc empty?
                if (failure_combination.empty())
                {
                    // Swap state
                    action_state = ActionState::BEGIN;
                }
                else // if (!failure_combination.empty())
                {
                    // advance last component iterator
                    failure_combination.back()++;
                    // Retain state
                    // action_state = ActionState::UPWARDS;
                }
            }
            else // if (failure_combination.back() ==
                 // component_instance_map.end())
            {
                // Swap State
                action_state = ActionState::CORE_1;
            }
            break;

        case ActionState::EXIT:
            // This just exists and is never traversed due to the surrounding
            // while loop. The action from the diagram is happening after the
            // loop.
            break;
        }
    }

    // State Machine is done. Return the generated FC from the internal FC.
    return copy_failure_combination();
}

std::vector<std::string> Machine::copy_failure_combination()
{
    std::vector<std::string> copied_fc;

    for (auto &fc_it : failure_combination)
    {
        copied_fc.push_back(fc_it->first);
    }

    return copied_fc;
}

bool Machine::fc_is_too_improbable()
{
    /**
     * "Too improbable" means that the probability is too low. This is the
     * more likely use case.
     */

    if (use_probability)
    {
        return (accumulate_rel_data_in_fc().P() < min_probability);
    }
    else // if (!use_probability)
    {
        return ((failure_combination.size()) > max_combination_size);
    }
}

bool Machine::fc_is_too_probable()
{
    /**
     * "Too Probable" means that the probability is too high. This is the
     * less likely use case.
     */
    if (use_probability)
    {
        return (accumulate_rel_data_in_fc().P() > max_probability);
    }
    else // if (!use_probability)
    {
        return (failure_combination.size() < min_combination_size);
    }
}

Kochs::Object Machine::read_rel_data(const std::string &fc_component)
{
    return node_properties.at(component_instance_map.at(fc_component).front())
        .get_rel_data();
}

Kochs::Object Machine::accumulate_rel_data_in_fc()
{
    if (!failure_combination.empty())
    {
        /**
         * Seed the algorithm with the first component. If this is the only
         * component, that is also the end of the accumulation.
         */
        Kochs::Object curr_fc_zvkd =
            read_rel_data(failure_combination.front()->first);

        /**
         * If there are more than one component, they are handled in series
         * (this aligns with the MCS method).
         */
        for (size_t i = 1; i < failure_combination.size(); i++)
        {
            curr_fc_zvkd =
                curr_fc_zvkd & read_rel_data(failure_combination.at(i)->first);
        }

        /**
         * Done! Return the zvkd.
         */
        return curr_fc_zvkd;
    }
    else
    {
        /**
         * At empty fc, return an ideal component.
         */
        return Kochs::Object();
    }
}

void Machine::reset(MCSSettings mcs_settings)
{
    use_probability      = mcs_settings.use_probability;
    min_combination_size = mcs_settings.min_combination_size;
    max_combination_size = mcs_settings.max_combination_size;
    min_probability      = mcs_settings.min_probability;
    max_probability      = mcs_settings.max_probability;
    reset();
}

void Machine::reset()
{
    failure_combination =
        std::vector<std::map<std::string, std::vector<int>>::iterator>();
    begin = true;
}

REBL::MCSSettings Machine::get_current_settings()
{
    return MCSSettings{.use_probability      = use_probability,
                       .min_combination_size = min_combination_size,
                       .max_combination_size = max_combination_size,
                       .min_probability      = min_probability,
                       .max_probability      = max_probability};
}