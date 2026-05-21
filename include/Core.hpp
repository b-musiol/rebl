/**
 * Core Implementation for REBL - Library for handling and analysis of
 * REliability BLock diagrams.
 *
 * Author: Bartek Musiol (bmusiol@proton.me)
 *
 * See LICENSE
 */

#ifndef _REBL_CORE_HPP
#define _REBL_CORE_HPP

#define KNOKAN_DIRECTED_GRAPH
#define KNOKAN_ALGORITHM_DFS

#include "ComponentData.hpp"
#include "REBL.hpp"
#include <KnoKan.hpp>
#include <string>

using namespace REBL;

struct RBD::Core
{
    std::unique_ptr<KnoKan::DirectedGraph<std::string, ComponentData, EmptyP>>
        rbd;

    Core();
    ~Core();
};

#endif // _REBL_CORE_HPP