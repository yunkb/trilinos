// //////////////////////////////////////////
// ExampleNLPObjGrad.cpp
//
// Copyright (C) 2001 Roscoe Ainsworth Bartlett
//
// This is free software; you can redistribute it and/or modify it
// under the terms of the "Artistic License" (see the web site
//   http://www.opensource.org/licenses/artistic-license.html).
// This license is spelled out in the file COPYING.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// above mentioned "Artistic License" for more details.

#include <assert.h>

#include <stdexcept>
#include <limits>

#include "ExampleNLPObjGrad.hpp"
#include "ExampleNLPDirectRTOps.h"
#include "AbstractLinAlgPack/src/abstract/tools/BasisSystemComposite.hpp"
#include "AbstractLinAlgPack/src/abstract/interfaces/VectorMutable.hpp"
#include "AbstractLinAlgPack/src/abstract/interfaces/VectorStdOps.hpp"
#include "AbstractLinAlgPack/src/abstract/tools/VectorAuxiliaryOps.hpp"
#include "AbstractLinAlgPack/src/abstract/interfaces/LinAlgOpPack.hpp"
#include "RTOpPack/src/RTOpCppC.hpp"
#include "Range1D.hpp"
#include "dynamic_cast_verbose.hpp"
#include "ThrowException.hpp"
#include "AbstractFactoryStd.hpp"

namespace {

static RTOpPack::RTOpC          explnlp2_c_eval_op;

// Simple class for an object that will initialize the RTOp_Server.
class init_rtop_server_t {
public:
	init_rtop_server_t() {
		if(0!=RTOp_TOp_explnlp2_c_eval_construct( &explnlp2_c_eval_op.op() ))
			assert(0);
	}
}; 

// When the program starts, this object will be created and the RTOp_Server object will
// be initialized before main() gets underway!
init_rtop_server_t  init_rtop_server;

} // end namespace

namespace NLPInterfacePack {

ExampleNLPObjGrad::ExampleNLPObjGrad(
	const VectorSpace::space_ptr_t&  vec_space
	,value_type                      xo
	,bool                            has_bounds
	,bool                            dep_bounded
	)
	:vec_space_(vec_space), vec_space_comp_(MemMngPack::null)
	,initialized_(false), obj_scale_(1.0)
	,has_bounds_(has_bounds), force_xinit_in_bounds_(true), n_(2*vec_space->dim())
{
	namespace rcp = MemMngPack;

	// Assert the size of the NLP
	THROW_EXCEPTION(
		vec_space->dim() <= 0, std::logic_error
		,"ExampleNLPObjGrad::ExampleNLPObjGrad(...) Error!" );

	// Setup the aggregate vector space object
	BasisSystemComposite::initialize_space_x(
		vec_space, vec_space, &var_dep_, &var_indep_, &vec_space_comp_ );

	// Set the initial starting point.
	xinit_ = vec_space_comp_->create_member();
	*xinit_ = xo;

	// Setup the sparse bounds
	//
	// xl(i) = 0.01  \ 
	//                }  for i <: bounded_rng
	// xu(i) = 20    /

	xl_ = vec_space_comp_->create_member();
	xu_ = vec_space_comp_->create_member();

	if(has_bounds) {
		const Range1D
			bounded_rng   = ( dep_bounded ? var_dep_   : var_indep_ ),
			unbounded_rng = ( dep_bounded ? var_indep_ : var_dep_   );
		*xl_->sub_view(bounded_rng)   = 0.01;
		*xl_->sub_view(unbounded_rng) = -NLP::infinite_bound();
		*xu_->sub_view(bounded_rng)   = 20.0;
		*xu_->sub_view(unbounded_rng) = +NLP::infinite_bound();
	}
	else {
		*xl_ = -NLP::infinite_bound();
		*xu_ = +NLP::infinite_bound();
	}
}

// Overridden public members from NLP

void ExampleNLPObjGrad::initialize(bool test_setup)
{
	if( initialized_ ) {
		NLPObjGrad::initialize(test_setup);
		return;
	}

	AbstractLinAlgPack::force_in_bounds( *xl_, *xu_, xinit_.get() );

	NLPObjGrad::initialize(test_setup);

	initialized_ = true;
}

bool ExampleNLPObjGrad::is_initialized() const
{
	return initialized_;
}

size_type ExampleNLPObjGrad::n() const
{
	assert_is_initialized();
	return n_;
}

size_type ExampleNLPObjGrad::m() const
{
	assert_is_initialized();
	return n_ / 2;
}

NLP::vec_space_ptr_t ExampleNLPObjGrad::space_x() const
{
	return vec_space_comp_;
}

NLP::vec_space_ptr_t ExampleNLPObjGrad::space_c() const
{
	return vec_space_;
}

size_type ExampleNLPObjGrad::num_bounded_x() const
{
	return has_bounds_ ? n_/2 : 0;
}

void ExampleNLPObjGrad::force_xinit_in_bounds(bool force_xinit_in_bounds)
{
	force_xinit_in_bounds_ = force_xinit_in_bounds;
}

bool ExampleNLPObjGrad::force_xinit_in_bounds() const
{
	return force_xinit_in_bounds_;
}

const Vector& ExampleNLPObjGrad::xinit() const
{
	assert_is_initialized();
	return *xinit_;
}

const Vector& ExampleNLPObjGrad::xl() const
{
	assert_is_initialized();
	return *xl_;
}

const Vector& ExampleNLPObjGrad::xu() const
{
	assert_is_initialized();
	return *xu_;
}

value_type ExampleNLPObjGrad::max_var_bounds_viol() const
{
	return std::numeric_limits<value_type>::max(); // No limits on the bounds
}

void ExampleNLPObjGrad::scale_f( value_type scale_f )
{
	assert_is_initialized();
	obj_scale_ = scale_f;
}

value_type ExampleNLPObjGrad::scale_f() const
{
	assert_is_initialized();
	return obj_scale_;
}

void ExampleNLPObjGrad::report_final_solution(
	const Vector&    x
	,const Vector*   lambda
	,const Vector*   lambdaI
	,const Vector*   nu
	,bool                  optimal
	) const
{
	assert_is_initialized();

	// Do what you want with the solution (or final values) here.
	// For this example we will just ignore it.
}

Range1D ExampleNLPObjGrad::var_dep() const
{
	return var_dep_;
}

Range1D ExampleNLPObjGrad::var_indep() const
{
	return var_indep_;
}

// Overridden protected members from NLP

void ExampleNLPObjGrad::imp_calc_f(const Vector& x, bool newx
	, const ZeroOrderInfo& zero_order_info) const
{
	using AbstractLinAlgPack::dot;
	assert_is_initialized();
	f(); // assert f is set
	THROW_EXCEPTION( n() != x.dim(), std::length_error, "ExampleNLPObjGrad::imp_calc_f(...)"  );
	// f(x) = (obj_scale/2) * sum( x(i)^2, for i = 1..n )
	*zero_order_info.f = obj_scale_ / 2.0 * dot(x,x);
}

void ExampleNLPObjGrad::imp_calc_c(const Vector& x, bool newx
	, const ZeroOrderInfo& zero_order_info) const
{
	assert_is_initialized();
	const size_type n = this->n();
	THROW_EXCEPTION( n != x.dim(), std::length_error, "ExampleNLPObjGrad::imp_calc_c(...)"  );

	// c(x)(j) = x(j) * (x(m+j) -1) - 10 * x(m+j) = 0, for j = 1...m

	Vector::vec_ptr_t
		xD= x.sub_view(var_dep()),
		xI = x.sub_view(var_indep());

	const Vector*  vecs[]      = { xD.get(), xI.get() };
	VectorMutable* targ_vecs[] = { zero_order_info.c };
	AbstractLinAlgPack::apply_op(explnlp2_c_eval_op,2,vecs,1,targ_vecs,RTOp_REDUCT_OBJ_NULL);

}

void ExampleNLPObjGrad::imp_calc_h(
	const Vector& x, bool newx, const ZeroOrderInfo& zero_order_info) const
{
	assert(0); // Should never be called!
}

// Overridden protected members from NLPFirstOrder

void ExampleNLPObjGrad::imp_calc_Gf(const Vector& x, bool newx
	, const ObjGradInfo& obj_grad_info) const
{
	assert_is_initialized();
	THROW_EXCEPTION( n() != x.dim(), std::length_error, "ExampleNLPObjGrad::imp_calc_Gf(...)"  );
	// Gf = obj_scale * x
	LinAlgOpPack::V_StV(obj_grad_info.Gf,obj_scale_,x);
}

}	// end namespace NLPInterfacePack
