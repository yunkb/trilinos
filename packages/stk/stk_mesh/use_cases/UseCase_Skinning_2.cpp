
#include <iostream>

#include <use_cases/UseCase_Skinning.hpp>

#include <Shards_BasicTopologies.hpp>

#include <stk_mesh/fixtures/HexFixture.hpp>

#include <stk_mesh/base/BulkModification.hpp>
#include <stk_mesh/base/MetaData.hpp>
#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/Entity.hpp>

#include <stk_mesh/base/GetEntities.hpp>
#include <stk_mesh/base/Selector.hpp>
#include <stk_mesh/base/GetBuckets.hpp>

#include <stk_mesh/fem/EntityRanks.hpp>
#include <stk_mesh/fem/TopologyHelpers.hpp>
#include <stk_mesh/fem/Stencils.hpp>

#include <stk_mesh/fem/SkinMesh.hpp>

#include <stk_util/parallel/ParallelReduce.hpp>

namespace {

  unsigned count_skin_entities( stk::mesh::BulkData & mesh, stk::mesh::Part & skin_part) {

    const unsigned mesh_rank = stk::mesh::Element;

    const stk::mesh::MetaData & meta = mesh.mesh_meta_data();

    stk::mesh::Selector select_skin = skin_part & meta.locally_owned_part()  ;

    const std::vector<stk::mesh::Bucket*>& buckets = mesh.buckets( mesh_rank -1);

    return count_selected_entities( select_skin, buckets);
  }


  void destroy_element_closure( stk::mesh::BulkData & mesh, stk::mesh::Entity * entity) {

    stk::mesh::PairIterRelation relations = entity->relations();
    stk::mesh::EntityRank rank = entity->entity_rank();

    for (; !relations.empty();) {
      --relations.second;
      stk::mesh::Entity * current_entity = (relations.second->entity());

      mesh.destroy_relation( *entity, *current_entity);

      if ( current_entity->relations(rank).empty()) {
        mesh.destroy_entity(current_entity);
      }
    }

    mesh.destroy_entity(entity);
  }

}

bool skinning_use_case_2(stk::ParallelMachine pm)
{

  const unsigned nx = 2 , ny = 1 , nz = 1 ;

  bool result = true;

  //TODO check the skin after each update to ensure that the appropriate
  //number of faces  and particles exist.
  try {
    stk::mesh::fixtures::HexFixture fixture( pm , nx , ny , nz );

    const unsigned p_rank = fixture.bulk_data.parallel_rank();

    stk::mesh::Part & skin_part = fixture.meta_data.declare_part("skin_part");

    stk::mesh::Part & shell_part = fixture.meta_data.declare_part("shell_part", stk::mesh::Element);
    stk::mesh::set_cell_topology<shards::ShellQuadrilateral<4> >(shell_part);

    fixture.meta_data.commit();

    fixture.generate_mesh();

    stk::mesh::Entity * elem_to_kill = fixture.elem( 0 , 0 , 0 );

    fixture.bulk_data.modification_begin();

    if ( elem_to_kill != NULL && p_rank == elem_to_kill->owner_rank() ) {
      //add shell between the two elements

      stk::mesh::EntityId elem_node[4] ;

      elem_node[0] = fixture.node_id( 1, 0, 0 );
      elem_node[1] = fixture.node_id( 1, 1, 0 );
      elem_node[2] = fixture.node_id( 1, 1, 1 );
      elem_node[3] = fixture.node_id( 1, 0, 1 );

      stk::mesh::EntityId elem_id = 3;

      stk::mesh::declare_element( fixture.bulk_data, shell_part, elem_id, elem_node);

    }
    fixture.bulk_data.modification_end();

    stk::mesh::skin_mesh(fixture.bulk_data, stk::mesh::Element, &skin_part);

    //----------------------------------------------------------------------
    //Actual usecase
    //----------------------------------------------------------------------

    {
      int num_skin_entities = count_skin_entities( fixture.bulk_data, skin_part);

      stk::all_reduce(pm, stk::ReduceSum<1>(&num_skin_entities));

      if ( num_skin_entities != 10 ) {
        result = false;
        std::cerr << std::endl << "incorrect number of entities in skin.  Expected 10, Found "
          << num_skin_entities << std::endl;
      }
    }

    fixture.bulk_data.modification_begin();

    if ( elem_to_kill != NULL && p_rank == elem_to_kill->owner_rank() ) {
      destroy_element_closure( fixture.bulk_data, elem_to_kill);
    }

    fixture.bulk_data.modification_end();

    stk::mesh::skin_mesh( fixture.bulk_data, stk::mesh::Element, &skin_part);

    {
      int num_skin_entities = count_skin_entities( fixture.bulk_data, skin_part);

      stk::all_reduce(pm, stk::ReduceSum<1>(&num_skin_entities));

      if ( num_skin_entities != 6 ) {
        result = false;
        std::cerr << std::endl << "incorrect number of entities in skin.  Expected 6, Found "
          << num_skin_entities << std::endl;
      }
    }

  }
  catch(std::exception & e) {
    std::cerr << std::endl << e.what() << std::endl;
    result = false;
  }

  return result;
}