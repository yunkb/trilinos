// ////////////////////////////////////////////////////////////////////////////
// EvalNewPointTailoredApproach_Step.h
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

#ifndef EVAL_NEW_POINT_TAILORED_APPROACH_STEP_H
#define EVAL_NEW_POINT_TAILORED_APPROACH_STEP_H

#include "ReducedSpaceSQPPack/include/ReducedSpaceSQPPackTypes.h"
#include "GeneralIterationPack/include/AlgorithmStep.h"
#include "ConstrainedOptimizationPack/include/VariableBoundsTester.h"
#include "NLPInterfacePack/test/NLPFirstOrderDirectTester.h"
#include "StandardCompositionMacros.h"
#include "StandardMemberCompositionMacros.h"

namespace ReducedSpaceSQPPack {

///
/** Base class for evaluating a new point for the "Tailored Approach".
 *
 * Uses the \c NLPFirstOrderDirect interface to compute <tt>Z = [ -inv(C)*N; I ]</tt>
 * and <tt>py = -inv(C)*c(decomp)</tt> explicitly.  Subclasses determine how
 * <tt>py</tt> and <tt>Y</tt> are updated.
 */
class EvalNewPointTailoredApproach_Step
	: public GeneralIterationPack::AlgorithmStep // doxygen needs full path
{
public:

	///
	typedef ReferenceCountingPack::ref_count_ptr<const MatrixWithOp> D_ptr_t;
	///
	typedef NLPInterfacePack::NLPFirstOrderDirectTester    NLPFirstOrderDirectTester;
	/// <<std comp>> members for testing object for NLPFirstOrderDirect
	STANDARD_COMPOSITION_MEMBERS( NLPFirstOrderDirectTester, deriv_tester )
	/// <<std comp>> Members for variable bounds tester object
	STANDARD_COMPOSITION_MEMBERS( VariableBoundsTester, bounds_tester )
	///
	enum EFDDerivTesting { FD_DEFAULT, FD_TEST, FD_NO_TEST };
	///
	/** Set how and if finite derivatives are tested.
	 *
	 * ToDo: Finish documentation.
	 */
	STANDARD_MEMBER_COMPOSITION_MEMBERS( EFDDerivTesting, fd_deriv_testing )

	///
	EvalNewPointTailoredApproach_Step(
		  const deriv_tester_ptr_t& 	deriv_tester
		, const bounds_tester_ptr_t&	bounds_tester
		, EFDDerivTesting				fd_deriv_testing = FD_DEFAULT
		);

	/** @name Overridden from AlgorithmStep */
	//@{

	///
	bool do_step(Algorithm& algo, poss_type step_poss, GeneralIterationPack::EDoStepType type
		, poss_type assoc_step_poss);
	///
	void print_step( const Algorithm& algo, poss_type step_poss, GeneralIterationPack::EDoStepType type
		, poss_type assoc_step_poss, std::ostream& out, const std::string& leading_str ) const;
	
	//@}

	/** @name To be overridden by subclasses */
	//@{

	///
	/** Overridden by subclass to compute \c py and \c Ypy.
	 *
	 * The matrix Y is never computed explicity.
	 *
	 * @param  D    [in/out] Smart pointer to matrix <tt>D = -inv(C)*N</tt>.
	 *              On output, D->count() may be incremented in order to
	 *              initialize <tt>Y</tt>.
	 * @param  py   [in/out] On input <tt>py = -inv(C)*c(decomp)</tt>.
	 *              On output <tt>py = -inv((Gc(decomp)'*Y)*c(decomp)</tt>
	 * @param  Y    [in/out] On ouput <tt>Y</tt> is initialized properly.
	 */
	virtual void calc_py_Y(
		const NLPFirstOrderDirect   &nlp
		,const D_ptr_t              &D
		,VectorWithOpMutable        *py
		,MatrixIdentConcatStd       *Y
		,EJournalOutputLevel        olevel
		,std::ostream               &out
		) = 0;

	///
	/** Overridden by subclass to recompute \c py and \c Ypy.
	 *
	 * @param  D    [in] matrix <tt>D = -inv(C)*N</tt>
	 * @param  py   [in/out] On input <tt>py = -inv(C)*c(decomp)</tt>.
	 *              On output <tt>py = -inv((Gc(decomp)'*Y)*c(decomp)</tt>
	 */
	virtual void recalc_py(
		const MatrixWithOp       &D
		,VectorWithOpMutable     *py
		,EJournalOutputLevel     olevel
		,std::ostream            &out
		) = 0;
	
	///
	/** Overridden by subclass to print how \c py and \c Y are computed.
	 */
	virtual void print_calc_py_Y(
		std::ostream& out, const std::string& leading_str
		) const = 0;

	//@}

private:

	// Not defined and not to be called
	EvalNewPointTailoredApproach_Step();

};	// end class EvalNewPointTailoredApproach_Step

}	// end namespace ReducedSpaceSQPPack 

#endif	// EVAL_NEW_POINT_TAILORED_APPROACH_STEP_H
