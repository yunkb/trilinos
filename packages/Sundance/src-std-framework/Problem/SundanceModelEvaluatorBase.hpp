/* @HEADER@ */
// ************************************************************************
// 
//                              Sundance
//                 Copyright (2005) Sandia Corporation
// 
// Copyright (year first published) Sandia Corporation.  Under the terms 
// of Contract DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government 
// retains certain rights in this software.
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
// Questions? Contact Kevin Long (krlong@sandia.gov), 
// Sandia National Laboratories, Livermore, California, USA
// 
// ************************************************************************
/* @HEADER@ */

#ifndef SUNDANCE_SUNDANCEMODELEVALUATORBASE_H
#define SUNDANCE_SUNDANCEMODELEVALUATORBASE_H

#include "SundanceDefs.hpp"

#ifdef HAVE_ENABLED_MOOCHO

#include "Sundance.hpp"
#include "Thyra_ModelEvaluator.hpp"

namespace Thyra
{
  using namespace SundanceUtils;
  using namespace SundanceStdMesh;
  using namespace SundanceStdMesh::Internal;
  using namespace SundanceCore;
  using namespace SundanceCore::Internal;
  using namespace Teuchos;

  /** 
   * 
   */
  class SundanceModelEvaluator : public ModelEvaluator<double>
  {
  public:
    /** */
    SundanceModelEvaluator(const VectorType<double>& vecType);

    /** */
    virtual ~SundanceModelEvaluator() {;}

    /** Get the space of state variables */
    RefCountPtr<const VectorSpaceBase<double> > get_x_space() const 
    {return stateSpace().ptr();}

    /** Get the range space of the constraints */
    RefCountPtr<const VectorSpaceBase<double> > get_f_space() const 
    {return constraintSpace().ptr();}

    /** Get the space of model parameters */
    RefCountPtr<const VectorSpaceBase<double> > get_p_space(int i) const 
    {
      TEST_FOR_EXCEPTION(i != 0, RuntimeError, "invalid index for parameter space");
      return paramSpace().ptr();
    }

    /** Get the range space of the objective function */
    RefCountPtr<const VectorSpaceBase<double> > get_g_space(int i) const 
    {
      TEST_FOR_EXCEPTION(i != 0, RuntimeError, "invalid index for objective space");
      return objectiveSpace_.ptr();
    }

    /** Get an initial guess for the parameters */
    RefCountPtr<const VectorBase<double> > get_p_init(int l) const ;

    /** Get an initial guess for the state variables */
    RefCountPtr<const VectorBase<double> > get_x_init() const ;

    /** Create an object for df/dx */
    RefCountPtr<LinearOpBase<double> > create_W_op() const 
    {return createW().ptr();}

    /** create an object for the derivatives of the constraint wrt the parameters  */
    ModelEvaluatorBase::DerivativeMultiVector<double> 
    create_DfDp_mv(int l, 
                   EDerivativeMultiVectorOrientation orientation) const;

    /** create an object for the derivatives of the objective wrt the state  */
    ModelEvaluatorBase::DerivativeMultiVector<double> 
    create_DgDx_mv(int l, 
                   EDerivativeMultiVectorOrientation orientation) const;

    /** create an object for the derivatives of the objective wrt the parameters  */
    ModelEvaluatorBase::DerivativeMultiVector<double> 
    create_DgDp_mv(int j, int l,
                   EDerivativeMultiVectorOrientation orientation) const;

    

    /** Create a container for the input arguments */
    InArgs<double> createInArgs() const ;

    /** Create a container for the output arguments */
    OutArgs<double> createOutArgs() const ;

    /** Wrapper to convert the Thyra arguments to/from Sundance arguments */
    void evalModel(const InArgs<double>& inArgs,
                   const OutArgs<double>& outArgs) const ;


    /** Run the model  */
    virtual void internalEvalModel(const Vector<double>& stateVec,
                                   const Vector<double>& params,
                                   Vector<double>& resid,
                                   double& objFuncVal,
                                   LinearOperator<double>& df_dx,
                                   Array<Vector<double> >& df_dp,
                                   Vector<double>& dg_dp_T,
                                   Vector<double>& dg_dx_T) const = 0 ;
                                   
    /** */
    virtual VectorSpace<double> paramSpace() const = 0;        
           
    /** */
    virtual VectorSpace<double> stateSpace() const = 0;   
                                   
    /** */
    virtual Vector<double> getInitialState() const = 0;        
           
    /** */
    virtual Vector<double> getInitialParameters() const = 0;   
           
    /** */
    virtual VectorSpace<double> constraintSpace() const = 0;
           
    /** */
    virtual VectorSpace<double> objectiveSpace() const {return objectiveSpace_;}

    /** */
    virtual LinearOperator<double> createW() const = 0;

    /** */
    const VectorType<double>& vecType() const {return vecType_;}

  private:
    VectorType<double> vecType_;
    VectorSpace<double> objectiveSpace_;

  };
    
}


#endif

#endif
