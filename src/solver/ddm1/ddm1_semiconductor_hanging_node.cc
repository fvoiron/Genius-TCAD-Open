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

#include "elem.h"
#include "simulation_system.h"
#include "semiconductor_region.h"
#include "jflux1.h"
#include "petsc_utils.h"


using PhysicalUnit::kb;
using PhysicalUnit::e;


void SemiconductorSimulationRegion::DDM1_Function_Hanging_Node(PetscScalar *x, Vec f, InsertMode &add_value_flag)
{

  //common used variable
  PetscScalar T   = T_external();
  PetscScalar Vt  = kb*T/e;

  // process 2D hanging node
  if(this->has_2d_hanging_node())
  {
    // buffer for record src and dst rows
    std::vector<PetscInt>    src_row;
    std::vector<PetscInt>    dst_row;
    std::vector<PetscScalar> alpha_buffer;

    // buffer for record position and value for insert
    std::vector<PetscInt>    insert_index;
    std::vector<PetscScalar> insert_buffer;

    hanging_node_on_elem_side_iterator  hanging_node_it = hanging_node_on_elem_side_begin();
    hanging_node_on_elem_side_iterator  hanging_node_it_end = hanging_node_on_elem_side_end();

    for(; hanging_node_it!=hanging_node_it_end; ++hanging_node_it )
    {
      const FVM_Node * fvm_node = (*hanging_node_it).first;

      // skip node not belongs to this processor
      if( fvm_node->root_node()->processor_id()!=Genius::processor_id() ) continue;

      // let the flux of hanging node flow into other "non-hanging" node on the element side averagely
      // this process ensure the global conservation of flux

      const Elem * elem = (*hanging_node_it).second.first;
      unsigned int side_index = (*hanging_node_it).second.second;

      AutoPtr<Elem>  side = elem->build_side(side_index);

      unsigned int n_side_node = side->n_nodes();
      std::vector<const FVM_Node *> side_fvm_nodes;

      for(unsigned int n=0; n < n_side_node; n++)
      {
        const Node * side_node = side->get_node(n);
        const FVM_Node * side_fvm_node = region_fvm_node(side_node);
        side_fvm_nodes.push_back(side_fvm_node);

        src_row.push_back(fvm_node->global_offset()+0);
        dst_row.push_back(side_fvm_node->global_offset()+0);
        alpha_buffer.push_back(1.0/n_side_node);

        src_row.push_back(fvm_node->global_offset()+1);
        dst_row.push_back(side_fvm_node->global_offset()+1);
        alpha_buffer.push_back(1.0/n_side_node);

        src_row.push_back(fvm_node->global_offset()+2);
        dst_row.push_back(side_fvm_node->global_offset()+2);
        alpha_buffer.push_back(1.0/n_side_node);
      }

      // and then, the value of hanging node is interpolated.
      genius_assert(side_fvm_nodes.size() == 2);

      // find the interpolation point
      const FVM_Node * interpolation_p1 = side_fvm_nodes[0];
      const FVM_Node * interpolation_p2 = side_fvm_nodes[1];

      PetscScalar V = x[fvm_node->local_offset()+0];
      PetscScalar n = x[fvm_node->local_offset()+1];
      PetscScalar p = x[fvm_node->local_offset()+2];

      PetscScalar V1 = x[interpolation_p1->local_offset()+0];
      PetscScalar n1 = x[interpolation_p1->local_offset()+1];
      PetscScalar p1 = x[interpolation_p1->local_offset()+2];

      PetscScalar V2 = x[interpolation_p2->local_offset()+0];
      PetscScalar n2 = x[interpolation_p2->local_offset()+1];
      PetscScalar p2 = x[interpolation_p2->local_offset()+2];

      // the psi is linear interpolated
      insert_index.push_back(fvm_node->global_offset()+0);
      insert_buffer.push_back( (V - 0.5*(V1+V2)) );

      // the electron density is interpolated by S-G scheme
      insert_index.push_back(fvm_node->global_offset()+1);
      insert_buffer.push_back( (n - nmid_dd(Vt, V1, V2, n1, n2)) );
      //insert_buffer.push_back( V1>V2 ? n-n2 : n-n1 );

      //src_row.push_back(fvm_node->global_offset()+1);
      //dst_row.push_back( V1 > V2 ? interpolation_p1->global_offset()+1 : interpolation_p2->global_offset()+1);
      //alpha_buffer.push_back(1.0);


      // the hole density is interpolated by S-G scheme
      insert_index.push_back(fvm_node->global_offset()+2);
      insert_buffer.push_back( (p - pmid_dd(Vt, V1, V2, p1, p2)) );
      //insert_buffer.push_back( V1>V2 ? p-p1 : p-p2 );

      //src_row.push_back(fvm_node->global_offset()+2);
      //dst_row.push_back(V1 > V2 ? interpolation_p2->global_offset()+2 : interpolation_p1->global_offset()+2);
      //alpha_buffer.push_back(1.0);

    }

    PetscUtils::VecAddRowToRow(f, src_row, dst_row, alpha_buffer);

    // do INSERT_VALUES to Vec
    if(insert_index.size())
      VecSetValues(f, insert_index.size(), &insert_index[0], &insert_buffer[0], INSERT_VALUES);

    add_value_flag = INSERT_VALUES;

    return;
  }



  // process 3D hanging node
  if( this->has_3d_hanging_node() )
  {
    // process hanging node lies on side center
    {
      // buffer for record src and dst rows
      std::vector<PetscInt>    src_row;
      std::vector<PetscInt>    dst_row;
      std::vector<PetscScalar> alpha_buffer;

      // buffer for record position and value for insert
      std::vector<PetscInt>    insert_index;
      std::vector<PetscScalar> insert_buffer;

      hanging_node_on_elem_side_iterator  hanging_node_it = hanging_node_on_elem_side_begin();
      hanging_node_on_elem_side_iterator  hanging_node_it_end = hanging_node_on_elem_side_end();

      for(; hanging_node_it!=hanging_node_it_end; ++hanging_node_it )
      {
        const FVM_Node * fvm_node = (*hanging_node_it).first;

        // skip node not belongs to this processor
        if( fvm_node->root_node()->processor_id()!=Genius::processor_id() ) continue;

        // let the flux of hanging node flow into other "non-hanging" node on the element side averagely
        // this process ensure the global conservation of flux

        const Elem * elem = (*hanging_node_it).second.first;
        unsigned int side_index = (*hanging_node_it).second.second;

        AutoPtr<Elem>  side = elem->build_side(side_index);

        unsigned int n_side_node = side->n_nodes();
        std::vector<const FVM_Node *> side_fvm_nodes;

        for(unsigned int n=0; n < n_side_node; n++)
        {
          const Node * side_node = side->get_node(n);
          const FVM_Node * side_fvm_node = region_fvm_node(side_node);
          side_fvm_nodes.push_back(side_fvm_node);

          src_row.push_back(fvm_node->global_offset()+0);
          dst_row.push_back(side_fvm_node->global_offset()+0);
          alpha_buffer.push_back(1.0/n_side_node);

          src_row.push_back(fvm_node->global_offset()+1);
          dst_row.push_back(side_fvm_node->global_offset()+1);
          alpha_buffer.push_back(1.0/n_side_node);

          src_row.push_back(fvm_node->global_offset()+2);
          dst_row.push_back(side_fvm_node->global_offset()+2);
          alpha_buffer.push_back(1.0/n_side_node);

        }

        // and then, the value of hanging node is interpolated.

        const FVM_Node * interpolation_p1;
        const FVM_Node * interpolation_p2;

        // find the interpolation point
        genius_assert(side_fvm_nodes.size() == 4);
        {
          // for 3D case, the side should be QUAD4, we use 2 point which has little psi difference as interpolation point
          PetscScalar dv1 = std::abs( x[side_fvm_nodes[0]->local_offset()] - x[side_fvm_nodes[2]->local_offset()] );
          PetscScalar dv2 = std::abs( x[side_fvm_nodes[1]->local_offset()] - x[side_fvm_nodes[3]->local_offset()] );

          if( dv1 < dv2 )
          {
            interpolation_p1 = side_fvm_nodes[0];
            interpolation_p2 = side_fvm_nodes[2];
          }
          else
          {
            interpolation_p1 = side_fvm_nodes[1];
            interpolation_p2 = side_fvm_nodes[3];
          }
        }

        PetscScalar V = x[fvm_node->local_offset()+0];
        PetscScalar n = x[fvm_node->local_offset()+1];
        PetscScalar p = x[fvm_node->local_offset()+2];

        PetscScalar V1 = x[interpolation_p1->local_offset()+0];
        PetscScalar n1 = x[interpolation_p1->local_offset()+1];
        PetscScalar p1 = x[interpolation_p1->local_offset()+2];

        PetscScalar V2 = x[interpolation_p2->local_offset()+0];
        PetscScalar n2 = x[interpolation_p2->local_offset()+1];
        PetscScalar p2 = x[interpolation_p2->local_offset()+2];

        // the psi is linear interpolated
        insert_index.push_back(fvm_node->global_offset()+0);
        insert_buffer.push_back( (V - 0.5*(V1+V2)) );

        // the electron density is interpolated by S-G scheme
        insert_index.push_back(fvm_node->global_offset()+1);
        insert_buffer.push_back( (n - nmid_dd(Vt, V1, V2, n1, n2)) );


        // the hole density is interpolated by S-G scheme
        insert_index.push_back(fvm_node->global_offset()+2);
        insert_buffer.push_back( (p - pmid_dd(Vt, V1, V2, p1, p2)) );

      }

      PetscUtils::VecAddRowToRow(f, src_row, dst_row, alpha_buffer);


      // do INSERT_VALUES to Vec
      if(insert_index.size())
        VecSetValues(f, insert_index.size(), &insert_index[0], &insert_buffer[0], INSERT_VALUES);

    }


    // process hanging node lies on edge center
    {
      // buffer for record src and dst rows
      std::vector<PetscInt>    src_row;
      std::vector<PetscInt>    dst_row;
      std::vector<PetscScalar> alpha_buffer;

      // buffer for record position and value for insert
      std::vector<PetscInt>    insert_index;
      std::vector<PetscScalar> insert_buffer;

      hanging_node_on_elem_edge_iterator  hanging_node_it = hanging_node_on_elem_edge_begin();
      hanging_node_on_elem_edge_iterator  hanging_node_it_end = hanging_node_on_elem_edge_end();

      for(; hanging_node_it!=hanging_node_it_end; ++hanging_node_it )
      {
        const FVM_Node * fvm_node = (*hanging_node_it).first;

        // skip node not belongs to this processor
        if( fvm_node->root_node()->processor_id()!=Genius::processor_id() ) continue;

        // let the flux of hanging node flow into other "non-hanging" node on the element side averagely
        // this process ensure the global conservation of flux

        const Elem * elem = (*hanging_node_it).second.first;
        unsigned int edge_index = (*hanging_node_it).second.second;

        AutoPtr<Elem>  edge = elem->build_edge(edge_index);

        unsigned int n_edge_node = 2;
        std::vector<const FVM_Node *> edge_fvm_nodes;

        for(unsigned int n=0; n < n_edge_node; n++)
        {
          const Node * edge_node = edge->get_node(n);
          const FVM_Node * edge_fvm_node = region_fvm_node(edge_node);
          genius_assert(edge_fvm_node!=NULL);

          edge_fvm_nodes.push_back(edge_fvm_node);

          src_row.push_back(fvm_node->global_offset()+0);
          src_row.push_back(fvm_node->global_offset()+1);
          src_row.push_back(fvm_node->global_offset()+2);

          dst_row.push_back(edge_fvm_node->global_offset()+0);
          dst_row.push_back(edge_fvm_node->global_offset()+1);
          dst_row.push_back(edge_fvm_node->global_offset()+2);

          alpha_buffer.push_back(1.0/n_edge_node);
          alpha_buffer.push_back(1.0/n_edge_node);
          alpha_buffer.push_back(1.0/n_edge_node);
        }

        // and then, the value of hanging node is interpolated.
        PetscScalar V = x[fvm_node->local_offset()+0];
        PetscScalar n = x[fvm_node->local_offset()+1];
        PetscScalar p = x[fvm_node->local_offset()+2];

        PetscScalar V1 = x[edge_fvm_nodes[0]->local_offset()+0];
        PetscScalar n1 = x[edge_fvm_nodes[0]->local_offset()+1];
        PetscScalar p1 = x[edge_fvm_nodes[0]->local_offset()+2];

        PetscScalar V2 = x[edge_fvm_nodes[1]->local_offset()+0];
        PetscScalar n2 = x[edge_fvm_nodes[1]->local_offset()+1];
        PetscScalar p2 = x[edge_fvm_nodes[1]->local_offset()+2];

        // the psi is linear interpolated
        insert_index.push_back(fvm_node->global_offset()+0);
        insert_buffer.push_back( V - 0.5*(V1+V2) );

        // the electron density is interpolated by S-G scheme
        insert_index.push_back(fvm_node->global_offset()+1);
        insert_buffer.push_back( n - nmid_dd(Vt, V1, V2, n1, n2) );


        // the hole density is interpolated by S-G scheme
        insert_index.push_back(fvm_node->global_offset()+2);
        insert_buffer.push_back( p - pmid_dd(Vt, V1, V2, p1, p2) );
      }

      PetscUtils::VecAddRowToRow(f, src_row, dst_row, alpha_buffer);

      // do INSERT_VALUES to Vec
      if(insert_index.size())
        VecSetValues(f, insert_index.size(), &insert_index[0], &insert_buffer[0], INSERT_VALUES);
    }

    add_value_flag = INSERT_VALUES;
    return;

  }

}





void SemiconductorSimulationRegion::DDM1_Jacobian_Hanging_Node(PetscScalar *x, Mat *jac, InsertMode &add_value_flag)
{

  //common used variable
  PetscScalar T   = T_external();
  PetscScalar Vt  = kb*T/e;

  // process 2D hanging node
  if(this->has_2d_hanging_node())
  {

    // buffer for add matrix row to row
    std::vector<PetscInt>    src_row;
    std::vector<PetscInt>    dst_row;
    std::vector<PetscScalar> alpha_buffer;

    // buffer for matrix entrance
    std::vector< PetscInt >                 row_index;
    std::vector< std::vector<PetscInt> >    cols_index;
    std::vector< AutoDScalar >              ad_values;

    hanging_node_on_elem_side_iterator  hanging_node_it = hanging_node_on_elem_side_begin();
    hanging_node_on_elem_side_iterator  hanging_node_it_end = hanging_node_on_elem_side_end();

    for(; hanging_node_it!=hanging_node_it_end; ++hanging_node_it )
    {
      const FVM_Node * fvm_node = (*hanging_node_it).first;

      // skip node not belongs to this processor
      if( fvm_node->root_node()->processor_id()!=Genius::processor_id() ) continue;

      // let the flux of hanging node flow into other "non-hanging" node on the element side averagely
      // this process ensure the global conservation of flux

      const Elem * elem = (*hanging_node_it).second.first;
      unsigned int side_index = (*hanging_node_it).second.second;

      AutoPtr<Elem>  side = elem->build_side(side_index);

      unsigned int n_side_node = side->n_nodes();
      std::vector<const FVM_Node *> side_fvm_nodes;

      for(unsigned int n=0; n < n_side_node; n++)
      {
        const Node * side_node = side->get_node(n);
        const FVM_Node * side_fvm_node = region_fvm_node(side_node);
        side_fvm_nodes.push_back(side_fvm_node);

        src_row.push_back(fvm_node->global_offset()+0);
        dst_row.push_back(side_fvm_node->global_offset()+0);
        alpha_buffer.push_back(1.0/n_side_node);

        src_row.push_back(fvm_node->global_offset()+1);
        dst_row.push_back(side_fvm_node->global_offset()+1);
        alpha_buffer.push_back(1.0/n_side_node);

        src_row.push_back(fvm_node->global_offset()+2);
        dst_row.push_back(side_fvm_node->global_offset()+2);
        alpha_buffer.push_back(1.0/n_side_node);
      }

      // and then, the value of hanging node is interpolated.
      //the indepedent variable number, we need 9 here
      adtl::AutoDScalar::numdir = 9;

      // find the interpolation point
      genius_assert(side_fvm_nodes.size() == 2);
      const FVM_Node * interpolation_p1 = side_fvm_nodes[0];
      const FVM_Node * interpolation_p2 = side_fvm_nodes[1];

      AutoDScalar V = x[fvm_node->local_offset()+0]; V.setADValue(0, 1.0);
      AutoDScalar n = x[fvm_node->local_offset()+1]; n.setADValue(1, 1.0);
      AutoDScalar p = x[fvm_node->local_offset()+2]; p.setADValue(2, 1.0);

      AutoDScalar V1 = x[interpolation_p1->local_offset()+0]; V1.setADValue(3, 1.0);
      AutoDScalar n1 = x[interpolation_p1->local_offset()+1]; n1.setADValue(4, 1.0);
      AutoDScalar p1 = x[interpolation_p1->local_offset()+2]; p1.setADValue(5, 1.0);

      AutoDScalar V2 = x[interpolation_p2->local_offset()+0]; V2.setADValue(6, 1.0);
      AutoDScalar n2 = x[interpolation_p2->local_offset()+1]; n2.setADValue(7, 1.0);
      AutoDScalar p2 = x[interpolation_p2->local_offset()+2]; p2.setADValue(8, 1.0);

      std::vector<PetscInt> cols(9);
      cols[0] = fvm_node->global_offset()+0;
      cols[1] = fvm_node->global_offset()+1;
      cols[2] = fvm_node->global_offset()+2;
      cols[3] = interpolation_p1->global_offset()+0;
      cols[4] = interpolation_p1->global_offset()+1;
      cols[5] = interpolation_p1->global_offset()+2;
      cols[6] = interpolation_p2->global_offset()+0;
      cols[7] = interpolation_p2->global_offset()+1;
      cols[8] = interpolation_p2->global_offset()+2;

      // the psi is linear interpolated
      AutoDScalar ff1 = (V - 0.5*(V1+V2));
      row_index.push_back(fvm_node->global_offset()+0);
      cols_index.push_back(cols);
      ad_values.push_back(ff1);

      // the electron density is interpolated by S-G scheme
      AutoDScalar ff2 = (n - nmid_dd(Vt, V1, V2, n1, n2));
      //AutoDScalar ff2 = V1>V2 ? n-n2 : n-n1;
      row_index.push_back(fvm_node->global_offset()+1);
      cols_index.push_back(cols);
      ad_values.push_back(ff2);


      //src_row.push_back(fvm_node->global_offset()+1);
      //dst_row.push_back( V1 > V2 ? interpolation_p1->global_offset()+1 : interpolation_p2->global_offset()+1);
      //alpha_buffer.push_back(1.0);


      // the hole density is interpolated by S-G scheme
      AutoDScalar ff3 = (p - pmid_dd(Vt, V1, V2, p1, p2));
      //AutoDScalar ff3 = V1>V2 ? p-p1 : p-p2;
      row_index.push_back(fvm_node->global_offset()+2);
      cols_index.push_back(cols);
      ad_values.push_back(ff3);

      //src_row.push_back(fvm_node->global_offset()+2);
      //dst_row.push_back(V1 > V2 ? interpolation_p2->global_offset()+2 : interpolation_p1->global_offset()+2);
      //alpha_buffer.push_back(1.0);
    }


    //ok, we add source rows to destination rows
    PetscUtils::MatAddRowToRow(*jac, src_row, dst_row, alpha_buffer);

    // clear row_index
    PetscUtils::MatZeroRows(*jac, row_index.size(), row_index.empty() ? NULL : &row_index[0], 0.0);


    // insert buffered AD values to Mat
    for(unsigned int n=0; n< row_index.size(); ++n )
      MatSetValues(*jac, 1, &row_index[n], (cols_index[n]).size(), &((cols_index[n])[0]), ad_values[n].getADValue(), INSERT_VALUES);

    add_value_flag = INSERT_VALUES;

    return;

  }

  // process 3D hanging node
  if( this->has_3d_hanging_node() )
  {
    // process hanging node lies on side center
    {

      // buffer for add matrix row to row
      std::vector<PetscInt>    src_row;
      std::vector<PetscInt>    dst_row;
      std::vector<PetscScalar> alpha_buffer;

      // buffer for matrix entrance
      std::vector< PetscInt >                 row_index;
      std::vector< std::vector<PetscInt> >    cols_index;
      std::vector< AutoDScalar >              ad_values;

      hanging_node_on_elem_side_iterator  hanging_node_it = hanging_node_on_elem_side_begin();
      hanging_node_on_elem_side_iterator  hanging_node_it_end = hanging_node_on_elem_side_end();

      for(; hanging_node_it!=hanging_node_it_end; ++hanging_node_it )
      {
        const FVM_Node * fvm_node = (*hanging_node_it).first;

        // skip node not belongs to this processor
        if( fvm_node->root_node()->processor_id()!=Genius::processor_id() ) continue;

        // let the flux of hanging node flow into other "non-hanging" node on the element side averagely
        // this process ensure the global conservation of flux

        const Elem * elem = (*hanging_node_it).second.first;
        unsigned int side_index = (*hanging_node_it).second.second;

        AutoPtr<Elem>  side = elem->build_side(side_index);

        unsigned int n_side_node = side->n_nodes();
        std::vector<const FVM_Node *> side_fvm_nodes;

        for(unsigned int n=0; n < n_side_node; n++)
        {
          const Node * side_node = side->get_node(n);
          const FVM_Node * side_fvm_node = region_fvm_node(side_node);
          side_fvm_nodes.push_back(side_fvm_node);

          src_row.push_back(fvm_node->global_offset()+0);
          src_row.push_back(fvm_node->global_offset()+1);
          src_row.push_back(fvm_node->global_offset()+2);

          dst_row.push_back(side_fvm_node->global_offset()+0);
          dst_row.push_back(side_fvm_node->global_offset()+1);
          dst_row.push_back(side_fvm_node->global_offset()+2);

          alpha_buffer.push_back(1.0/n_side_node);
          alpha_buffer.push_back(1.0/n_side_node);
          alpha_buffer.push_back(1.0/n_side_node);
        }

        // and then, the value of hanging node is interpolated.
        //the indepedent variable number, we need 9 here
        adtl::AutoDScalar::numdir = 9;

        const FVM_Node * interpolation_p1;
        const FVM_Node * interpolation_p2;

        // find the interpolation point
        genius_assert(side_fvm_nodes.size() == 4);
        {
          // for 3D case, the side should be QUAD4, we use 2 point which has little psi difference as interpolation point
          PetscScalar dv1 = std::abs( x[side_fvm_nodes[0]->local_offset()] - x[side_fvm_nodes[2]->local_offset()] );
          PetscScalar dv2 = std::abs( x[side_fvm_nodes[1]->local_offset()] - x[side_fvm_nodes[3]->local_offset()] );

          if( dv1 < dv2 )
          {
            interpolation_p1 = side_fvm_nodes[0];
            interpolation_p2 = side_fvm_nodes[2];
          }
          else
          {
            interpolation_p1 = side_fvm_nodes[1];
            interpolation_p2 = side_fvm_nodes[3];
          }
        }

        AutoDScalar V = x[fvm_node->local_offset()+0]; V.setADValue(0, 1.0);
        AutoDScalar n = x[fvm_node->local_offset()+1]; n.setADValue(1, 1.0);
        AutoDScalar p = x[fvm_node->local_offset()+2]; p.setADValue(2, 1.0);

        AutoDScalar V1 = x[interpolation_p1->local_offset()+0]; V1.setADValue(3, 1.0);
        AutoDScalar n1 = x[interpolation_p1->local_offset()+1]; n1.setADValue(4, 1.0);
        AutoDScalar p1 = x[interpolation_p1->local_offset()+2]; p1.setADValue(5, 1.0);

        AutoDScalar V2 = x[interpolation_p2->local_offset()+0]; V2.setADValue(6, 1.0);
        AutoDScalar n2 = x[interpolation_p2->local_offset()+1]; n2.setADValue(7, 1.0);
        AutoDScalar p2 = x[interpolation_p2->local_offset()+2]; p2.setADValue(8, 1.0);

        std::vector<PetscInt> cols(9);
        cols[0] = fvm_node->global_offset()+0;
        cols[1] = fvm_node->global_offset()+1;
        cols[2] = fvm_node->global_offset()+2;
        cols[3] = interpolation_p1->global_offset()+0;
        cols[4] = interpolation_p1->global_offset()+1;
        cols[5] = interpolation_p1->global_offset()+2;
        cols[6] = interpolation_p2->global_offset()+0;
        cols[7] = interpolation_p2->global_offset()+1;
        cols[8] = interpolation_p2->global_offset()+2;

        // the psi is linear interpolated
        AutoDScalar ff1 = (V - 0.5*(V1+V2));
        row_index.push_back(fvm_node->global_offset()+0);
        cols_index.push_back(cols);
        ad_values.push_back(ff1);

        // the electron density is interpolated by S-G scheme
        AutoDScalar ff2 = (n - nmid_dd(Vt, V1, V2, n1, n2));
        row_index.push_back(fvm_node->global_offset()+1);
        cols_index.push_back(cols);
        ad_values.push_back(ff2);



        // the hole density is interpolated by S-G scheme
        AutoDScalar ff3 = (p - pmid_dd(Vt, V1, V2, p1, p2));
        row_index.push_back(fvm_node->global_offset()+2);
        cols_index.push_back(cols);
        ad_values.push_back(ff3);
      }


      //ok, we add source rows to destination rows
      PetscUtils::MatAddRowToRow(*jac, src_row, dst_row, alpha_buffer);

      // clear row_index
      PetscUtils::MatZeroRows(*jac, row_index.size(), row_index.empty() ? NULL : &row_index[0], 0.0);


      // insert buffered AD values to Mat
      for(unsigned int n=0; n< row_index.size(); ++n )
        MatSetValues(*jac, 1, &row_index[n], (cols_index[n]).size(), &((cols_index[n])[0]), ad_values[n].getADValue(), INSERT_VALUES);

    }


    // process hanging node lies on edge center
    {

      // buffer for add matrix row to row
      std::vector<PetscInt>    src_row;
      std::vector<PetscInt>    dst_row;
      std::vector<PetscScalar> alpha_buffer;

      // buffer for matrix entrance
      std::vector< PetscInt >                 row_index;
      std::vector< std::vector<PetscInt> >    cols_index;
      std::vector< AutoDScalar >              ad_values;

      hanging_node_on_elem_edge_iterator  hanging_node_it = hanging_node_on_elem_edge_begin();
      hanging_node_on_elem_edge_iterator  hanging_node_it_end = hanging_node_on_elem_edge_end();

      for(; hanging_node_it!=hanging_node_it_end; ++hanging_node_it )
      {
        const FVM_Node * fvm_node = (*hanging_node_it).first;

        // skip node not belongs to this processor
        if( fvm_node->root_node()->processor_id()!=Genius::processor_id() ) continue;

        // let the flux of hanging node flow into other "non-hanging" node on the element side averagely
        // this process ensure the global conservation of flux

        const Elem * elem = (*hanging_node_it).second.first;
        unsigned int edge_index = (*hanging_node_it).second.second;

        AutoPtr<Elem>  edge = elem->build_edge(edge_index);

        unsigned int n_edge_node = 2;
        std::vector<const FVM_Node *> edge_fvm_nodes;

        for(unsigned int n=0; n < n_edge_node; n++)
        {
          const Node * edge_node = edge->get_node(n);
          const FVM_Node * edge_fvm_node = region_fvm_node(edge_node);
          edge_fvm_nodes.push_back(edge_fvm_node);

          src_row.push_back(fvm_node->global_offset()+0);
          src_row.push_back(fvm_node->global_offset()+1);
          src_row.push_back(fvm_node->global_offset()+2);

          dst_row.push_back(edge_fvm_node->global_offset()+0);
          dst_row.push_back(edge_fvm_node->global_offset()+1);
          dst_row.push_back(edge_fvm_node->global_offset()+2);

          alpha_buffer.push_back(1.0/n_edge_node);
          alpha_buffer.push_back(1.0/n_edge_node);
          alpha_buffer.push_back(1.0/n_edge_node);
        }

        // and then, the value of hanging node is interpolated.

        // the indepedent variable number, we need 9 here
        adtl::AutoDScalar::numdir = 9;

        AutoDScalar V = x[fvm_node->local_offset()+0]; V.setADValue(0, 1.0);
        AutoDScalar n = x[fvm_node->local_offset()+1]; n.setADValue(1, 1.0);
        AutoDScalar p = x[fvm_node->local_offset()+2]; p.setADValue(2, 1.0);

        AutoDScalar V1 = x[edge_fvm_nodes[0]->local_offset()+0]; V1.setADValue(3, 1.0);
        AutoDScalar n1 = x[edge_fvm_nodes[0]->local_offset()+1]; n1.setADValue(4, 1.0);
        AutoDScalar p1 = x[edge_fvm_nodes[0]->local_offset()+2]; p1.setADValue(5, 1.0);

        AutoDScalar V2 = x[edge_fvm_nodes[1]->local_offset()+0]; V2.setADValue(6, 1.0);
        AutoDScalar n2 = x[edge_fvm_nodes[1]->local_offset()+1]; n2.setADValue(7, 1.0);
        AutoDScalar p2 = x[edge_fvm_nodes[1]->local_offset()+2]; p2.setADValue(8, 1.0);

        std::vector<PetscInt> cols(9);
        cols[0] = fvm_node->global_offset()+0;
        cols[1] = fvm_node->global_offset()+1;
        cols[2] = fvm_node->global_offset()+2;
        cols[3] = edge_fvm_nodes[0]->global_offset()+0;
        cols[4] = edge_fvm_nodes[0]->global_offset()+1;
        cols[5] = edge_fvm_nodes[0]->global_offset()+2;
        cols[6] = edge_fvm_nodes[1]->global_offset()+0;
        cols[7] = edge_fvm_nodes[1]->global_offset()+1;
        cols[8] = edge_fvm_nodes[1]->global_offset()+2;

        // the psi is linear interpolated
        AutoDScalar ff1 = V - 0.5*(V1+V2);
        row_index.push_back(fvm_node->global_offset()+0);
        cols_index.push_back(cols);
        ad_values.push_back(ff1);

        // the electron density is interpolated by S-G scheme
        AutoDScalar ff2 = n - nmid_dd(Vt, V1, V2, n1, n2);
        row_index.push_back(fvm_node->global_offset()+1);
        cols_index.push_back(cols);
        ad_values.push_back(ff2);

        // the hole density is interpolated by S-G scheme
        AutoDScalar ff3 = p - pmid_dd(Vt, V1, V2, p1, p2);
        row_index.push_back(fvm_node->global_offset()+2);
        cols_index.push_back(cols);
        ad_values.push_back(ff3);
      }


      //ok, we add source rows to destination rows
      PetscUtils::MatAddRowToRow(*jac, src_row, dst_row, alpha_buffer);

      // clear row_index
      PetscUtils::MatZeroRows(*jac, row_index.size(), row_index.empty() ? NULL : &row_index[0], 0.0);


      // insert buffered AD values to Mat
      for(unsigned int n=0; n< row_index.size(); ++n )
        MatSetValues(*jac, 1, &row_index[n], (cols_index[n]).size(), &((cols_index[n])[0]), ad_values[n].getADValue(), INSERT_VALUES);

    }

    add_value_flag = INSERT_VALUES;
    return;
  }
}





