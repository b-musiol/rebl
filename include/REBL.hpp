/**
 * Public Header for REBL - Library for handling and analysis of REliability
 * BLock diagrams.
 *
 * Author: Bartek Musiol (bmusiol@proton.me)
 *
 * See LICENSE
 */

#ifndef _REBL_HPP
#define _REBL_HPP

#include <memory>

namespace REBL
{

class RBD
{
  public:
    RBD();
    ~RBD();

  private:
    /// PIMPL
    struct Core;
    std::unique_ptr<Core> core;
};

} // namespace REBL

#endif // _REBL_HPP