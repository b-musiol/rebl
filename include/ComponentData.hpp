/**
 * ComponentData Property - Library for handling and analysis of
 * REliability BLock diagrams.
 *
 * Author: Bartek Musiol (bmusiol@proton.me)
 *
 * See LICENSE
 */

#ifndef _REBL_COMPONENTDATA_HPP
#define _REBL_COMPONENTDATA_HPP

#include <Kochs.hpp>
#include <misc/Property.hpp>

using EmptyP = KnoKan::Property::Predefined::NoWeight;

namespace REBL
{

class ComponentData : public KnoKan::Property::Base
{
    Kochs::Object rel_data;

  public:
    ComponentData();
    ComponentData(double H, double T);
    ComponentData(const Kochs::Object &rel_data);

  public:
    double get_weight() const override;
    Kochs::Object get_rel_data() const;
};
} // namespace REBL

#endif // _REBL_COMPONENTDATA_HPP