/*
// @HEADER
// ***********************************************************************
// 
// RTOp: Interfaces and Support Software for Vector Reduction Transformation
//       Operations
//                Copyright (2006) Sandia Corporation
// 
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact Roscoe A. Bartlett (rabartl@sandia.gov) 
// 
// ***********************************************************************
// @HEADER
*/


#include "RTOpPack_Types.hpp"

#include "Teuchos_UnitTestHarness.hpp"

#include "RTOpPack_SPMD_apply_op.hpp"
#include "RTOpPack_ROpSum.hpp"
#include "Teuchos_DefaultComm.hpp"
#include "Teuchos_CommHelpers.hpp"
#include "Teuchos_as.hpp"


namespace RTOpPack {


using Teuchos::as;
using Teuchos::inoutArg;


TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL( SPMD_apply_op, args_1_0_reduce, Scalar )
{

  const RCP<const Teuchos::Comm<Ordinal> > comm =
    Teuchos::DefaultComm<Ordinal>::getComm();

  //const int procRank = rank(*comm);

  const Ordinal localDim = 2; // ToDo: Make a commandline argument!

  Ordinal localOffset = 0;
  Teuchos::scan<Ordinal>( *comm, Teuchos::REDUCE_SUM, localDim, inoutArg(localOffset) );
  localOffset -= localDim;
  
  Teuchos::Array<Scalar> x_dat(localDim);
  std::fill_n(&x_dat[0], localDim, as<Scalar>(1.0));
  RTOpPack::SubVectorView<Scalar> x(
    localOffset, localDim, Teuchos::arcpFromArray(x_dat), 1);

  RTOpPack::ROpSum<Scalar> sumOp;
  RCP<RTOpPack::ReductTarget> sumTarget = sumOp.reduct_obj_create();
  RTOpPack::SPMD_apply_op<Scalar>(&*comm, sumOp, 1, &x, 0, 0, &*sumTarget);
  Scalar sum_x = sumOp(*sumTarget);

  TEST_EQUALITY(sum_x, as<Scalar>(localDim * comm->getSize()))

}

TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT_SCALAR_TYPES(
  SPMD_apply_op, args_1_0_reduce)


} // namespace RTOpPack
