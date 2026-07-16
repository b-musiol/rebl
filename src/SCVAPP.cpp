/**
 * CLI header for REBL-SCV - Library for handling and analysis of
 * REliability BLock diagrams.
 *
 * "What, you run out of marines?"
 *
 * Author: Bartek Musiol (bmusiol@proton.me)
 *
 * See LICENSE
 */

#include "../include_private/SCVAPP.hpp"
#include "../include/SCV.hpp"

#include <filesystem>
#include <iostream>
#include <string>

using namespace REBL::SCVAPP;

bool REBL::SCVAPP::is_valid_path(const std::string &path)
{
    std::filesystem::path p(path);

    std::filesystem::path parent =
        p.has_parent_path() ? p.parent_path() : std::filesystem::current_path();

    return std::filesystem::exists(parent) &&
           std::filesystem::is_directory(parent);
}

int main(int argc, const char **argv)
{
    if (argc > 1)
    {
        if (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help")
        {
            std::cout << helptext;
            return 2;
        }
        else
        {
            std::cout << "You have provided " << argc - 1
                      << " unnecessary argument" << ((argc == 2) ? "" : "s")
                      << ".\nThe Terran Dominion appreciates "
                         "the donation but is also confused...\n";
        }
        // </unfunny_joke>
    }

    REBL::SCV scv;
    std::cout << "\nIssue the [h] command for help.\n";
    std::string user_buffer, name_buffer, filepath_buffer;
    unsigned int current_fork_level = 0;

    InputState input_state = InputState::INIT;

    while (input_state != InputState::EXIT)
    {
        switch (input_state)
        {
        case InputState::INIT:
            std::cout << "Re-Initializing the JSON...\n";
            scv.init();
            current_fork_level = 0;
            input_state        = InputState::WAIT;
            break;

        case InputState::EXIT:
            std::cout << "Exiting...\n";
            // No-Op
            break;

        case InputState::PLACE:
            std::cout << "Placing a new block...\n";
            std::cout << "Enter a name for this block: ";
            std::cin >> name_buffer;
            scv.place(name_buffer);
            input_state = InputState::WAIT;
            break;

        case InputState::FORK:
            std::cout << "Placing a new fork...\n";
            scv.fork();
            current_fork_level++;
            input_state = InputState::WAIT;

            break;

        case InputState::REWIND:
            std::cout << "Rewinding to last fork...\n";
            if (current_fork_level > 0) { scv.rewind(); }
            else
            {
                std::cout
                    << "You are at Fork Level 0. "
                    << "You cannot rewind from here.\n"
                    << "You need to fork at least once to be able to rewind.\n";
            }
            input_state = InputState::WAIT;

            break;

        case InputState::JOIN:
            std::cout << "Joining all branches from last fork...\n";
            if (current_fork_level > 0)
            {
                scv.join();
                current_fork_level--;
            }
            else
            {
                std::cout
                    << "You are at Fork Level 0. "
                    << "You cannot join from here.\n"
                    << "You need to fork at least once to be able to join.\n";
            }
            input_state = InputState::WAIT;

            break;

        case InputState::HELP:
            std::cout << helptext;
            input_state = InputState::WAIT;
            break;

        case InputState::SAVE: {
            std::cout << "Saving JSON to file...\n";
            std::cout << "Enter a filename (all folders must exist): ";
            std::cin >> filepath_buffer;
            if (is_valid_path(filepath_buffer))
            {
                scv.save_to_file(filepath_buffer);
                std::cout << "Saved to " << filepath_buffer << std::endl;
            }
            else
            {
                std::cout << "This is not a valid path: " << filepath_buffer
                          << "\n";
                std::cout << "Nothing was saved. You can retry." << std::endl;
            }
            input_state = InputState::WAIT;
            break;
        }

        case InputState::DUMP:
            std::cout << "\n" << scv.get_json_string() << "\n";
            input_state = InputState::WAIT;
            break;

        case InputState::WAIT: {
            bool input_was_ok = false;
            std::cout << "\n++Current Fork Level: " << current_fork_level
                      << "\n";
            while (!input_was_ok)
            {
                std::cout << "Enter a command [hiepfrjsd]: ";
                std::cin >> user_buffer;
                if (user_buffer == "i" || user_buffer == "INIT" ||
                    user_buffer == "init")
                {
                    input_state  = InputState::INIT;
                    input_was_ok = true;
                }
                else if (user_buffer == "e" || user_buffer == "EXIT" ||
                         user_buffer == "exit")
                {
                    input_state  = InputState::EXIT;
                    input_was_ok = true;
                }
                else if (user_buffer == "p" || user_buffer == "PLACE" ||
                         user_buffer == "place")
                {
                    input_state  = InputState::PLACE;
                    input_was_ok = true;
                }
                else if (user_buffer == "f" || user_buffer == "FORK" ||
                         user_buffer == "fork")
                {
                    input_state  = InputState::FORK;
                    input_was_ok = true;
                }
                else if (user_buffer == "r" || user_buffer == "REWIND" ||
                         user_buffer == "rewind")
                {
                    input_state  = InputState::REWIND;
                    input_was_ok = true;
                }
                else if (user_buffer == "j" || user_buffer == "JOIN" ||
                         user_buffer == "join")
                {
                    input_state  = InputState::JOIN;
                    input_was_ok = true;
                }
                else if (user_buffer == "h" || user_buffer == "HELP" ||
                         user_buffer == "help")
                {
                    input_state  = InputState::HELP;
                    input_was_ok = true;
                }
                else if (user_buffer == "s" || user_buffer == "SAVE" ||
                         user_buffer == "save")
                {
                    input_state  = InputState::SAVE;
                    input_was_ok = true;
                }
                else if (user_buffer == "d" || user_buffer == "DUMP" ||
                         user_buffer == "dump")
                {
                    input_state  = InputState::DUMP;
                    input_was_ok = true;
                }
                else
                {
                    std::cout << "[" << user_buffer << "]"
                              << "is no valid command. Use [h] for help.";
                }

                std::cout << std::endl; // guarantee flush
            }
        }

        break;

        default:
            std::cerr << "Unimplemented input state encountered. id: "
                      << static_cast<int>(input_state);
            return 1;
        }
    }

    return 0;
}