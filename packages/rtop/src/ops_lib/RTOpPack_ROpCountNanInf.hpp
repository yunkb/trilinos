// @HEADER
// ***********************************************************************
// 
//      Thyra: Interfaces and Support Code for the Interoperability of Abstract Numerical Algorithms 
//                 Copyright (2004) Sandia Corporation
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
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ***********************************************************************
// @HEADER

#ifndef RTOPPACK_ROP_COUNT_NAN_INF_HPP
#define RTOPPACK_ROP_COUNT_NAN_INF_HPP

#include "RTOpPack_RTOpTHelpers.hpp"

namespace RTOpPack {

/** \brief Reduction operator that counts the number of entries that are NaN or Inf.
 */
template<class Scalar>
class ROpCountNanInf : public ROpIndexReductionBase<Scalar> {
public:
  /** \brief . */
  ROpCountNanInf() : RTOpT<Scalar>("ROpCountNanInf") {}
  /** \brief . */
  index_type operator()(const ReductTarget& reduct_obj ) const { return this->getRawVal(reduct_obj); }
  /** @name Overridden from RTOpT */
  //@{
  /** \brief . */
  void apply_op(
    const int   num_vecs,       const ConstSubVectorView<Scalar>         sub_vecs[]
    ,const int  num_targ_vecs,  const SubVectorView<Scalar>  targ_sub_vecs[]
    ,ReductTarget *_reduct_obj
    ) const
    {
      using Teuchos::dyn_cast;
      ReductTargetScalar<index_type> &reduct_obj = dyn_cast<ReductTargetScalar<index_type> >(*_reduct_obj); 
      RTOP_APPLY_OP_1_0(num_vecs,sub_vecs,num_targ_vecs,targ_sub_vecs);
      index_type sum = reduct_obj.get();
      for( Teuchos_Index i = 0; i < subDim; ++i, v0_val += v0_s ) {
        sum += ( Teuchos::ScalarTraits<Scalar>::isnaninf(*v0_val) ? 1 : 0 );
      }
      reduct_obj.set(sum);
    }
  //@}
}; // class ROpCountNanInf

} // namespace RTOpPack

#endif // RTOPPACK_ROP_COUNT_NAN_INF_HPP
