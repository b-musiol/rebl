/**
 * Wrapper App to run basic things in REBL - Library for handling and analysis of
 * REliability BLock diagrams.
 *
 * Author: Bartek Musiol (bmusiol@proton.me)
 *
 * See LICENSE
 */

#ifndef _REBL_REBLAPP_HPP
#define _REBL_REBLAPP_HPP

constexpr const char * helptext = R"helptext(
NAME
    REBLAPP - Wrapper App to run basic things in REBL (Library for handling and
              analysis of REliability BLock diagrams)

SYNOPSIS
    REBL [-hmt]
         [--use-probability]
         [-f rbd_db_path]
         [--min-probability min_probability] [--max-probability max_probability]
         [--min-combination min_combination] [--max-combination max_combination]
         

DESCRIPTION
    
    -h  print this help string

    -m  run the MCS Algorithm on the reliability block diagram, 
        not compatible with -t.

    -t  output a template database (SQLite), not compatible with -m.

    --use-probability  If set, min_probability and max_probability are used 
                       instead of min_combination and max_combination.
    

    -f rbd_db_path  Points to the file at rbd_db_path. If with -m, the file is 
                    read and the algorithm is executed, writing the results 
                    into the file as a new run. If with -t a template rbd_db 
                    is written there.


    --min-probability min_probability  Value between 0.0 and 1.0. Must be lower
                                       than max_probability. Lowest probability
                                       of a failure combination to be 
                                       considered.
                                       If not set, defaults to 1e-10.

    --max-probability max_probability  Value between 0.0 and 1.0. Must be higher
                                       than min_probability. Highest probability
                                       of a failure combination to be 
                                       considered.
                                       If not set, defaults to 1.0.

    --min-combination min_combination  Integer greater than 0. Must be lower
                                       than max_combination. Minimum amount of 
                                       components in a failure combination. If 
                                       not set, defaults to 1.

    --max-combination max_combination  Integer greater than 0. Must be higher
                                       than min_combination. Maximum amount of 
                                       components in a failure combination. If 
                                       not set, defaults to 2.


    -

)helptext";

int main(int argc, const char** argv);


#endif // _REBL_REBLAPP_HPP