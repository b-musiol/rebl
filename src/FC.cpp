/**
 * Failure Combinations for REBL - Library for handling and analysis of
 * REliability BLock diagrams.
 *
 * Author: Bartek Musiol (bmusiol@proton.me)
 *
 * See LICENSE
 */

#include "../include/FC.hpp"
#include "Kochs.hpp"
#include <format>
#include <stdexcept>

using namespace REBL::FailureCombination;

Machine::Machine(
    std::map<std::string, std::vector<int>> &component_instance_map,
    MCSSettings mcs_settings,
    KnoKan::DirectedGraph<int, ComponentData, EmptyP> &rbd)
    : component_instance_map(component_instance_map),
      failure_combination(std::vector<std::string>()),
      node_properties(rbd.node_properties)
{
    reset(mcs_settings);
}

Machine::~Machine()
{
}

std::vector<std::string> Machine::next()
{
    // This part only happens during the initial stage
    if (failure_combination.empty())
    {
        for (auto &[component, instance] : component_instance_map)
        {
            if (!(read_rel_data(component).is_ideal()))
            {
                failure_combination.push_back(component);
                break;
            }
        }
        while (fc_is_too_improbable() || fc_is_too_probable())
        {
            if (is_incr_possible())
            {
                fc_increment_vertical();
            }
            else if (is_vert_decr_possible())
            {
                fc_decrement_vertical();
                fc_increment_horizontal();
            }
            else
            {
                throw std::runtime_error(std::format(
                    "REBL Error: The MCSSettings given are making the rbd not "
                    "calculable\nuse_probability: {}; min_combination_size: "
                    "{}; max_combination_size: {}; min_probability: {}; "
                    "max_probability: {};",
                    use_probability,
                    min_combination_size,
                    max_combination_size,
                    min_probability,
                    max_probability));
            }
        }
        return failure_combination;
    }

    /**
     * Initialize State Machine
     */
    State state;
    // Pre State always starts with new FC
    state.pre_state = PreState::NEW_FC;
    // "This" state depends on the current internal FC
    set_fc_gen_this_state(state.this_state);
    // Incrementation possibility is defined as in the method.
    state.incr_possible = is_incr_possible();
    // Same for vertical decrementation.
    state.vert_decr_possible = is_vert_decr_possible();

    ActionState action_state = ActionState::INIT;

    /**
     * Loop and execute the state machine until done.
     */
    while (action_state != ActionState::DONE)
    {

        /**
         * Evaluate the state machine for action state
         */
        action_state = evaluate_fc_state_machine(state);

        /**
         * Execute the action
         */
        switch (action_state)
        {

        case ActionState::RESET_AND_DONE:
            // The action
            reset();
            action_state = ActionState::DONE;
            break;

        case ActionState::VERT_INCREMENT:
            // The action
            fc_increment_vertical();
            break;

        case ActionState::VERT_DECREMENT_AND_HORZ_INCREMENT:
            // The action
            fc_decrement_vertical();
            fc_increment_horizontal();
            break;

        case ActionState::HORZ_INCREMENT:
            // The action
            fc_increment_horizontal();
            break;

        case ActionState::DONE:
            // Does nothing here, but will break the loop next round.
            break;

        /**
         * Down here is bad.
         */
        default:
        case ActionState::INIT:
            throw std::runtime_error("REBL Error: Failure Combination Machine "
                                     "bad state.");
            break;
        }

        if (action_state != ActionState::DONE)
        {
            /**
             * Update the state
             */
            state.pre_state = static_cast<PreState>(state.this_state);
            set_fc_gen_this_state(state.this_state);
            state.incr_possible      = is_incr_possible();
            state.vert_decr_possible = is_vert_decr_possible();
        }
    }

    /**
     * State Machine is done. Return the generated FC from the internal
     * FC.
     */
    return failure_combination;
}
void Machine::set_fc_gen_this_state(ThisState &this_state)
{
    if (fc_is_too_improbable())
    {
        this_state = ThisState::TOO_IMPROBABLE;
    }
    else if (fc_is_too_probable())
    {
        this_state = ThisState::TOO_PROBABLE;
    }
    else
    {
        this_state = ThisState::OK;
    }
}

bool Machine::is_incr_possible()
{
    /**
     * Incrementation is possible if
     * - The internal FC is not empty (requires a reset)
     * - The internal FC's last component is not the largest component.
     */
    if (!failure_combination.empty())
    {
        return failure_combination.back() !=
               component_instance_map.rbegin()->first;
    }
    else
    {
        return false;
    }
}

bool Machine::is_vert_decr_possible()
{
    /**
     * Vertical decrementation is possible if the current FC is larger than
     * one component.
     */
    return failure_combination.size() > 1;
}

Machine::ActionState Machine::evaluate_fc_state_machine(State &state)
{
    /**
     * The state machine should have 48 states, which is
     * - 4 pre states
     * - 3 "this" states
     * - Incrementation is possible or not (2 states)
     * - Vert. decrementation is possible or not (2 states)
     * Thus 4*3*2*2 = 48
     */
    ActionState action_state;
    auto &pre_state      = state.pre_state;
    auto &this_state     = state.this_state;
    auto &incr_possible  = state.incr_possible;
    auto &vdecr_possible = state.vert_decr_possible;

    /**
     * Evaluation start
     * Dead state := A state that is logically present in the branch, but
     * leads to a previous branch, as it also exists there. Thus this dead
     * state will never lead to the branch it is defined as dead in.
     * Subsequently it does not contribute to the total amount of states.
     */
    if (!incr_possible && !vdecr_possible) // 12 States
    {
        /**
         * If no increments or vertical decrements are possible, this is
         * reset and done. This happens when we are at the last FC.
         */
        action_state = ActionState::RESET_AND_DONE;
    }
    else if (pre_state == PreState::NEW_FC &&
             this_state != ThisState::OK) // 6 States (+2 dead)
    {
        /**
         * If this is a new call to the FC state machine and the internal
         * FC is not ok, this is a bad state.
         * This is used to find the first valid FC and thus is not an
         * error, but should not be used in regular FCs.
         */
        action_state = ActionState::RESET_AND_DONE;
    }
    else if (pre_state == PreState::NEW_FC && this_state == ThisState::OK &&
             incr_possible) // 2 States
    {
        /**
         * next_fc was just called, found an ok FC and can increment
         * vertically.
         */
        action_state = ActionState::VERT_INCREMENT;
    }
    else if (pre_state == PreState::NEW_FC && this_state == ThisState::OK &&
             !incr_possible && vdecr_possible) // 1 State
    {
        /**
         * next_fc was just called, found an ok FC and cannot increment
         * vertically, but the penultimate component can be incremented
         * horizontally.
         */
        action_state = ActionState::VERT_DECREMENT_AND_HORZ_INCREMENT;
    }
    else if ((pre_state == PreState::TOO_PROBABLE ||
              pre_state == PreState::TOO_IMPROBABLE) &&
             this_state == ThisState::TOO_PROBABLE && incr_possible) // 4 States
    {
        /**
         * Previous round had a valid non-ok FC and the change made it too
         * probable. Incrementation is possible.
         */
        action_state = ActionState::VERT_INCREMENT;
    }
    else if ((pre_state == PreState::TOO_PROBABLE ||
              pre_state == PreState::TOO_IMPROBABLE) &&
             this_state == ThisState::TOO_PROBABLE && !incr_possible &&
             vdecr_possible) // 2 States
    {
        /**
         * Previous round had a valid non-ok FC and the change made it too
         * probable. Incrementation is not possible.
         */
        action_state = ActionState::VERT_DECREMENT_AND_HORZ_INCREMENT;
    }
    else if ((pre_state == PreState::TOO_PROBABLE ||
              pre_state == PreState::TOO_IMPROBABLE) &&
             this_state == ThisState::TOO_IMPROBABLE &&
             incr_possible) // 4 States
    {
        /**
         * Previous round had a valid non-ok FC and the change made it too
         * improbable. Incrementation is possible.
         */
        action_state = ActionState::HORZ_INCREMENT;
    }
    else if ((pre_state == PreState::TOO_PROBABLE ||
              pre_state == PreState::TOO_IMPROBABLE) &&
             this_state == ThisState::TOO_IMPROBABLE && !incr_possible &&
             vdecr_possible) // 2 States
    {
        /**
         * Previous round had a valid non-ok FC and the change made it too
         * improbable. Incrementation is not possible.
         */
        action_state = ActionState::VERT_DECREMENT_AND_HORZ_INCREMENT;
    }
    else if (pre_state != PreState::NEW_FC &&
             this_state == ThisState::OK) // 9 States (+3 dead)
    {
        /**
         * Previous round had a valid FC and the change kept it ok. This is
         * a new valid FC that we want to export.
         */
        action_state = ActionState::DONE;
    }
    else if (pre_state == PreState::OK && this_state != ThisState::OK &&
             incr_possible) // 4 States
    {
        /**
         * Previous round had a valid ok FC and the change made it not ok.
         * Incrementation is possible.
         */
        action_state = ActionState::VERT_INCREMENT;
    }
    else if (pre_state == PreState::OK && this_state != ThisState::OK &&
             !incr_possible && vdecr_possible) // 2 States
    {
        /**
         * Previous round had a valid ok FC and the change made it not ok.
         * Incrementation is not possible.
         */
        action_state = ActionState::VERT_DECREMENT_AND_HORZ_INCREMENT;
    }
    else
    {
        /**
         * This branch can only be reached if the states are not correctly
         * set, e.g. with illegal values in the enums.
         * This should only ever be happening during development when the
         * logic is being debugged.
         */
        throw std::runtime_error(
            "REBL Error: Unexpected (machine) state in FC generator.");
    }

    /**
     * Evaluation end!
     * Previously implemented states amount to:
     * 12+6+2+1+4+2+4+2+9+4+2 = 48
     * So the state machine is complete.
     */

    return action_state;
}

bool Machine::fc_is_too_improbable()
{
    /**
     * "Too improbable" means that the probability is too low. This is the
     * more likely use case.
     */

    /**
     * Failure combinations with ideal components are not legal.
     */
    for (auto &fc_component : failure_combination)
    {
        if (read_rel_data(fc_component).is_ideal())
        {
            return true;
        }
    }
    /**
     * Only failure combinations without ideal components get through.
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

bool Machine::fc_increment_vertical()
{
    /**
     * Turns e.g.:
     * [1,
     *  3,
     *  5]
     * into
     * [1,
     *  3,
     *  5,
     *  6]
     *
     * Returns if this was possible.
     * Does not change the fc if this was not possible.
     */
    if (!failure_combination.empty())
    {
        auto adjusted_component_it =
            component_instance_map.find(failure_combination.back());
        adjusted_component_it++;
        if (adjusted_component_it != component_instance_map.end())
        {
            failure_combination.push_back(adjusted_component_it->first);
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool Machine::fc_decrement_vertical()
{
    /**
     * Turns e.g.:
     * [1,
     *  3,
     *  5]
     * into
     * [1,
     *  3]
     *
     * Returns if this was possible.
     * Does not change the fc if this was not possible.
     * Does not decrement a 1 sized FC into one without components.
     */
    if (failure_combination.size() > 1)
    {
        failure_combination.pop_back();
        return true;
    }
    else
    {
        return false;
    }
}

bool Machine::fc_increment_horizontal()
{
    /**
     * Turns e.g.:
     * [1,
     *  3,
     *  5]
     * into
     * [1,
     *  3,
     *  6]
     *
     * Returns if this was possible.
     * Does not change the fc if this was not possible.
     */
    if (!failure_combination.empty())
    {
        auto adjusted_component_it =
            component_instance_map.find(failure_combination.back());
        adjusted_component_it++;
        if (adjusted_component_it != component_instance_map.end())
        {
            failure_combination.back() = adjusted_component_it->first;
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
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
        auto &first_comp_in_fc     = failure_combination.front();
        Kochs::Object curr_fc_zvkd = read_rel_data(first_comp_in_fc);

        /**
         * If there are more than one component, they are handled in series
         * (this aligns with the MCS method).
         */
        for (size_t i = 1; i < failure_combination.size(); i++)
        {
            curr_fc_zvkd =
                curr_fc_zvkd & read_rel_data(failure_combination.at(i));
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
    failure_combination = std::vector<std::string>{};
}