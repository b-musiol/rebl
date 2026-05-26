/**
 * Failure Combinations for REBL - Library for handling and analysis of
 * REliability BLock diagrams.
 *
 * Author: Bartek Musiol (bmusiol@proton.me)
 *
 * See LICENSE
 */
#ifndef _REBL_MCSSETTINGS_HPP
#define _REBL_MCSSETTINGS_HPP

namespace REBL
{

/**
 * `use_probability` switches between whether the failure combinations have to
 * be between `min_probability` or `max_probability` (at `true`), or containing
 * between `min_combination_size` and `max_combination_size` amount of entries
 * (at `false`). Both include the edges.
 */
struct MCSSettings
{
    bool use_probability              = false;
    unsigned int min_combination_size = 1;
    unsigned int max_combination_size = 2;
    double min_probability            = 1e-6;
    double max_probability            = 1.0;
};
} // namespace REBL

#endif // _REBL_MCSSETTINGS_HPP