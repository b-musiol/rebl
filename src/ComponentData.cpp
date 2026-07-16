/**
 * ComponentData Property - Library for handling and analysis of
 * REliability BLock diagrams.
 *
 * Author: Bartek Musiol (bmusiol@proton.me)
 *
 * See LICENSE
 */

#include "../include/ComponentData.hpp"
using namespace REBL;

double ComponentData::get_weight() const { return rel_data.P(); }
Kochs::Object ComponentData::get_rel_data() const { return rel_data; }

ComponentData::ComponentData() : rel_data() {}
ComponentData::ComponentData(double H, double T) : rel_data(H, T) {}
ComponentData::ComponentData(const Kochs::Object &rel_data)
    : rel_data(rel_data.H(), rel_data.T())
{
}