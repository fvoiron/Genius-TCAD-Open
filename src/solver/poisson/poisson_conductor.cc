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

//  $Id: poisson_conductor.cc,v 1.11 2008/07/09 05:58:16 gdiso Exp $

#include "elem.h"
#include "simulation_system.h"
#include "conductor_region.h"

using PhysicalUnit::kb;
using PhysicalUnit::e;
using PhysicalUnit::eps0;

///////////////////////////////////////////////////////////////////////
//----------------Function and Jacobian evaluate---------------------//
///////////////////////////////////////////////////////////////////////


void ElectrodeSimulationRegion::Poissin_Fill_Value(Vec x, Vec L)
{
  std::vector<int> ix;
  std::vector<PetscScalar> y;
  std::vector<PetscScalar> s;

  ix.reserve(this->n_node());
  y.reserve(this->n_node());
  s.reserve(this->n_node());

  const PetscScalar eps = eps0*mt->basic->Permittivity();

  const_processor_node_iterator node_it = on_processor_nodes_begin();
  const_processor_node_iterator node_it_end = on_processor_nodes_end();
  for(; node_it!=node_it_end; ++node_it)
  {
    const FVM_Node * fvm_node = *node_it;
    const FVM_NodeData * node_data = fvm_node->node_data();

    ix.push_back(fvm_node->global_offset());
    y.push_back(node_data->psi());
    s.push_back(1.0/(eps*fvm_node->volume()));
  }

  if( ix.size() )
  {
    VecSetValues(x, ix.size(), &ix[0], &y[0], INSERT_VALUES) ;
    VecSetValues(L, ix.size(), &ix[0], &s[0], INSERT_VALUES) ;
  }

}

/*---------------------------------------------------------------------
 * build function and its jacobian for poisson solver
 */
void ElectrodeSimulationRegion::Poissin_Function(PetscScalar * x, Vec f, InsertMode &add_value_flag)
{

  // note, we will use ADD_VALUES to set values of vec f
  // if the previous operator is not ADD_VALUES, we should assembly the vec
  if( (add_value_flag != ADD_VALUES) && (add_value_flag != NOT_SET_VALUES) )
  {
    VecAssemblyBegin(f);
    VecAssemblyEnd(f);
  }

  // set local buf here
  std::vector<int>          iy;
  std::vector<PetscScalar>  y;
  iy.reserve(2*n_edge());
  y.reserve(2*n_edge());

  // search all the edges of this region, do integral over control volume...
  const_edge_iterator it = edges_begin();
  const_edge_iterator it_end = edges_end();
  for(; it!=it_end; ++it)
  {
    // fvm_node of node1
    const FVM_Node * fvm_n1 = (*it).first;
    // fvm_node of node2
    const FVM_Node * fvm_n2 = (*it).second;

    // fvm_node_data of node1
    const FVM_NodeData * n1_data =  fvm_n1->node_data();
    // fvm_node_data of node2
    const FVM_NodeData * n2_data =  fvm_n2->node_data();

    const unsigned int n1_local_offset = fvm_n1->local_offset();
    const unsigned int n2_local_offset = fvm_n2->local_offset();

    {
      // electrostatic potential, as independent variable
      PetscScalar V1   =  x[n1_local_offset];
      PetscScalar eps1 =  n1_data->eps();


      PetscScalar V2   =  x[n2_local_offset];
      PetscScalar eps2 =  n2_data->eps();

      PetscScalar eps = 0.5*(eps1+eps2);

      // "flux" from node 2 to node 1
      PetscScalar f =  eps*fvm_n1->cv_surface_area(fvm_n2)*(V2 - V1)/fvm_n1->distance(fvm_n2) ;

      // ignore thoese ghost nodes
      if( fvm_n1->on_processor() )
      {
        iy.push_back(fvm_n1->global_offset());
        y.push_back(f);
      }

      if( fvm_n2->on_processor() )
      {
        iy.push_back(fvm_n2->global_offset());
        y.push_back(-f);
      }
    }
  }

  if(iy.size()) VecSetValues(f, iy.size(), &iy[0], &y[0], ADD_VALUES);

  // after the first scan, every nodes are updated.
  // however, boundary condition should be processed later.

  // the last operator is ADD_VALUES
  add_value_flag = ADD_VALUES;

}



/*---------------------------------------------------------------------
 * build function and its jacobian for poisson solver
 */
void ElectrodeSimulationRegion::Poissin_Jacobian(PetscScalar * x, Mat *jac, InsertMode &add_value_flag)
{

  // note, we will use ADD_VALUES to set values of matrix J
  // if the previous operator is not ADD_VALUES, we should flush the matrix
  if( add_value_flag != ADD_VALUES && add_value_flag != NOT_SET_VALUES)
  {
    MatAssemblyBegin(*jac, MAT_FLUSH_ASSEMBLY);
    MatAssemblyEnd(*jac, MAT_FLUSH_ASSEMBLY);
  }

  //the indepedent variable number, since we only process edges, 2 is enough
  adtl::AutoDScalar::numdir=2;

  //synchronize with material database
  mt->set_ad_num(adtl::AutoDScalar::numdir);


  // search all the edges of this region, do integral over control volume...
  const_edge_iterator it = edges_begin();
  const_edge_iterator it_end = edges_end();
  for(; it!=it_end; ++it)
  {
    // fvm_node of node1
    const FVM_Node * fvm_n1 = (*it).first;
    // fvm_node of node2
    const FVM_Node * fvm_n2 = (*it).second;

    // fvm_node_data of node1
    const FVM_NodeData * n1_data =  fvm_n1->node_data();
    // fvm_node_data of node2
    const FVM_NodeData * n2_data =  fvm_n2->node_data();

    const unsigned int n1_local_offset = fvm_n1->local_offset();
    const unsigned int n2_local_offset = fvm_n2->local_offset();

    // the row/colume position of variables in the matrix
    PetscInt row[2],col[2];
    row[0] = col[0] = fvm_n1->global_offset();
    row[1] = col[1] = fvm_n2->global_offset();

    // here we use AD, however it is great overkill for such a simple problem.
    {
      // electrostatic potential, as independent variable
      AutoDScalar V1   =  x[n1_local_offset];   V1.setADValue(0,1.0);
      PetscScalar eps1 =  n1_data->eps();


      AutoDScalar V2   =  x[n2_local_offset];   V2.setADValue(1,1.0);
      PetscScalar eps2 =  n2_data->eps();

      PetscScalar eps = 0.5*(eps1+eps2);

      AutoDScalar f =  eps*fvm_n1->cv_surface_area(fvm_n2)*(V2 - V1)/fvm_n1->distance(fvm_n2) ;

      // ignore thoese ghost nodes
      if( fvm_n1->on_processor() )
      {
        MatSetValues(*jac, 1, &row[0], 2, &col[0], f.getADValue(), ADD_VALUES);
      }

      if( fvm_n2->on_processor() )
      {
        MatSetValues(*jac, 1, &row[1], 2, &col[0], (-f).getADValue(), ADD_VALUES);
      }
    }
  }


  // boundary condition should be processed later!

  // the last operator is ADD_VALUES
  add_value_flag = ADD_VALUES;

}


#if 0
/*---------------------------------------------------------------------
 * build function and its jacobian for poisson solver
 */
void ElectrodeSimulationRegion::Poissin_Function(PetscScalar * x, Vec f, InsertMode &add_value_flag)
{

  // note, we will use ADD_VALUES to set values of vec f
  // if the previous operator is not ADD_VALUES, we should assembly the vec
  if( (add_value_flag != ADD_VALUES) && (add_value_flag != NOT_SET_VALUES) )
  {
    VecAssemblyBegin(f);
    VecAssemblyEnd(f);
  }

  // set local buf here
  std::vector<int>          iy;
  std::vector<PetscScalar>  y;
  // slightly overkill -- the HEX8 element has 12 edges, each edge has 2 node
  iy.reserve(24*this->n_cell()+this->n_node());
  y.reserve(24*this->n_cell()+this->n_node());

  // search all the element in this region.
  // note, they are all local element, thus must be processed

  const_element_iterator it = elements_begin();
  const_element_iterator it_end = elements_end();
  for(; it!=it_end; ++it)
  {
    const Elem * elem = *it;
    // for conductor, \rho is always zero. no need to process it.
    // the poisson's equation becoms laplace equation now.

    // search for all the Edge this cell own
    for(unsigned int ne=0; ne<elem->n_edges(); ++ne )
    {
      std::pair<unsigned int, unsigned int> edge_nodes;
      elem->nodes_on_edge(ne, edge_nodes);

      // the length of this edge
      const double length = elem->edge_length(ne);

      // partial area associated with this edge
      double partial_area = elem->partial_area_with_edge(ne);

      // fvm_node of node1
      const FVM_Node * fvm_n1 = elem->get_fvm_node(edge_nodes.first);
      // fvm_node of node2
      const FVM_Node * fvm_n2 = elem->get_fvm_node(edge_nodes.second);

      // fvm_node_data of node1
      const FVM_NodeData * n1_data = fvm_n1->node_data();
      // fvm_node_data of node2
      const FVM_NodeData * n2_data = fvm_n2->node_data();

      unsigned int n1_local_offset = fvm_n1->local_offset();
      unsigned int n2_local_offset = fvm_n2->local_offset();

      // build nonlinear poisson's equation
      {

        PetscScalar V1   =  x[n1_local_offset]; // electrostatic potential of node1
        PetscScalar eps1 =  n1_data->eps();     // permittivity of node1

        PetscScalar V2   =  x[n2_local_offset]; // electrostatic potential of node2
        PetscScalar eps2 =  n2_data->eps();     // permittivity of node2

        PetscScalar eps = 0.5*(eps1+eps2);      // eps at mid point of the edge

        // ignore thoese ghost nodes (ghost nodes is local but with different processor_id())
        if( fvm_n1->root_node()->processor_id()==Genius::processor_id() )
        {
          iy.push_back( fvm_n1->global_offset() );
          y.push_back ( eps*partial_area*(V2 - V1)/length );
        }

        if( fvm_n2->root_node()->processor_id()==Genius::processor_id() )
        {
          iy.push_back( fvm_n2->global_offset() );
          y.push_back ( eps*partial_area*(V1 - V2)/length );
        }
      }

    }

  }

  if(iy.size()) VecSetValues(f, iy.size(), &iy[0], &y[0], ADD_VALUES);

  // after the first scan, every nodes are updated.
  // however, boundary condition should be processed later.

  // the last operator is ADD_VALUES
  add_value_flag = ADD_VALUES;

}






/*---------------------------------------------------------------------
 * build function and its jacobian for poisson solver
 */
void ElectrodeSimulationRegion::Poissin_Jacobian(PetscScalar * x, Mat *jac, InsertMode &add_value_flag)
{


  // note, we will use ADD_VALUES to set values of matrix J
  // if the previous operator is not ADD_VALUES, we should flush the matrix
  if( (add_value_flag != ADD_VALUES) && (add_value_flag != NOT_SET_VALUES) )
  {
    MatAssemblyBegin(*jac, MAT_FLUSH_ASSEMBLY);
    MatAssemblyEnd(*jac, MAT_FLUSH_ASSEMBLY);
  }

  //the indepedent variable number, since we only process edges, 2 is enough
  adtl::AutoDScalar::numdir=2;

  //synchronize with material database
  mt->set_ad_num(adtl::AutoDScalar::numdir);

  // search all the element in this region.
  // note, they are all local element, thus must be processed

  const_element_iterator it = elements_begin();
  const_element_iterator it_end = elements_end();
  for(; it!=it_end; ++it)
  {
    const Elem * elem = *it;

    // for conductor, \rho is always zero. no need to process it.
    // the poisson's equation becoms laplace equation now.

    //search for all the Edge this cell own
    for(unsigned int ne=0; ne<elem->n_edges(); ++ne )
    {
      std::pair<unsigned int, unsigned int> edge_nodes;
      elem->nodes_on_edge(ne, edge_nodes);

      // the length of this edge
      const double length = elem->edge_length(ne);

      // partial area associated with this edge
      const double partial_area = elem->partial_area_with_edge(ne);

      // fvm_node of node1
      const FVM_Node * fvm_n1 = elem->get_fvm_node(edge_nodes.first);
      // fvm_node of node2
      const FVM_Node * fvm_n2 = elem->get_fvm_node(edge_nodes.second);

      // fvm_node_data of node1
      const FVM_NodeData * n1_data =  fvm_n1->node_data();
      // fvm_node_data of node2
      const FVM_NodeData * n2_data =  fvm_n2->node_data();

      const unsigned int n1_local_offset = fvm_n1->local_offset();
      const unsigned int n2_local_offset = fvm_n2->local_offset();

      // the row/colume position of variables in the matrix
      PetscInt row[2],col[2];
      row[0] = col[0] = fvm_n1->global_offset();
      row[1] = col[1] = fvm_n2->global_offset();

      // here we use AD, however it is great overkill for such a simple problem.
      {

        // electrostatic potential, as independent variable
        AutoDScalar V1   =  x[n1_local_offset];   V1.setADValue(0,1.0);
        PetscScalar eps1 =  n1_data->eps();


        AutoDScalar V2   =  x[n2_local_offset];   V2.setADValue(1,1.0);
        PetscScalar eps2 =  n2_data->eps();

        PetscScalar eps = 0.5*(eps1+eps2);

        // ignore thoese ghost nodes
        if( fvm_n1->root_node()->processor_id()==Genius::processor_id() )
        {
          AutoDScalar f =  eps*partial_area*(V2 - V1)/length ;
          MatSetValues(*jac, 1, &row[0], 2, &col[0], f.getADValue(), ADD_VALUES);
        }

        if( fvm_n2->root_node()->processor_id()==Genius::processor_id() )
        {
          AutoDScalar f =  eps*partial_area*(V1 - V2)/length ;
          MatSetValues(*jac, 1, &row[1], 2, &col[0], f.getADValue(), ADD_VALUES);
        }
      }
    }


  }

  // boundary condition should be processed later!

  // the last operator is ADD_VALUES
  add_value_flag = ADD_VALUES;

}
#endif


void ElectrodeSimulationRegion::Poissin_Update_Solution(PetscScalar *lxx)
{

  local_node_iterator node_it = on_local_nodes_begin();
  local_node_iterator node_it_end = on_local_nodes_end();
  for(; node_it!=node_it_end; ++node_it)
  {
    FVM_Node * fvm_node = *node_it;
    FVM_NodeData * node_data = fvm_node->node_data();

    //update psi
    node_data->psi() = lxx[fvm_node->local_offset()];
  }

  // addtional work: compute electrical field for all the node.
  // however, the electrical field is always zero. We needn't do anything here.
  for(unsigned int n=0; n<n_cell(); ++n)
  {
    FVM_CellData * elem_data = this->get_region_elem_data(n);
    elem_data->E()  = VectorValue<PetscScalar>(0.0, 0.0, 0.0);
  }
}




