/********************************************************************************/
/*     888888    888888888   88     888  88888   888      888    88888888       */
/*   8       8   8           8 8     8     8      8        8    8               */
/*  8            8           8  8    8     8      8        8    8               */
/*  8            888888888   8   8   8     8      8        8     8888888        */
/*  8      8888  8           8    8  8     8      8        8            8       */
/*   8       8   8           8     8 8     8      8        8            8       */
/*     888888    888888888  888     88   88888     88888888     88888888        */
/*                                                                              */
/*       A Three-Dimensional General Purpose Semiconductor Simulator.           */
/*                                                                              */
/*                                                                              */
/*  Copyright (C) 2007-2008                                                     */
/*  Cogenda Pte Ltd                                                             */
/*                                                                              */
/*  Please contact Cogenda Pte Ltd for license information                      */
/*                                                                              */
/*  Author: Gong Ding   gdiso@ustc.edu                                          */
/*                                                                              */
/********************************************************************************/



#ifndef __df_ise_io_h__
#define __df_ise_io_h__

// C++ includes
#include <map>
#include <fstream>

// Local includes
#include "field_input.h"
#include "field_output.h"
#include "simulation_system.h"
#include "simulation_region.h"

// Forward declarations
class MeshBase;



/**
 * This class implements reading and writing solution data in the DF-ISE format.
 * The native grid/data file used by Synopsys sentaurus
 */

// ------------------------------------------------------------
// DFISEIO class definition
class DFISEIO : public FieldInput<SimulationSystem>,
                public FieldOutput<SimulationSystem>
{
public:
  /**
   * Constructor.  Takes a writeable reference to a mesh object.
   * This is the constructor required to read a mesh.
   */
  DFISEIO (SimulationSystem& system);

  /**
   * Constructor.  Takes a read-only reference to a mesh object.
   * This is the constructor required to write a mesh.
   */
  DFISEIO (const SimulationSystem& system);


  /**
   * This method implements reading a mesh from a specified file
   * in DF-ISE format.
   */
  virtual void read (const std::string& );


  /**
   * This method implements writing a mesh to a specified DF-ISE file.
   * NOT implemented yet!
   */
  virtual void write (const std::string& )
  { genius_error(); }

private:


};



// ------------------------------------------------------------
// VTKIO inline members
inline
DFISEIO::DFISEIO (SimulationSystem& system) :
    FieldInput<SimulationSystem> (system),
    FieldOutput<SimulationSystem> (system)
{

}



inline
DFISEIO::DFISEIO (const SimulationSystem& system) :
    FieldOutput<SimulationSystem>(system)
{

}



#endif // #define __df_ise_io_h__