// @HEADER
// ***********************************************************************
// 
// Moocho: Multi-functional Object-Oriented arCHitecture for Optimization
//                  Copyright (2003) Sandia Corporation
// 
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
// 
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//  
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//  
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Roscoe A. Bartlett (rabartl@sandia.gov) 
// 
// ***********************************************************************
// @HEADER

#include "AbstractLinAlgPack_VectorMutable.hpp"
#include "AbstractLinAlgPack_VectorMutableSubView.hpp"
#include "AbstractLinAlgPack_VectorSpace.hpp"
#include "RTOp_TOp_assign_scalar.h"
#include "RTOp_TOp_assign_vectors.h"
#include "RTOp_TOp_axpy.h"
#include "RTOp_TOp_set_sub_vector.h"
#include "RTOpPack_RTOpC.hpp"
#include "Teuchos_TestForException.hpp"

namespace {

// vector scalar assignment operator
static RTOpPack::RTOpC          assign_scalar_op;
// vector assignment operator
static RTOpPack::RTOpC          assign_vec_op;
// set element operator
static RTOpPack::RTOpC          set_ele_op;
// set sub-vector operator
static RTOpPack::RTOpC          set_sub_vector_op;
// axpy operator
static RTOpPack::RTOpC          axpy_op;

// Simple class for an object that will initialize the operator objects
class init_rtop_server_t {
public:
	init_rtop_server_t() {
		// Vector scalar assignment operator
		TEST_FOR_EXCEPT(0!=RTOp_TOp_assign_scalar_construct(0.0,&assign_scalar_op.op()));
		// Vector assignment operator
		TEST_FOR_EXCEPT(0!=RTOp_TOp_assign_vectors_construct(&assign_vec_op.op()));
		// Set sub-vector operator
		RTOp_SparseSubVector spc_sub_vec;
		RTOp_sparse_sub_vector_null(&spc_sub_vec);
		TEST_FOR_EXCEPT(0!=RTOp_TOp_set_sub_vector_construct(&spc_sub_vec,&set_sub_vector_op.op()));
		// axpy operator
		TEST_FOR_EXCEPT(0!=RTOp_TOp_axpy_construct(0.0,&axpy_op.op()));
	}
}; 

// When the program starts, this object will be created and the RTOp_Server object will
// be initialized before main() gets underway!
init_rtop_server_t  init_rtop_server;

} // end namespace

namespace AbstractLinAlgPack {

// VectorMutable

VectorMutable& VectorMutable::operator=(value_type alpha)
{
	TEST_FOR_EXCEPT(0!=RTOp_TOp_assign_scalar_set_alpha(alpha,&assign_scalar_op.op()));
	VectorMutable* targ_vecs[1] = { this };
	AbstractLinAlgPack::apply_op(assign_scalar_op,0,NULL,1,targ_vecs,NULL);
	return *this;
}

VectorMutable& VectorMutable::operator=(const Vector& vec)
{
	if( dynamic_cast<const void*>(&vec) == dynamic_cast<const void*>(this) )
		return *this; // Assignment to self!
	const Vector*   vecs[1]      = { &vec };
	VectorMutable*  targ_vecs[1] = { this };
	AbstractLinAlgPack::apply_op(assign_vec_op,1,vecs,1,targ_vecs,NULL);
	return *this;
}

VectorMutable& VectorMutable::operator=(const VectorMutable& vec)
{
	return this->operator=(static_cast<const Vector&>(vec));
}

void VectorMutable::set_ele( index_type i, value_type alpha )
{
	TEST_FOR_EXCEPT(0!=RTOp_TOp_assign_scalar_set_alpha(alpha,&assign_scalar_op.op()));
	VectorMutable* targ_vecs[1] = { this };
	AbstractLinAlgPack::apply_op(
		assign_scalar_op,0,NULL,1,targ_vecs,NULL
		,i,1,0 // first_ele, sub_dim, global_offset
		);
}

VectorMutable::vec_mut_ptr_t
VectorMutable::sub_view( const Range1D& rng_in )
{
	namespace rcp = MemMngPack;
	const index_type dim = this->dim();
	const Range1D    rng = rng_in.full_range() ? Range1D(1,dim) : rng_in;
#ifdef _DEBUG
	TEST_FOR_EXCEPTION(
		rng.ubound() > dim, std::out_of_range
		,"VectorMutable::sub_view(rng): Error, rng = ["<<rng.lbound()<<","<<rng.ubound()<<"] "
		"is not in the range [1,this->dim()] = [1,"<<dim<<"]" );
#endif	
	if( rng.lbound() == 1 && rng.ubound() == dim )
		return Teuchos::rcp( this, false );
	// We are returning a view that could change this vector so we had better
	// wipe out the cache
	//this->has_changed();  // I don't think this line is needed!
	return Teuchos::rcp(
		new VectorMutableSubView(
			Teuchos::rcp( this, false )
			,rng ) );
}

void VectorMutable::zero()
{
	this->operator=(0.0);
}

void VectorMutable::axpy( value_type alpha, const Vector& x )
{
	TEST_FOR_EXCEPT(0!=RTOp_TOp_axpy_set_alpha(alpha,&axpy_op.op()));
	const Vector*  vecs[1]      = { &x   };
	VectorMutable* targ_vecs[1] = { this };
	AbstractLinAlgPack::apply_op(axpy_op,1,vecs,1,targ_vecs,NULL);
}

void VectorMutable::get_sub_vector( const Range1D& rng, RTOpPack::MutableSubVector* sub_vec_inout )
{
	//
	// Here we get a copy of the data for the sub-vector that the
	// client will modify.  We must later commit these changes to the
	// actual vector when the client calls commitSubVector(...).
	// Note, this implementation is very dependent on the behavior of
	// the default implementation of constant version of
	// Vector<Scalar>::get_sub_vector(...) and the implementation of
	// Vector<Scalar>::set_sub_vector(...)!
	//
	RTOpPack::SubVector sub_vec;
	Vector::get_sub_vector( rng, &sub_vec );
	sub_vec_inout->initialize(
		sub_vec.globalOffset(),sub_vec.subDim(),const_cast<value_type*>(sub_vec.values()),sub_vec.stride());
}

void VectorMutable::commit_sub_vector( RTOpPack::MutableSubVector* sub_vec_inout )
{
	RTOpPack::SparseSubVector spc_sub_vec(
		sub_vec_inout->globalOffset(), sub_vec_inout->subDim()
		,sub_vec_inout->values(), sub_vec_inout->stride()
		);
	VectorMutable::set_sub_vector( spc_sub_vec );            // Commit the changes!
	RTOpPack::SubVector sub_vec(*sub_vec_inout);
	Vector::free_sub_vector( &sub_vec );                     // Free the memory!
	sub_vec_inout->set_uninitialized();                      // Make null as promised!
}

void VectorMutable::set_sub_vector( const RTOpPack::SparseSubVector& sub_vec )
{
	RTOp_SparseSubVector spc_sub_vec;
	if(sub_vec.indices()) {
		RTOp_sparse_sub_vector(
			sub_vec.globalOffset(), sub_vec.subDim(), sub_vec.subNz()
			,sub_vec.values(), sub_vec.valuesStride(), sub_vec.indices(), sub_vec.indicesStride()
			,sub_vec.localOffset(), sub_vec.isSorted()
			,&spc_sub_vec
			);
	}
	else {
		RTOp_SubVector _sub_vec;
		RTOp_sub_vector(
			sub_vec.globalOffset(), sub_vec.subDim(), sub_vec.values(), sub_vec.valuesStride()
			,&_sub_vec
			);
		RTOp_sparse_sub_vector_from_dense( &_sub_vec, &spc_sub_vec );
	}
	RTOpPack::RTOpC  set_sub_vector_op;
	TEST_FOR_EXCEPT(0!=RTOp_TOp_set_sub_vector_construct(&spc_sub_vec,&set_sub_vector_op.op()));
	VectorMutable* targ_vecs[1] = { this };
	AbstractLinAlgPack::apply_op(
		set_sub_vector_op,0,NULL,1,targ_vecs,NULL
		,sub_vec.globalOffset()+1,sub_vec.subDim(),sub_vec.globalOffset() // first_ele, sub_dim, global_offset
		);
}

void VectorMutable::Vp_StMtV(
	value_type                       alpha
	,const GenPermMatrixSlice        &P
	,BLAS_Cpp::Transp                P_trans
	,const Vector                    &x
	,value_type                      beta
	)
{
	TEST_FOR_EXCEPTION(
		true, std::logic_error
		,"VectorMutable::Vp_StMtV(...): Error, this function has not yet been "
		"given a default implementation and has not been overridden for the "
		"subclass \'" << typeid(*this).name() << "\'!"
		);
  // ToDo: Implement using reduction or transformation operators that will
	// work with any type of vector.
}

// Overridden from Vector

Vector::vec_ptr_t
VectorMutable::sub_view( const Range1D& rng ) const
{
	namespace rcp = MemMngPack;
	return const_cast<VectorMutable*>(this)->sub_view(rng);
}

} // end namespace AbstractLinAlgPack
