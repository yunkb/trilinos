/*------------------------------------------------------------------------*/
/*      phdMesh : Parallel Heterogneous Dynamic unstructured Mesh         */
/*                Copyright (2007) Sandia Corporation                     */
/*                                                                        */
/*  Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive   */
/*  license for use of this work by or on behalf of the U.S. Government.  */
/*                                                                        */
/*  This library is free software; you can redistribute it and/or modify  */
/*  it under the terms of the GNU Lesser General Public License as        */
/*  published by the Free Software Foundation; either version 2.1 of the  */
/*  License, or (at your option) any later version.                       */
/*                                                                        */
/*  This library is distributed in the hope that it will be useful,       */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of        */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     */
/*  Lesser General Public License for more details.                       */
/*                                                                        */
/*  You should have received a copy of the GNU Lesser General Public      */
/*  License along with this library; if not, write to the Free Software   */
/*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307   */
/*  USA                                                                   */
/*------------------------------------------------------------------------*/
/**
 * @author H. Carter Edwards
 */

#ifndef phdmesh_Schema_hpp
#define phdmesh_Schema_hpp

//----------------------------------------------------------------------

#include <mesh/Types.hpp>
#include <mesh/Part.hpp>
#include <mesh/Field.hpp>
#include <util/Parallel.hpp>

namespace phdmesh {

//----------------------------------------------------------------------
/** Description of a Parallel Heterogeneous Dynamic Mesh.
 *  A collection of parts, fields, part relations, and field relations.
 */
class Schema {
public:

  //------------------------------------
  // Predefined parts:

  /** Universal: superset of all other parts. */
  Part & universal_part() const { return const_cast<Part&>(m_universal_part); }

  /** Uses: used by the local processor, subset of 'universal_part'.
   *  The aura is a subset of 'universal_part \ uses_part'.
   */
  Part & uses_part() const { return *m_uses_part ; }

  /** Owned: owned by the local processor, subset of 'uses_part'.
   *  Each mesh entity is owned by exactly one processor.
   */
  Part & owns_part()  const { return *m_owns_part ; }

  //------------------------------------
  /** Get an existing part of the given name and type.
   *  Return NULL if not present and required_by == NULL.
   *  If required and not present then throws an exception
   *  with the 'required_by' text.
   */
  Part * get_part( const std::string & ,
                   const char * required_by = NULL ) const ;

  /** Query all parts of the mesh.
   *  Once the mesh is commited the parts will know
   *  their ordinals within this vector.
   */
  const PartSet & get_parts() const { return m_universal_part.subsets(); }

  /** Declare a part of the given name.
   *  Redeclaration returns the previously declared part.
   */
  Part & declare_part( const std::string & );

  /** Declare a part that is defined as the
   *  intersection of the given part set.
   */
  Part & declare_part( const PartSet & );

  /** Declare a superset-subset relationship */
  void declare_part_subset( Part & superset , Part & subset );

  /** Declare a part relationship.
   *  Given Entity 'e1' and 'e2' with a relation 'e1-to-e2' that
   *        is in the domain of the relation stencil 'stencil'.
   *  If   'e1' is a member of 'root_part'
   *  then 'e2' is a member of 'target_part'.
   */
  void declare_part_relation( Part & root_part ,
                              relation_stencil_ptr stencil ,
                              Part & target_part );

  /** The collection of part relationships, used internally */
  const std::vector<PartRelation> & get_part_relations() const
    { return m_part_relations ; }

  /** Declare an attribute on a part */
  template<class T>
  CSet::Span<T> declare_part_attribute( Part & , const T * , bool );

  //------------------------------------
  /** Get a field, return NULL if it does not exist.
   *  An exception will be thrown
   *  if the field exits and the type or number of dimensions does not match.
   *  If required and not present then throws an exception
   *  with the 'required_by' text.
   */
  template< class field_type >
  field_type * get_field( const std::string & name ,
                          const char * required_by = NULL ) const ;

  /** Get all fields associated with the given entity type */
  const std::vector< FieldBase * > & get_fields() const { return m_fields ; }

  /** Declare a field within the mesh.
   *  Redeclaration with compatible parameters returns the
   *  previously declared field.
   *  Redeclaration with incompatible parameters throws an exception.
   */
  template< class field_type >
  field_type & declare_field( const std::string & name ,
                              unsigned number_of_states = 1 );

  /** Declare a scalar field to exist for a given entity type and Part. */
  template< class field_type >
  void declare_field_exists( field_type & arg_field ,
                             EntityType   arg_entity_type ,
                             const Part & arg_part );

  template< class field_type >
  void declare_field_size( field_type & arg_field ,
                           EntityType   arg_entity_type ,
                           const Part & arg_part ,
                           unsigned     arg_n1 );

  template< class field_type >
  void declare_field_size( field_type & arg_field ,
                           EntityType   arg_entity_type ,
                           const Part & arg_part ,
                           unsigned     arg_n1 ,
                           unsigned     arg_n2 );

  template< class field_type >
  void declare_field_size( field_type & arg_field ,
                           EntityType   arg_entity_type ,
                           const Part & arg_part ,
                           unsigned     arg_n1 ,
                           unsigned     arg_n2 ,
                           unsigned     arg_n3 );

  template< class field_type >
  void declare_field_size( field_type & arg_field ,
                           EntityType   arg_entity_type ,
                           const Part & arg_part ,
                           unsigned     arg_n1 ,
                           unsigned     arg_n2 ,
                           unsigned     arg_n3 ,
                           unsigned     arg_n4 );

  template< class field_type >
  void declare_field_size( field_type & arg_field ,
                           EntityType   arg_entity_type ,
                           const Part & arg_part ,
                           unsigned     arg_n1 ,
                           unsigned     arg_n2 ,
                           unsigned     arg_n3 ,
                           unsigned     arg_n4 ,
                           unsigned     arg_n5 );

  template< class field_type >
  void declare_field_size( field_type & arg_field ,
                           EntityType   arg_entity_type ,
                           const Part & arg_part ,
                           unsigned     arg_n1 ,
                           unsigned     arg_n2 ,
                           unsigned     arg_n3 ,
                           unsigned     arg_n4 ,
                           unsigned     arg_n5 ,
                           unsigned     arg_n6 );

  template< class field_type >
  void declare_field_size( field_type & arg_field ,
                           EntityType   arg_entity_type ,
                           const Part & arg_part ,
                           unsigned     arg_n1 ,
                           unsigned     arg_n2 ,
                           unsigned     arg_n3 ,
                           unsigned     arg_n4 ,
                           unsigned     arg_n5 ,
                           unsigned     arg_n6 ,
                           unsigned     arg_n7 );

  /** Declare a field relationship.
   *  The pointer_field's scalar type must be a pointer to the
   *  scalar type of the reference_field.  The following
   *  derived field data relationship maintained.
   *
   *  Let   e_root -> Relation( e_target , attribute )
   *  Let   i = stencil( e_root.entity_type() , attribute );
   *  Let   Scalar ** ptr = field_data( pointer_field , e_root )
   *  then  ptr[i] = field_data( referenced_field , e_target )
   *
   *  This derived field data relationship is typically used
   *  to support fast access to field data on entities
   *  related to the root entity; e.g. field data associated with
   *  the nodes of an element.
   */
  void declare_field_relation( FieldBase & pointer_field ,
                               relation_stencil_ptr stencil ,
                               FieldBase & referenced_field ); 

  const std::vector<FieldRelation> & get_field_relations() const
    { return m_field_relations ; }

  /** Declare an attribute on a field */
  template<class T>
  CSet::Span<T> declare_field_attribute( FieldBase & , const T * , bool );

  //------------------------------------
  /** Commit the part and field declarations.
   *  Verifies consistency and assigns ordinals for faster usage.
   *  No further declarations can be made.
   */
  void commit();

  //------------------------------------

  void assert_committed( const char * ) const ;

  void assert_not_committed( const char * ) const ;

  void assert_same_schema( const char * , const Schema & ) const ;

  bool is_commit() const { return m_commit ; }

  ~Schema();

  Schema();

private:

  Schema( const Schema & );
  Schema & operator = ( const Schema & );

  bool   m_commit ;
  Part   m_universal_part ; /* Subset list contains all other parts */
  Part * m_uses_part ;
  Part * m_owns_part ;

  std::vector< FieldBase * >   m_fields ;
  std::vector< PartRelation >  m_part_relations ;
  std::vector< FieldRelation > m_field_relations ;

  void declare_field_stride( FieldBase & ,
                             EntityType , const Part & ,
                             const unsigned * );
  
  FieldBase & declare_field_base( const std::string & ,
                                  unsigned arg_scalar_type ,
                                  const DimensionTag * ,
                                  const DimensionTag * ,
                                  const DimensionTag * ,
                                  const DimensionTag * ,
                                  const DimensionTag * ,
                                  const DimensionTag * ,
                                  const DimensionTag * ,
                                  unsigned arg_num_states );

  FieldBase * get_field_base( const std::string & ,
                              unsigned arg_scalar_type ,
                              const DimensionTag * ,
                              const DimensionTag * ,
                              const DimensionTag * ,
                              const DimensionTag * ,
                              const DimensionTag * ,
                              const DimensionTag * ,
                              const DimensionTag * ,
                              int arg_num_states ,
                              const char * required_by ) const ;

  void clean_field_dimension();
};

void verify_parallel_consistency( const Schema & , ParallelMachine );

} // namespace phdmesh

//----------------------------------------------------------------------
//----------------------------------------------------------------------

namespace phdmesh {

template< class field_type >
inline
field_type * Schema::get_field( const std::string & name ,
                                const char * required_by ) const
{
  typedef typename field_type::data_type Scalar ;
  typedef typename field_type::dimension_tag_1 Tag1 ;
  typedef typename field_type::dimension_tag_2 Tag2 ;
  typedef typename field_type::dimension_tag_3 Tag3 ;
  typedef typename field_type::dimension_tag_4 Tag4 ;
  typedef typename field_type::dimension_tag_5 Tag5 ;
  typedef typename field_type::dimension_tag_6 Tag6 ;
  typedef typename field_type::dimension_tag_7 Tag7 ;

  return static_cast< field_type * >(
    get_field_base( name ,
                    NumericEnum<Scalar>::value ,
                    Tag1::descriptor() ,
                    Tag2::descriptor() ,
                    Tag3::descriptor() ,
                    Tag4::descriptor() ,
                    Tag5::descriptor() ,
                    Tag6::descriptor() ,
                    Tag7::descriptor() ,
                    -1 , required_by ) );
}

template< class field_type >
inline
field_type & Schema::declare_field( const std::string & name ,
                                    unsigned number_of_states )
{
  typedef typename field_type::data_type Scalar ;
  typedef typename field_type::dimension_tag_1 Tag1 ;
  typedef typename field_type::dimension_tag_2 Tag2 ;
  typedef typename field_type::dimension_tag_3 Tag3 ;
  typedef typename field_type::dimension_tag_4 Tag4 ;
  typedef typename field_type::dimension_tag_5 Tag5 ;
  typedef typename field_type::dimension_tag_6 Tag6 ;
  typedef typename field_type::dimension_tag_7 Tag7 ;

  return static_cast< field_type & >(
    declare_field_base( name ,
                        NumericEnum<Scalar>::value ,
                        Tag1::descriptor() ,
                        Tag2::descriptor() ,
                        Tag3::descriptor() ,
                        Tag4::descriptor() ,
                        Tag5::descriptor() ,
                        Tag6::descriptor() ,
                        Tag7::descriptor() ,
                        number_of_states ) );
}

template< class field_type >
inline
void Schema::declare_field_exists(
  field_type & arg_field ,
  EntityType   arg_entity_type ,
  const Part & arg_part )
{
  declare_field_stride( arg_field , arg_entity_type , arg_part , NULL );
}

template< class field_type >
inline
void Schema::declare_field_size( field_type & arg_field ,
                                 EntityType   arg_entity_type ,
                                 const Part & arg_part ,
                                 unsigned     arg_n1 )
{
  const typename field_type::Dimension dim( arg_n1 );
  declare_field_stride( arg_field, arg_entity_type, arg_part, dim.stride );
}

template< class field_type >
inline
void Schema::declare_field_size( field_type & arg_field ,
                                 EntityType   arg_entity_type ,
                                 const Part & arg_part ,
                                 unsigned     arg_n1 ,
                                 unsigned     arg_n2 )
{
  const typename field_type::Dimension dim( arg_n1 , arg_n2 );
  declare_field_stride( arg_field, arg_entity_type, arg_part, dim.stride );
}

template< class field_type >
inline
void Schema::declare_field_size( field_type & arg_field ,
                                 EntityType   arg_entity_type ,
                                 const Part & arg_part ,
                                 unsigned     arg_n1 ,
                                 unsigned     arg_n2 ,
                                 unsigned     arg_n3 )
{
  const typename field_type::Dimension dim( arg_n1 , arg_n2 , arg_n3 );
  declare_field_stride( arg_field, arg_entity_type, arg_part, dim.stride );
}

template< class field_type >
inline
void Schema::declare_field_size( field_type & arg_field ,
                                 EntityType   arg_entity_type ,
                                 const Part & arg_part ,
                                 unsigned     arg_n1 ,
                                 unsigned     arg_n2 ,
                                 unsigned     arg_n3 ,
                                 unsigned     arg_n4 )
{
  const typename field_type::Dimension dim( arg_n1 , arg_n2 , arg_n3 , arg_n4 );
  declare_field_stride( arg_field, arg_entity_type, arg_part, dim.stride );
}

template< class field_type >
inline
void Schema::declare_field_size( field_type & arg_field ,
                                 EntityType   arg_entity_type ,
                                 const Part & arg_part ,
                                 unsigned     arg_n1 ,
                                 unsigned     arg_n2 ,
                                 unsigned     arg_n3 ,
                                 unsigned     arg_n4 ,
                                 unsigned     arg_n5 )
{
  const typename field_type::Dimension
    dim( arg_n1 , arg_n2 , arg_n3 , arg_n4 , arg_n5 );
  declare_field_stride( arg_field, arg_entity_type, arg_part, dim.stride );
}

template< class field_type >
inline
void Schema::declare_field_size( field_type & arg_field ,
                                 EntityType   arg_entity_type ,
                                 const Part & arg_part ,
                                 unsigned     arg_n1 ,
                                 unsigned     arg_n2 ,
                                 unsigned     arg_n3 ,
                                 unsigned     arg_n4 ,
                                 unsigned     arg_n5 ,
                                 unsigned     arg_n6 )
{
  const typename field_type::Dimension
    dim( arg_n1 , arg_n2 , arg_n3 , arg_n4 , arg_n5 , arg_n6 );
  declare_field_stride( arg_field, arg_entity_type, arg_part, dim.stride );
}

template< class field_type >
inline
void Schema::declare_field_size( field_type & arg_field ,
                                 EntityType   arg_entity_type ,
                                 const Part & arg_part ,
                                 unsigned     arg_n1 ,
                                 unsigned     arg_n2 ,
                                 unsigned     arg_n3 ,
                                 unsigned     arg_n4 ,
                                 unsigned     arg_n5 ,
                                 unsigned     arg_n6 ,
                                 unsigned     arg_n7 )
{
  const typename field_type::Dimension
    dim( arg_n1 , arg_n2 , arg_n3 , arg_n4 , arg_n5 , arg_n6 , arg_n7 );
  declare_field_stride( arg_field, arg_entity_type, arg_part, dim.stride );
}

template<class T>
inline
CSet::Span<T>
Schema::declare_part_attribute( Part & p , const T * a , bool d )
{
  assert_not_committed( "phdmesh::Schema::declare_part_attribute" );
  return p.m_cset.template insert<T>( a , d );
}

template<class T>
inline
CSet::Span<T>
Schema::declare_field_attribute( FieldBase & f , const T * a , bool d )
{
  assert_not_committed( "phdmesh::Schema::declare_field_attribute" );
  return f.m_cset.template insert<T>( a , d );
}

} // namespace phdmesh

//----------------------------------------------------------------------
//----------------------------------------------------------------------

#endif

