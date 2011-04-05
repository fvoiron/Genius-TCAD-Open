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
/*  Last update: Feb 17, 2006                                                */
/*                                                                           */
/*  Gong Ding                                                                */
/*  gdiso@ustc.edu                                                           */
/*  NINT, No.69 P.O.Box, Xi'an City, China                                   */
/*                                                                           */
/*****************************************************************************/
//
// Material Type: Poly Silicon


#include "PMI.h"

class GSS_PolySi_Mob_Analytic : public PMIS_Mobility
{
private:
  // parameters for Analytic mobility
  PetscScalar MUN_MIN ;
  PetscScalar MUN_MAX ;
  PetscScalar NREFN   ;
  PetscScalar NUN     ;
  PetscScalar XIN     ;
  PetscScalar ALPHAN  ;
  PetscScalar MUP_MIN ;
  PetscScalar MUP_MAX ;
  PetscScalar NREFP   ;
  PetscScalar NUP     ;
  PetscScalar XIP     ;
  PetscScalar ALPHAP  ;
  PetscScalar T300    ;

  void Mob_Analytic_Init()
  {
    MUN_MIN = 5.524000E+01*cm*cm/V/s;
    MUN_MAX = 1.429230E+03*cm*cm/V/s;
    NREFN   = 1.072000E+17*std::pow(cm,-3);
    NUN     = -2.300000E+00;
    XIN     = -3.800000E+00;
    ALPHAN  = 7.300000E-01;
    MUP_MIN = 4.970000E+01*cm*cm/V/s;
    MUP_MAX = 4.793700E+02*cm*cm/V/s;
    NREFP   = 1.606000E+17*std::pow(cm,-3);
    NUP     = -2.200000E+00;
    XIP     = -3.700000E+00;
    ALPHAP  = 7.000000E-01;
    T300    = 300.0*K;

#ifdef __CALIBRATE__
         parameter_map.insert(para_item("MUN.MIN",    PARA("MUN.MIN",    "", "cm*cm/V/s",cm*cm/V/s , &MUN_MIN)) );
         parameter_map.insert(para_item("MUN.MAX",    PARA("MUN.MAX",    "", "cm*cm/V/s", cm*cm/V/s, &MUN_MAX)) );
         parameter_map.insert(para_item("NREFN",    PARA("NREFN",    "", "cm^-3", std::pow(cm,-3), &NREFN)) );
         parameter_map.insert(para_item("NUN",    PARA("NUN",    "", "-", 1.0, &NUN)) );
         parameter_map.insert(para_item("XIN",    PARA("XIN",    "", "-", 1.0, &XIN)) );
         parameter_map.insert(para_item("ALPHAN",    PARA("ALPHAN",    "", "-", 1.0, &ALPHAN)) );
         parameter_map.insert(para_item("MUP.MIN",    PARA("MUP.MIN",    "", "cm*cm/V/s", cm*cm/V/s, &MUP_MIN)) );
         parameter_map.insert(para_item("MUP.MAX",    PARA("MUP.MAX",    "", "cm*cm/V/s",cm*cm/V/s , &MUP_MAX)) );
         parameter_map.insert(para_item("NREFP",    PARA("NREFP",    "", "cm^-3",std::pow(cm,-3) , &NREFP)) );
         parameter_map.insert(para_item("NUP",    PARA("NUP",    "", "-",1.0 , &NUP)) );
         parameter_map.insert(para_item("XIP",    PARA("XIP",    "", "-",1.0 , &XIP)) );
         parameter_map.insert(para_item("ALPHAP",    PARA("ALPHAP",    "", "-", 1.0, &ALPHAP)) );
#endif
  }

private:
  //---------------------------------------------------------------------------
  // Electron low field mobility
  PetscScalar ElecMobLowField(const PetscScalar &Tl) const
  {
    PetscScalar Na = ReadDopingNa();
    PetscScalar Nd = ReadDopingNd();
    return MUN_MIN+(MUN_MAX*std::pow(Tl/T300,NUN)-MUN_MIN)/ \
           (1+std::pow(Tl/T300,XIN)*std::pow((Na+Nd)/NREFN,ALPHAN));
  }
  AutoDScalar ElecMobLowField(const AutoDScalar &Tl) const
  {
    PetscScalar Na = ReadDopingNa();
    PetscScalar Nd = ReadDopingNd();
    return MUN_MIN+(MUN_MAX*adtl::pow(Tl/T300,NUN)-MUN_MIN)/ \
           (1+adtl::pow(Tl/T300,XIN)*std::pow((Na+Nd)/NREFN,ALPHAN));
  }

  //---------------------------------------------------------------------------
  // Hole low field mobility
  PetscScalar HoleMobLowField(const PetscScalar &Tl) const
  {
    PetscScalar Na = ReadDopingNa();
    PetscScalar Nd = ReadDopingNd();
    return MUP_MIN+(MUP_MAX*std::pow(Tl/T300,NUP)-MUP_MIN)/ \
           (1+std::pow(Tl/T300,XIP)*std::pow((Na+Nd)/NREFP,ALPHAP));
  }
  AutoDScalar HoleMobLowField(const AutoDScalar &Tl) const
  {
    PetscScalar Na = ReadDopingNa();
    PetscScalar Nd = ReadDopingNd();
    return MUP_MIN+(MUP_MAX*adtl::pow(Tl/T300,NUP)-MUP_MIN)/ \
           (1+adtl::pow(Tl/T300,XIP)*std::pow((Na+Nd)/NREFP,ALPHAP));
  }


public:
  // Hall mobility factor  for electrons
  PetscScalar RH_ELEC()  { return 1.1; }

  // Hall mobility factor  for holes
  PetscScalar RH_HOLE()  { return 0.7; }

  //---------------------------------------------------------------------------
  // Electron mobility
  PetscScalar ElecMob(const PetscScalar &p, const PetscScalar &n, const PetscScalar &Tl,
                      const PetscScalar &Ep, const PetscScalar &Et, const PetscScalar &Tn) const
  {
    return ElecMobLowField(Tl);
  }
  AutoDScalar ElecMob(const AutoDScalar &p, const AutoDScalar &n, const AutoDScalar &Tl,
                      const AutoDScalar &Ep, const AutoDScalar &Et, const AutoDScalar &Tn) const
  {
    return ElecMobLowField(Tl);
  }

  //---------------------------------------------------------------------------
  // Hole mobility
  PetscScalar HoleMob (const PetscScalar &p, const PetscScalar &n, const PetscScalar &Tl,
                       const PetscScalar &Ep, const PetscScalar &Et, const PetscScalar &Tp) const
  {
    return HoleMobLowField(Tl);
  }
  AutoDScalar HoleMob(const AutoDScalar &p, const AutoDScalar &n, const AutoDScalar &Tl,
                      const AutoDScalar &Ep, const AutoDScalar &Et, const AutoDScalar &Tp) const
  {
    return HoleMobLowField(Tl);
  }


// constructor and destructor
public:
  GSS_PolySi_Mob_Analytic(const PMIS_Environment &env):PMIS_Mobility(env)
  {
    Mob_Analytic_Init();
  }


  ~GSS_PolySi_Mob_Analytic()
  {
  }

}
;

/*---------------------------------------------------------------
 *  the interface function called by material databse controller
 *  use Analytic model as default mobility model
 */
extern "C"
{
  DLL_EXPORT_DECLARE PMIS_Mobility* PMIS_PolySi_Mob_Default (const PMIS_Environment& env)
  {
    return new GSS_PolySi_Mob_Analytic(env);
  }
}
/* alias */
extern "C"
{
  DLL_EXPORT_DECLARE PMIS_Mobility* PMIS_PolySi_Mob_Analytic (const PMIS_Environment& env)
  {
    return new GSS_PolySi_Mob_Analytic(env);
  }
}
