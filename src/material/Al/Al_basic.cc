/*****************************************************************************/
/*                                                                           */
/*              8888888         88888888         88888888                    */
/*            8                8                8                            */
/*           8                 8                8                            */
/*           8                  88888888         88888888                    */
/*           8      8888                8                8                   */
/*            8       8                 8                8                   */
/*              888888         888888888        888888888                    */
/*                                                                           */
/*       A Two-Dimensional General Purpose Semiconductor Simulator.          */
/*                                                                           */
/*  GSS material database Version 0.4                                        */
/*  Last update: May 27, 2007                                                */
/*                                                                           */
/*  Gong Ding                                                                */
/*  gdiso@ustc.edu                                                           */
/*  NINT, No.69 P.O.Box, Xi'an City, China                                   */
/*                                                                           */
/*****************************************************************************/
//
// Material Type: Aluminum


#include "PMI.h"

class GSS_Al_BasicParameter : public PMIC_BasicParameter
{
private:

  PetscScalar PERMITTI;      // The relative dielectric permittivity.
  PetscScalar PERMEABI;      // The relative megnetic permeability.
  PetscScalar AFFINITY;      // The electron affinity for the material.
  PetscScalar DENSITY;       // Specific mass density for the material.
  PetscScalar CONDUCTANCE;   // Specific conductance for the material.

  void   Basic_Init()
  {
    PERMITTI = 1.0;//corrected by zhangxih
    PERMEABI = 1.0;
    AFFINITY = 4.250000e+00*eV;
    DENSITY  = 2.700000e-03*kg*std::pow(cm,-3);//corrected by zhangxih
    CONDUCTANCE = 1.0/(2.874e-6*V/A*cm);

#ifdef __CALIBRATE__
    parameter_map.insert(para_item("PERMITTI",    PARA("PERMITTI",    "The relative dielectric permittivity", "-", 1.0, &PERMITTI)) );
    parameter_map.insert(para_item("PERMEABI",  PARA("PERMEABI",  "The relative megnetic permeability", "-", 1.0, &PERMEABI)) );
    parameter_map.insert(para_item("AFFINITY", PARA("AFFINITY", "The electron affinity for the material", "eV", eV, &AFFINITY)) );
    parameter_map.insert(para_item("DENSITY", PARA("DENSITY", "Specific mass density for the material", "kg*cm^-3", kg*std::pow(cm,-3), &DENSITY)) );
    parameter_map.insert(para_item("CONDUCTANCE", PARA("CONDUCTANCE", "Specific conductance for the material", "(ohmic*m)^-1", A/V/m, &CONDUCTANCE)) );
#endif
  }

public:
  PetscScalar Density       (const PetscScalar &Tl) const { return DENSITY;     }
  PetscScalar Permittivity  ()                      const { return PERMITTI;    }
  PetscScalar Permeability  ()                      const { return PERMEABI;    }
  PetscScalar Affinity      (const PetscScalar &Tl) const { return AFFINITY;    }
  PetscScalar Conductance   ()                      const { return CONDUCTANCE; }

  void atom_fraction(std::vector<std::string> &atoms, std::vector<double> & fraction) const
  {
    atoms.push_back("Aluminum");
    fraction.push_back(1.0);
  }

public:
  GSS_Al_BasicParameter(const PMIC_Environment &env):PMIC_BasicParameter(env)
  {
    Basic_Init();
  }
  ~GSS_Al_BasicParameter()
  {
  }
}
;

extern "C"
{
  DLL_EXPORT_DECLARE  PMIC_BasicParameter* PMIC_Al_BasicParameter_Default (const PMIC_Environment& env)
  {
    return new GSS_Al_BasicParameter(env);
  }
}
