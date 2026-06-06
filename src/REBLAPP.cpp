/**
 * Wrapper App to run basic things in REBL - Library for handling and analysis
 * of REliability BLock diagrams.
 *
 * Author: Bartek Musiol (bmusiol@proton.me)
 *
 * See LICENSE
 */

#include "../include_private/REBLAPP.hpp"

#include "../include/REBL.hpp"

#include <filesystem>
#include <iostream>
#include <string>

int main(int argc, const char **argv)
{

    if (argc > 1)
    {
        bool output_template_db = false;
        bool has_file           = false;
        bool run_mcs            = false;
        int target_file_exists  = -1;

        bool next_is_min_probability = false;
        bool next_is_max_probability = false;
        bool next_is_min_combination = false;
        bool next_is_max_combination = false;

        bool use_probability         = false;
        double min_probability       = 1e-10;
        double max_probability       = 1.0;
        unsigned int min_combination = 1;
        unsigned int max_combination = 2;

        std::filesystem::path rbd_db_path;
        bool next_is_rbd_db_path = false;
        for (size_t i = 1; i < argc; i++)
        {
            auto arg = std::string(argv[i]);
            if (next_is_rbd_db_path)
            {
                next_is_rbd_db_path = false;
                rbd_db_path         = arg;
                if (std::filesystem::exists(rbd_db_path))
                {
                    target_file_exists = 1;
                }
                else
                {
                    target_file_exists = 0;
                }
                has_file = true;
            }
            if (next_is_min_probability)
            {
                min_probability         = std::stod(arg);
                next_is_min_probability = false;
            }
            if (next_is_max_probability)
            {
                max_probability         = std::stod(arg);
                next_is_max_probability = false;
            }
            if (next_is_min_combination)
            {
                min_combination = std::stoul(arg);
                if (min_combination == 0)
                {
                    std::cout << "min_combination must be greater than 0!";
                    return 7;
                }
                next_is_min_combination = false;
            }
            if (next_is_max_combination)
            {
                max_combination = std::stoul(arg);
                if (max_combination == 0)
                {
                    std::cout << "max_combination must be greater than 0!";
                    return 8;
                }
                next_is_max_combination = false;
            }
            else
            {
                if (arg == "-m")
                {
                    run_mcs = true;
                }
                else if (arg == "-t")
                {
                    output_template_db = true;
                }
                else if (arg == "-f")
                {
                    next_is_rbd_db_path = true;
                }
                else if (arg == "-h")
                {
                    std::cout << helptext;
                    return 1;
                }
                else if (arg == "--min-probability")
                {
                    next_is_min_probability = true;
                }
                else if (arg == "--max-probability")
                {
                    next_is_max_probability = true;
                }
                else if (arg == "--min-combination")
                {
                    next_is_min_combination = true;
                }
                else if (arg == "--max-combination")
                {
                    next_is_max_combination = true;
                }
                else if (arg == "--use-probability")
                {
                    use_probability = true;
                }
            }
        }
        if (output_template_db && has_file)
        {
            if (target_file_exists)
            {
                std::cout
                    << "Target file exists. Will be overwritten if possible.\n";
            }
            std::cout << "Writing RDB DB template...\n";
            REBL::RBD::spawn_rbd_db_template(rbd_db_path.string());
            std::cout << "Done! Now adapt the template.\n\n";
        }
        else if (run_mcs && has_file)
        {
            if (!target_file_exists)
            {
                std::cout << "Target file does not exist. Please specify a "
                             "valid target RBD DB.\n";
                return 3;
            }
            if (use_probability)
            {
                if (min_probability >= max_probability)
                {
                    std::cout << "You must specify a min_probability higher "
                                 "than max_probability (not equal!)!"
                              << "\nmin_probability given: " << min_probability
                              << "\nmax_probability given: " << max_probability
                              << "\n\n";
                    return 5;
                }
            }
            else
            {
                if (min_combination > max_combination)
                {
                    std::cout << "You must specify a min_combination higher "
                                 "than max_combination (equal is ok)!"
                              << "\nmin_combination given: " << min_combination
                              << "\nmax_combination given: " << max_combination
                              << "\n\n";
                    return 6;
                }
            }
            REBL::MCSSettings mcssettings{
                .use_probability      = use_probability,
                .min_combination_size = min_combination,
                .max_combination_size = max_combination,
                .min_probability      = min_probability,
                .max_probability      = max_probability};
            std::cout << "Initializing REBL...\n";
            REBL::RBD rebl(rbd_db_path.string(), mcssettings);
            std::cout << "Running MCS Algorithm...\n";
            rebl.run_minimal_cut_sets();
            std::cout << "Done! Check the given RBD DB for output.\n\n";
        }
        else
        {
            std::cout << "Bad combination of arguments encountered!\n\n"
                      << helptext;
            return 2;
        }
    }
    else
    {
        std::cout << helptext;
    }
    return 0;
}
