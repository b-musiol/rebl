/**
 * CLI for REBL-SCV - Library for handling and analysis of
 * REliability BLock diagrams.
 *
 * "What, you run out of marines?"
 *
 * Author: Bartek Musiol (bmusiol@proton.me)
 *
 * See LICENSE
 */

#ifndef _REBL_SCVAPP_HPP
#define _REBL_SCVAPP_HPP

#include <string>

namespace REBL
{
namespace SCVAPP
{

enum class InputState : int
{
    WAIT   = 0,
    INIT   = 1,
    EXIT   = 2,
    PLACE  = 3,
    FORK   = 4,
    REWIND = 5,
    JOIN   = 6,
    HELP   = 7,
    SAVE   = 8,
    DUMP   = 9,
};

constexpr const char *helptext = R"asfhelp(
_______________________________________________________________________________
                             "SCV good to go, sir!"


This tool helps you create JSON representations of reliability block diagrams
as required for REBL. You can create them, dump them to the console or save
them to a file. As per this version, you cannot undo or delete blocks. You can
always reset to the start though.

As per startup, an empty JSON is already pre-set.

Just keep typing in commands and press enter. All commands are lowercase.
At times you may be asked for a secondary command like a name or a file path.
There is no tab-completion for paths currently.

A Fork Level means how many recursive forks are active now. Fork Level 0 is the
state without any forks.

_________
Commands:
  ________
  General:

    h (HELP):   Print this help text.

    i (INIT):   Initializes the JSON to an empty array [].
                This is your restart command.

    e (EXIT):   Exits this program. No saving is done here. It just abandons
                the internal state.
  _____________
  RBD specific:

    p (PLACE):  Places a block. This will ask you for a name after issuing.

    f (FORK):   Place a fork. This works recursively.
                Increases the Fork Level by one.

    r (REWIND): Rewind to the last fork you placed, finalizing this branch.
                This will fail if you rewind without any active fork. 
                (Aka Fork Level 0)

    j (JOIN):   Joins all branches from the last fork. Places a "null" block.
                This will fail if you join without any active fork. 
                (Aka Fork Level 0)
  _______
  Output:

    s (SAVE):   Saves the current JSON to a file.
                This will ask you for a path after issuing.
                This is unformatted, raw serialized JSON.

    d (DUMP):   Print the current JSON to the terminal.
                This is unformatted, raw serialized JSON.
_______________________________________________________________________________
)asfhelp";

bool is_valid_path(const std::string &path);

} // namespace SCVAPP

} // namespace REBL

int main(int argc, const char **argv);

#endif // _REBL_SCVAPP_HPP