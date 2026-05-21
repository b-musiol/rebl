/**
 * Core Implementation for REBL - Library for handling and analysis of
 * REliability BLock diagrams.
 *
 * Author: Bartek Musiol (bmusiol@proton.me)
 *
 * See LICENSE
 */

#include "../include/Core.hpp"
#include <memory>

using namespace REBL;

RBD::Core::Core()
    : rbd(std::make_unique<
          KnoKan::DirectedGraph<std::string, ComponentData, EmptyP>>())
{
}

RBD::Core::~Core()
{
}