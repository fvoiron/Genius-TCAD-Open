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

//  $Id: petsc_type.cc,v 1.3 2008/07/09 05:58:16 gdiso Exp $

#include <string>
#include <map>

#include "petsc_type.h"

// convert string to enum

namespace SolverSpecify
{

  std::map<std::string, NonLinearSolverType> NonLinearSolverName_to_NonLinearSolverType;

  void init_NonLinearSolverName_to_NonLinearSolverType()
  {
    if ( NonLinearSolverName_to_NonLinearSolverType.empty() )
    {
      NonLinearSolverName_to_NonLinearSolverType["newton"     ]  = Newton;
      NonLinearSolverName_to_NonLinearSolverType["basic"      ]  = Newton;
      NonLinearSolverName_to_NonLinearSolverType["linesearch" ]  = LineSearch;
      NonLinearSolverName_to_NonLinearSolverType["trustregion"]  = TrustRegion;
    }

  }

  NonLinearSolverType nonlinear_solver_type(const std::string & ns)
  {
    init_NonLinearSolverName_to_NonLinearSolverType();
    return NonLinearSolverName_to_NonLinearSolverType[ns];
  }


  //-------------------------------------------------------------------------------------

  std::map<std::string, LinearSolverType>    LinearSolverName_to_LinearSolverType;

  void init_LinearSolverName_to_LinearSolverType()
  {
    if ( LinearSolverName_to_LinearSolverType.empty() )
    {
      LinearSolverName_to_LinearSolverType["cg"          ]  = CG;
      LinearSolverName_to_LinearSolverType["cgn"         ]  = CGN;
      LinearSolverName_to_LinearSolverType["cgs"         ]  = CGS;
      LinearSolverName_to_LinearSolverType["cr"          ]  = CR;
      LinearSolverName_to_LinearSolverType["qmr"         ]  = QMR;
      LinearSolverName_to_LinearSolverType["tcqmr"       ]  = TCQMR;
      LinearSolverName_to_LinearSolverType["tfqmr"       ]  = TFQMR;
      LinearSolverName_to_LinearSolverType["bicg"        ]  = BICG;
      LinearSolverName_to_LinearSolverType["bcgs"        ]  = BCGSL;//BCGSL is more stable
      LinearSolverName_to_LinearSolverType["bicgstable"  ]  = BICGSTAB;
      LinearSolverName_to_LinearSolverType["bcgsl"       ]  = BCGSL;

      LinearSolverName_to_LinearSolverType["minres"      ]  = MINRES;
      LinearSolverName_to_LinearSolverType["gmres"       ]  = GMRES;
      LinearSolverName_to_LinearSolverType["dgmres"      ]  = DGMRES;
      LinearSolverName_to_LinearSolverType["fgmres"      ]  = FGMRES;
      LinearSolverName_to_LinearSolverType["lsqr"        ]  = LSQR;

      LinearSolverName_to_LinearSolverType["jacobian"    ]  = JACOBI;
      LinearSolverName_to_LinearSolverType["sor_forward" ]  = SOR_FORWARD;
      LinearSolverName_to_LinearSolverType["sor_backward"]  = SOR_BACKWARD;

      LinearSolverName_to_LinearSolverType["ssor"        ]  = SSOR;
      LinearSolverName_to_LinearSolverType["richardson"  ]  = RICHARDSON;

      LinearSolverName_to_LinearSolverType["chebyshev"   ]  = CHEBYSHEV;
      LinearSolverName_to_LinearSolverType["lu"          ]  = LU;
      LinearSolverName_to_LinearSolverType["umfpack"     ]  = UMFPACK;
      LinearSolverName_to_LinearSolverType["superlu"     ]  = SuperLU;
      LinearSolverName_to_LinearSolverType["pastix"      ]  = PASTIX;
      LinearSolverName_to_LinearSolverType["mumps"       ]  = MUMPS;
      LinearSolverName_to_LinearSolverType["superlu_dist"]  = SuperLU_DIST;
      LinearSolverName_to_LinearSolverType["gss"         ]  = GSS;
    }

  }

  LinearSolverType linear_solver_type(const std::string & ls)
  {
    init_LinearSolverName_to_LinearSolverType();
    return LinearSolverName_to_LinearSolverType[ls];
  }

  //-------------------------------------------------------------------------------------

  std::map<std::string, PreconditionerType>  PreconditionerName_to_PreconditionerType;

  void init_PreconditionerName_to_PreconditionerType()
  {
    if ( PreconditionerName_to_PreconditionerType.empty() )
    {
      PreconditionerName_to_PreconditionerType["identity"    ]  = IDENTITY_PRECOND;
      PreconditionerName_to_PreconditionerType["jacobian"    ]  = JACOBI_PRECOND;
      PreconditionerName_to_PreconditionerType["bjacobian"   ]  = BLOCK_JACOBI_PRECOND;
      PreconditionerName_to_PreconditionerType["sor"         ]  = SOR_PRECOND;
      PreconditionerName_to_PreconditionerType["ssor"        ]  = SSOR_PRECOND;
      PreconditionerName_to_PreconditionerType["asm"         ]  = ASM_PRECOND;
      PreconditionerName_to_PreconditionerType["asmilu0"     ]  = ASMILU0_PRECOND;
      PreconditionerName_to_PreconditionerType["asmilu1"     ]  = ASMILU1_PRECOND;
      PreconditionerName_to_PreconditionerType["asmilu2"     ]  = ASMILU2_PRECOND;
      PreconditionerName_to_PreconditionerType["asmilu3"     ]  = ASMILU3_PRECOND;
      PreconditionerName_to_PreconditionerType["asmlu"       ]  = ASMLU_PRECOND;
      PreconditionerName_to_PreconditionerType["amg"         ]  = BOOMERAMG_PRECOND;
      PreconditionerName_to_PreconditionerType["bjacobian"   ]  = CHOLESKY_PRECOND;
      PreconditionerName_to_PreconditionerType["eisenstat"   ]  = EISENSTAT_PRECOND;
      PreconditionerName_to_PreconditionerType["icc"         ]  = ICC_PRECOND;
      PreconditionerName_to_PreconditionerType["ilu"         ]  = ILU_PRECOND;
      PreconditionerName_to_PreconditionerType["ilut"        ]  = ILUT_PRECOND;
      PreconditionerName_to_PreconditionerType["lu"          ]  = LU_PRECOND;
      PreconditionerName_to_PreconditionerType["parms"       ]  = PARMS_PRECOND;
    }
  }

  PreconditionerType preconditioner_type(const std::string & pc)
  {
    init_PreconditionerName_to_PreconditionerType();
    return PreconditionerName_to_PreconditionerType[pc];
  }


  //-------------------------------------------------------------------------------------

  LinearSolverCategory  linear_solver_category(LinearSolverType ls)
  {
    switch(ls)
    {
      case CG           :
      case CGN          :
      case CGS          :
      case CR           :
      case QMR          :
      case TCQMR        :
      case TFQMR        :
      case BICG         :
      case BICGSTAB     :
      case BCGSL        :
      case MINRES       :
      case GMRES        :
      case DGMRES       :
      case FGMRES       :
      case LSQR         :
      case JACOBI       :
      case SOR_FORWARD  :
      case SOR_BACKWARD :
      case SSOR         :
      case RICHARDSON   : return ITERATIVE;
      case CHEBYSHEV    :// direct solvers
      case LU           :
      case UMFPACK      :
      case SuperLU      :
      case PASTIX       :
      case MUMPS        :
      case SuperLU_DIST :
      case GSS          : return DIRECT;
    }

    return HYBRID;
  }


}


