// @HEADER
// ***********************************************************************
// 
//    Thyra: Interfaces and Support for Abstract Numerical Algorithms
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

#ifndef TSF_MULTI_VECTOR_COlS_DECL_HPP
#define TSF_MULTI_VECTOR_COlS_DECL_HPP

#include "Thyra_MultiVectorBase.hpp"
#include "Thyra_VectorSpaceBase.hpp"
#include "Thyra_VectorBase.hpp"

namespace Thyra {

/** \brief Default subclass for <tt>MultiVectorBase</tt> implemented using columns
 * of separate abstract vectors.
 *
 * This is a very bad implementation of a multi-vector but this will
 * work in situations where you need a multi-vector but some
 * underlying linear algebra library does not directly support them.
 *
 * This subclass can be used to represent a <tt>%MultiVectorBase</tt>
 * wrapper around a single <tt>VectorBase</tt> object so that a single
 * vector can be passed to a method that expects a <tt>%MultiVectorBase</tt>
 * object.
 */
template<class Scalar>
class MultiVectorCols : virtual public MultiVectorBase<Scalar> {
public:

  /** \brief . */
  using MultiVectorBase<Scalar>::col; // Inject *all* functions!

  /** \brief . */
  using MultiVectorBase<Scalar>::subView; // Inject *all* functions!

  /** @name Constructors/Initializers */
  //@{

  /** \brief Construct to initialized.
   *
   * Postconditions:<ul>
   * <tt> <tt>this->range().get() == NULL</tt>
   * <tt> <tt>this->domain().get() == NULL</tt>
   * </ul>
   */
  MultiVectorCols();

  /// Calls <tt>initialize()</tt>.
  MultiVectorCols(
    const Teuchos::RefCountPtr<VectorBase<Scalar> > &col_vec
    );

  /// Calls <tt>initialize()</tt>.
  MultiVectorCols(
    const Teuchos::RefCountPtr<const VectorSpaceBase<Scalar> >          &range
    ,const Teuchos::RefCountPtr<const VectorSpaceBase<Scalar> >         &domain
    ,const Teuchos::RefCountPtr<VectorBase<Scalar> >                    col_vecs[] = NULL
    );
  
  /** \brief Initialize given a single vector object.
   *
   * @param  col_vec  [in] A single column vector.  It is not allowed for
   *                  <tt>col_vecs==NULL</tt>.
   *
   * Preconditions:<ul>
   * <li> <tt>col_vec.get() != NULL</tt> (throw <tt>std::invalid_argument</tt>)
   * <li> <tt>col_vec->dim() > 0</tt> (throw <tt>std::invalid_argument</tt>)
   * </ul>
   *
   * Postconditions:<ul>
   * <tt> <tt>this->range().get() == col_vec.space().get()</tt>
   * <tt> <tt>this->domain()->dim() == 1</tt>
   * <li> <tt>this->col(1).get() == col_vec.get()</tt>
   * </ul>
   */
  void initialize(
    const Teuchos::RefCountPtr<VectorBase<Scalar> > &col_vec
    );

  /** \brief Initialize given the spaces for the columns and rows and possibly the column vectors.
   *
   * @param  range    [in] The space that the columns must lie in.  The underlying
   *                  vector space must not be changed while <tt>this</tt> is in use.
   * @param  domain   [in] The space that the rows must lie in.  The underlying
   *                  vector space must not be changed while <tt>this</tt> is in use.
   *                  What this argument really specifies is what vector type
   *                  will be compatible with the vectors that the client may
   *                  try to use to interact with the rows of this multivector.
   * @param  col_vecs [in] Array (size <tt>domain->dim()</tt>) of column
   *                  vectors to use for the columns of <tt>this</tt>.
   *                  It is allowed for <tt>col_vecs==NULL</tt> in which case
   *                  <tt>range->createMember()</tt> will be used to
   *                  create the columns of <tt>this</tt>.
   *
   * Preconditions:<ul>
   * <li> <tt>range.get() != NULL</tt> (throw <tt>std::invalid_argument</tt>)
   * <li> <tt>domain.get() != NULL</tt> (throw <tt>std::invalid_argument</tt>)
   * <li> <tt>range->dim() > 0</tt> (throw <tt>std::invalid_argument</tt>)
   * <li> <tt>domain->dim() > 0</tt> (throw <tt>std::invalid_argument</tt>)
   * <li> [<tt>col_vecs != NULL</tt>]
   *      <tt>col_vecs[j-1].get() != NULL && col_vecs[j-1]->space()->is_compatible(*range) == true</tt>,
   *      for <tt>j=1..domain->dim()</tt>
   * </ul>
   *
   * Postconditions:<ul>
   * <tt> <tt>this->range().get() == range.get()</tt>
   * <tt> <tt>this->domain().get() == domain.get()</tt>
   * <li> [<tt>col_vecs != NULL</tt>] <tt>this->col(j).get() == col_vecs[j-1].get()</tt>,
   *      for <tt>j=1..domain->dim()</tt>
   * </ul>
   */
  void initialize(
    const Teuchos::RefCountPtr<const VectorSpaceBase<Scalar> >          &range
    ,const Teuchos::RefCountPtr<const VectorSpaceBase<Scalar> >         &domain
    ,const Teuchos::RefCountPtr<VectorBase<Scalar> >                    col_vecs[] = NULL
    );

  /// Set uninitialized.
  void set_uninitialized();

  //@}

  /** @name Overridden from LinearOpBase */
  //@{
  /** \brief . */
  Teuchos::RefCountPtr<const VectorSpaceBase<Scalar> > range() const;
  /** \brief . */
  Teuchos::RefCountPtr<const VectorSpaceBase<Scalar> > domain() const;
  //@}

  /** @name Overridden from MultiVectorBase */
  //@{
  /** \brief . */
  Teuchos::RefCountPtr<VectorBase<Scalar> > col(Index j);
  /** \brief . */
  Teuchos::RefCountPtr<MultiVectorBase<Scalar> > subView( const Range1D& col_rng );
  //@}

private:
  
  Teuchos::RefCountPtr<const VectorSpaceBase<Scalar> >        range_;
  Teuchos::RefCountPtr<const VectorSpaceBase<Scalar> >        domain_;
  std::vector< Teuchos::RefCountPtr<VectorBase<Scalar> > >    col_vecs_;
  int                                                         num_cols_;
  
}; // end class MultiVectorCols

} // end namespace Thyra

#endif // TSF_MULTI_VECTOR_COlS_DECL_HPP
