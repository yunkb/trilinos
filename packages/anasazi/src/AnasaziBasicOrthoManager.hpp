// @HEADER
// ***********************************************************************
//
//                 Anasazi: Block Eigensolvers Package
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


/*! \file AnasaziBasicOrthoManager.hpp
  \brief Basic implementation of the Anasazi::OrthoManager class
*/

#ifndef ANASAZI_BASIC_ORTHOMANAGER_HPP
#define ANASAZI_BASIC_ORTHOMANAGER_HPP

/*!   \class Anasazi::BasicOrthoManager
      \brief An implementation of the Anasazi::MatOrthoManager that performs orthogonalization
      using (potentially) multiple steps of classical Gram-Schmidt.
      
      \author Chris Baker, Ulrich Hetmaniuk, Rich Lehoucq, and Heidi Thornquist
*/

// #define ORTHO_DEBUG

#include "AnasaziMatOrthoManager.hpp"
#include "AnasaziMultiVecTraits.hpp"
#include "AnasaziOperatorTraits.hpp"

namespace Anasazi {

  template<class ScalarType, class MV, class OP>
  class BasicOrthoManager : public MatOrthoManager<ScalarType,MV,OP> {

  private:
    typedef typename Teuchos::ScalarTraits<ScalarType>::magnitudeType MagnitudeType;
    typedef Teuchos::ScalarTraits<ScalarType>  SCT;
    typedef MultiVecTraits<ScalarType,MV>      MVT;
    typedef OperatorTraits<ScalarType,MV,OP>   OPT;

  public:
    
    //! @name Constructor/Destructor
    //@{ 
    //! Constructor specifying re-orthogonalization tolerance.
    BasicOrthoManager( Teuchos::RefCountPtr<const OP> Op = Teuchos::null,
                       const MagnitudeType kappa = SCT::magnitude(1.5625) ) : MatOrthoManager<ScalarType,MV,OP>(Op), _kappa(kappa) {};

    //! Destructor
    ~BasicOrthoManager() {};
    //@}


    //! @name Accessor routines
    //@{ 

    //! Set parameter for re-orthogonalization threshhold.
    void setKappa( const MagnitudeType kappa ) { _kappa = kappa; };

    //! Return parameter for re-orthogonalization threshhold.
    MagnitudeType getKappa() const { return _kappa; } 

    //@} 


    //! @name Orthogonalization methods
    //@{ 

    /*! \brief Given a list of (mutually and independently) orthonormal bases \c Q, this method
     * takes a multivector \c X and projects it onto the space orthogonal to the individual \c Q[i], 
     * returning optionally the coefficients of \c X in the individual \c Q[i]. All of this is done with respect
     * to innerProd(). The method has the option of
     * exploiting a caller-provided \c MX, and returning updated information to the caller.
     *  
     @param X [in/out] The multivector to be modified.
       On output, \c X will be orthogonal to \c Q[i] with respect to innerProd().

     @param MX [in/out] The image of \c X under the operator \c M. 
       If <tt>MX != 0</tt>: On input, this is expected to be consistant with \c X. On output, this is updated consistant with updates to \c X.
       If <tt>MX == 0</tt> or <tt>M == 0</tt>: \c MX is not referenced.
            
     @param C [out] The coefficients of \c X in the \c Q[i], with respect to innerProd().

     @param Q [in] A list of multivectors specifying the bases to be orthogonalized against. Each \c Q[i] is assumed to have
     orthonormal columns, and the \c Q[i] are assumed to be mutually orthogonal.
    */
    virtual void project ( MV &X, Teuchos::RefCountPtr<MV> MX, 
                                  Teuchos::Array<Teuchos::RefCountPtr<Teuchos::SerialDenseMatrix<int,ScalarType> > > C, 
                                  Teuchos::Array<Teuchos::RefCountPtr<const MV> > Q) const;

    /*! \brief This method takes a multivector and orthonormalizes the columns, with respect to \c innerProd().
     *  The method has the option of
     *  exploiting a caller-provided \c MX, and returning updated information to the caller.
     *  
     @param X [in/out] The multivector to the modified. 
       On output, the columns are Op-orthonormal
    
     @param MX [in/out] The image of \c X under the operator \c Op. 
       If \f$ MX != 0\f$: On input, this is expected to be consistant with \c X. On output, this is updated consistant with updates to \c X.
       If \f$ MX == 0\f$ or \f$ Op == 0\f$: \c MX is not referenced.
    
     @return Rank of the basis computed by this method.
    */
    int normalize ( MV &X, Teuchos::RefCountPtr<MV> MX, 
                    Teuchos::RefCountPtr<Teuchos::SerialDenseMatrix<int,ScalarType> > R) const;


    /*! \brief This method takes a multivector and projects it onto the space orthogonal to 
     *  another given multivector.  It also orthonormalizes the 
     *  columns of the resulting multivector. Both of these operations are conducted 
     *  with respect to \c innerProd().
     *  The method has the option of
     *  exploiting a caller-provided \c MX, and returning updated information to the caller.
     *  
     @param X [in/out] The multivector to the modified. 
       On output, the columns of X are Op-orthonormal and Op-orthogonal to Q.
      
     @param MX [in/out] The image of \c X under the operator \c Op. 
       If \f$ MX != 0\f$: On input, this is expected to be consistant with \c X. On output, this is updated consistant with updates to \c X.
       If \f$ MX == 0\f$ or \f$ Op == 0\f$: \c MX is not referenced.
      
     @param Q [in] A multivector specifying the space to be orthogonalized against. \c Q is assumed to have orthonormal
     columns with respect to \c innerProd().
      
     @return Rank of the basis computed by this method.
    */
    int projectAndNormalize ( MV &X, Teuchos::RefCountPtr<MV> MX, 
                              Teuchos::Array<Teuchos::RefCountPtr<Teuchos::SerialDenseMatrix<int,ScalarType> > > C, 
                              Teuchos::RefCountPtr<Teuchos::SerialDenseMatrix<int,ScalarType> > R, 
                              Teuchos::Array<Teuchos::RefCountPtr<const MV> > Q) const;

    //@}

    //! @name Error methods
    //@{ 

    /*! \brief This method computes the error in orthonormality of a multivector, measured via \f$ \|X^T Op X - I\|_F \f$.
     *  The method has the option of
     *  exploiting a caller-provided \c MX.
     */
    typename Teuchos::ScalarTraits<ScalarType>::magnitudeType 
    orthonormError(const MV &X, Teuchos::RefCountPtr<const MV> MX) const;

    /*! \brief This method computes the error in orthogonality of two multivectors, measured via \f$ \|Q^T Op X \|_F \f$.
     *  The method has the option of
     *  exploiting a caller-provided \c MX.
     */
    typename Teuchos::ScalarTraits<ScalarType>::magnitudeType 
    orthogError(const MV &X1, Teuchos::RefCountPtr<const MV> MX1, const MV &X2) const;

    //@}

  private:
    
    //! Parameter for re-orthogonalization.
    MagnitudeType _kappa;
  
    // ! Routine to find an orthonormal basis for the 
    int findBasis(MV &X, Teuchos::RefCountPtr<MV> MX, 
                         Teuchos::RefCountPtr<Teuchos::SerialDenseMatrix<int,ScalarType> > C, 
                         bool completeBasis, int howMany = -1 ) const;
    
  };


  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Compute the distance from orthonormality
  template<class ScalarType, class MV, class OP>
  typename Teuchos::ScalarTraits<ScalarType>::magnitudeType 
  BasicOrthoManager<ScalarType,MV,OP>::orthonormError(const MV &X, Teuchos::RefCountPtr<const MV> MX) const {
    const ScalarType ONE = SCT::one();
    int rank = MVT::GetNumberVecs(X);
    Teuchos::SerialDenseMatrix<int,ScalarType> xTx(rank,rank);
    innerProd(X,X,MX,xTx);
    for (int i=0; i<rank; i++) {
      xTx(i,i) -= ONE;
    }
    return xTx.normFrobenius();
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Compute the distance from orthogonality
  template<class ScalarType, class MV, class OP>
  typename Teuchos::ScalarTraits<ScalarType>::magnitudeType 
  BasicOrthoManager<ScalarType,MV,OP>::orthogError(const MV &X1, Teuchos::RefCountPtr<const MV> MX1, const MV &X2) const {
    int r1 = MVT::GetNumberVecs(X1);
    int r2  = MVT::GetNumberVecs(X2);
    Teuchos::SerialDenseMatrix<int,ScalarType> xTx(r2,r1);
    innerProd(X2,X1,MX1,xTx);
    return xTx.normFrobenius();
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Find an Op-orthonormal basis for span(X) - span(W)
  template<class ScalarType, class MV, class OP>
  int BasicOrthoManager<ScalarType, MV, OP>::projectAndNormalize(
                                    MV &X, Teuchos::RefCountPtr<MV> MX, 
                                    Teuchos::Array<Teuchos::RefCountPtr<Teuchos::SerialDenseMatrix<int,ScalarType> > > C, 
                                    Teuchos::RefCountPtr<Teuchos::SerialDenseMatrix<int,ScalarType> > R, 
                                    Teuchos::Array<Teuchos::RefCountPtr<const MV> > Q ) const {

    int nq = Q.length();
    int xc = MVT::GetNumberVecs( X );
    int xr = MVT::GetVecLength( X );
    int rank;

    /* if the user doesn't want to store the coefficienets, 
     * allocate some local memory for them 
     */
    if ( R == Teuchos::null ) {
      R = Teuchos::rcp( new Teuchos::SerialDenseMatrix<int,ScalarType>(xc,xc) );
    }

    /******   DO NO MODIFY *MX IF _hasOp == false   ******/
    if (this->_hasOp) {
      if (MX == Teuchos::null) {
        // we need to allocate space for MX
        MX = MVT::Clone(X,MVT::GetNumberVecs(X));
        OPT::Apply(*(this->_Op),X,*MX);
      }
    }
    else {
      // Op == I  -->  MX = X (ignore it if the user passed it in)
      MX = Teuchos::rcp( &X, false );
    }

    int mxc = MVT::GetNumberVecs( *MX );
    int mxr = MVT::GetVecLength( *MX );

    // short-circuit
    TEST_FOR_EXCEPTION( xc == 0 || xr == 0, OrthoError, "Anasazi::BasicOrthoManager::projectAndNormalize(): X must be non-empty" );

    int numbas = 0;
    for (int i=0; i<nq; i++) {
      numbas += MVT::GetNumberVecs( *Q[i] );
    }

    // check size of C, R
    TEST_FOR_EXCEPTION( R->numRows() != xc || R->numCols() != xc, OrthoError, 
                        "Anasazi::BasicOrthoManager::projectAndNormalize(): Size of X must be consistant with size of R" );
    // check size of X and MX
    TEST_FOR_EXCEPTION( xc<0 || xr<0 || mxc<0 || mxr<0, OrthoError, 
                        "Anasazi::BasicOrthoManager::projectAndNormalize(): MVT returned negative dimensions for X,MX" );
    // check size of X w.r.t. MX 
    TEST_FOR_EXCEPTION( xc!=mxc || xr!=mxr, OrthoError, 
                        "Anasazi::BasicOrthoManager::projectAndNormalize(): Size of X must be consistant with size of MX" );
    // check feasibility
    TEST_FOR_EXCEPTION( numbas+xc > xr, OrthoError, 
                        "Anasazi::BasicOrthoManager::projectAndNormalize(): Orthogonality constraints not feasible" );

    // orthogonalize all of X against Q
    project(X,MX,C,Q);

    Teuchos::SerialDenseMatrix<int,ScalarType> oldCoeff(xc,1);

    // start working
    rank = 0;
    int numTries = 10;   // each vector in X gets 10 random chances to escape degeneracy
    int oldrank = -1;
    do {
      int curxsize = xc - rank;

      // orthonormalize X, but quit if it is rank deficient
      // we can't let findBasis generated random vectors to complete the basis,
      // because it doesn't know about Q; we will do this ourselves below
      rank = findBasis(X,MX,R,false,curxsize);

      if (rank < xc && numTries == 10) {
        // we quit on this vector, and for the first time;
        // save the coefficient information, because findBasis will overwrite it
        for (int i=0; i<xc; i++) {
          oldCoeff(i,0) = (*R)(i,rank);
        }
      }

      if (oldrank != -1 && rank != oldrank) {
        // we moved on; restore the previous coefficients
        for (int i=0; i<xc; i++) {
          (*R)(i,oldrank) = oldCoeff(i,0);
        }
      }

      if (ret == xc) {
        // we are done
        break;
      }
      else {
        TEST_FOR_EXCEPTION( rank < oldrank, OrthoError,   
                            "Anasazi::BasicOrthoManager::projectAndNormalize(): basis lost rank; this shouldn't happen");

        if (rank != oldrank) {
          // we added a basis vector from random info; reset the chance counter
          numTries = 10;
        }

        // store old rank
        oldrank = rank;

        // has this vector run out of chances to escape degeneracy?
        if (numTries <= 0) {
          break;
        }
        // use one of this vector's chances
        numTries--;

        // randomize troubled direction
#ifdef ORTHO_DEBUG
        cout << "Random for column " << rank << endl;
#endif
        Teuchos::RefCountPtr<MV> curX, curMX;
        std::vector<int> ind(1);
        ind[0] = rank;
        curX  = MVT::CloneView(X,ind);
        MVT::MvRandom(*curX);
        if (this->_hasOp) {
          curMX = MVT::CloneView(*MX,ind);
          OPT::Apply( *(this->_Op), *curX, *curMX );
        }

        // orthogonalize against Q
        // if !this->_hasOp, the curMX will be ignored.
        // we don't care about these coefficients; in fact, we need to preserve the previous coeffs
        project(*curX,curMX,Teuchos::null,Q);
      }
    } while (1);

    // this should not raise an exception; but our post-conditions oblige us to check
    TEST_FOR_EXCEPTION( rank > xc || rank < 0, OrthoError, 
                        "Anasazi::BasicOrthoManager::projectAndNormalize(): Debug error in rank variable." );
    return rank;
  }



  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Find an M-orthonormal basis for span(X), with rank numvectors(X)
  template<class ScalarType, class MV, class OP>
  int BasicOrthoManager<ScalarType, MV, OP>::normalize(
                                MV &X, Teuchos::RefCountPtr<MV> MX, 
                                Teuchos::RefCountPtr<Teuchos::SerialDenseMatrix<int,ScalarType> > R ) const {
    // call findBasis, with the instruction to try to generate a basis of rank numvecs(X)
    return findBasis(X, MX, R, true );
  }



  //////////////////////////////////////////////////////////////////////////////////////////////////
  template<class ScalarType, class MV, class OP>
  void BasicOrthoManager<ScalarType, MV, OP>::project(
                          MV &X, Teuchos::RefCountPtr<MV> MX, 
                          Teuchos::Array<Teuchos::RefCountPtr<Teuchos::SerialDenseMatrix<int,ScalarType> > > C, 
                          Teuchos::Array<Teuchos::RefCountPtr<const MV> > Q) const {
    // For the inner product defined by the operator M or the identity (M == 0)
    //   -> Orthogonalize X against each Q[i]
    // Modify MX accordingly
    //
    // Note that when M is 0, MX is not referenced
    //
    // Parameter variables
    //
    // X  : Vectors to be transformed
    //
    // MX : Image of the block vector X by the mass matrix
    //
    // Q  : Bases to orthogonalize against. These are assumed orthonormal, mutually and independenty.
    //

    ScalarType    ONE  = SCT::one();
    MagnitudeType ZERO = SCT::magnitude(SCT::zero());
    ScalarType    EPS  = SCT::eps();

    int xc = MVT::GetNumberVecs( X );
    int xr = MVT::GetVecLength( X );
    int nq = Q.length();
    std::vector<int> qcs(nq);
    // short-circuit
    if (nq == 0 || xc == 0 || xr == 0) {
      return;
    }
    int qr = MVT::GetVecLength ( *Q[0] );
    // if we don't have enough C, expand it with null references
    // if we have too many, resize to throw away the latter ones
    // if we have exactly as many as we have Q, this call has no effect
    C.resize(nq);


    /******   DO NO MODIFY *MX IF _hasOp == false   ******/
    if (this->_hasOp) {
      if (MX == Teuchos::null) {
        // we need to allocate space for MX
        MX = MVT::Clone(X,MVT::GetNumberVecs(X));
        OPT::Apply(*(this->_Op),X,*MX);
      }
    }
    else {
      // Op == I  -->  MX = X (ignore it if the user passed it in)
      MX = Teuchos::rcp( &X, false );
    }
    int mxc = MVT::GetNumberVecs( *MX );
    int mxr = MVT::GetVecLength( *MX );

    // check size of X and Q w.r.t. common sense
    TEST_FOR_EXCEPTION( xc<0 || xr<0 || mxc<0 || mxr<0, OrthoError, 
                        "Anasazi::BasicOrthoManager::project(): MVT returned negative dimensions for X,MX" );
    // check size of X w.r.t. MX and Q
    TEST_FOR_EXCEPTION( xc!=mxc || xr!=mxr || xr!=qr, OrthoError, 
                        "Anasazi::BasicOrthoManager::project(): Size of X not consistant with MX,Q" );

    // tally up size of all Q and check/allocate C
    int baslen = 0;
    for (int i=0; i<nq; i++) {
      TEST_FOR_EXCEPTION( MVT::GetVecLength( *Q[i] ) != qr, OrthoError, 
                          "Anasazi::BasicOrthoManager::project(): Q lengths not mutually consistant" );
      qcs[i] = MVT::GetNumberVecs( *Q[i] );
      TEST_FOR_EXCEPTION( qr < qcs[i], OrthoError, 
                          "Anasazi::BasicOrthoManager::project(): Q has less rows than columns" );
      baslen += qcs[i];

      // check size of C[i]
      if ( C[i] == Teuchos::null ) {
        C[i] = Teuchos::rcp( new Teuchos::SerialDenseMatrix<int,ScalarType>(qcs[i],xc) );
      }
      else {
        TEST_FOR_EXCEPTION( C[i]->numRows() != qcs[i] || C[i]->numCols() != xc , OrthoError, 
                           "Anasazi::BasicOrthoManager::project(): Size of Q not consistant with size of C" );
      }
    }

    // Perform the Gram-Schmidt transformation for a block of vectors

    // Compute the initial M-norms
    std::vector<ScalarType> oldDot( xc );
    MVT::MvDot( X, *MX, &oldDot );

    Teuchos::Array<Teuchos::RefCountPtr<MV> > MQ(nq);
    // Define the product Q^T * (M*X)
    for (int i=0; i<nq; i++) {
      // Multiply Q' with MX
      innerProd(*Q[i],X,MX,*C[i]);
      // Multiply by Q and subtract the result in X
      MVT::MvTimesMatAddMv( -ONE, *Q[i], *C[i], ONE, X );

      // Update MX, with the least number of applications of M as possible
      if (this->_hasOp) {
        if (xc <= qcs[i]) {
          OPT::Apply( *(this->_Op), X, *MX);
        }
        else {
          // this will possibly be used again below; don't delete it
          MQ[i] = MVT::Clone( *Q[i], qcs[i] );
          OPT::Apply( *(this->_Op), *Q[i], *MQ[i] );
          MVT::MvTimesMatAddMv( -ONE, *MQ[i], *C[i], ONE, *MX );
        }
      }
    }

    // Compute new M-norms
    std::vector<ScalarType> newDot(xc);
    MVT::MvDot( X, *MX, &newDot );

    // determine (individually) whether to do another step of classical Gram-Schmidt
    for (int j = 0; j < xc; ++j) {
      
      if ( SCT::magnitude(_kappa*newDot[j]) < SCT::magnitude(oldDot[j]) ) {

        for (int i=0; i<nq; i++) {
          Teuchos::SerialDenseMatrix<int,ScalarType> C2(*C[i]);
          
          // Apply another step of classical Gram-Schmidt
          innerProd(*Q[i],X,MX,C2);
          *C[i] += C2;
          MVT::MvTimesMatAddMv( -ONE, *Q[i], C2, ONE, X );
          
          // Update MX, with the least number of applications of M as possible
          if (this->_hasOp) {
            if (MQ[i].get()) {
              // MQ was allocated and computed above; use it
              MVT::MvTimesMatAddMv( -ONE, *MQ[i], C2, ONE, *MX );
            }
            else if (xc <= qcs[i]) {
              // MQ was not allocated and computed above; it was cheaper to use X before and it still is
              OPT::Apply( *(this->_Op), X, *MX);
            }
          }
        }
        break;
      } // if (_kappa*newDot[j] < oldDot[j])
    } // for (int j = 0; j < xc; ++j)
  }


  //////////////////////////////////////////////////////////////////////////////////////////////////
  // Find an M-orthonormal basis for span(X), with the option of extending the subspace so that 
  // the rank is numvectors(X)
  template<class ScalarType, class MV, class OP>
  int BasicOrthoManager<ScalarType, MV, OP>::findBasis(
                MV &X, Teuchos::RefCountPtr<MV> MX, 
                Teuchos::RefCountPtr<Teuchos::SerialDenseMatrix<int,ScalarType> > R,
                bool completeBasis, int howMany ) const {
    // For the inner product defined by the operator M or the identity (M == 0)
    //   -> Orthonormalize X 
    // Modify MX accordingly
    //
    // Note that when M is 0, MX is not referenced
    //
    // Parameter variables
    //
    // X  : Vectors to be orthonormalized
    //
    // MX : Image of the multivector X under the operator M
    //
    // M  : Pointer to the operator for the inner product
    //
    // TODO: add reference
    // kappa= Coefficient determining when to perform a second Gram-Schmidt step
    //        Default value = 1.5625 = (1.25)^2 (as suggested in Parlett's book)
    //

    const ScalarType ONE  = SCT::one();
    const MagnitudeType ZERO = SCT::magnitude(SCT::zero());
    const ScalarType EPS  = SCT::eps();

    int xc = MVT::GetNumberVecs( X );
    int xr = MVT::GetVecLength( X );

    if (howMany == -1) {
      howMany = xc;
    }

    /*******************************************************
     *  If _hasOp == false, we will not reference MX below *
     *******************************************************/

    // if Op==null, MX == X (via pointer)
    // Otherwise, either the user passed in MX or we will allocated and compute it
    if (this->_hasOp) {
      if (MX == Teuchos::null) {
        // we need to allocate space for MX
        MX = MVT::Clone(X,xc);
        OPT::Apply(*(this->_Op),X,*MX);
      }
    }

    /* if the user doesn't want to store the coefficienets, 
     * allocate some local memory for them 
     */
    if ( R == Teuchos::null ) {
      R = Teuchos::rcp( new Teuchos::SerialDenseMatrix<int,ScalarType>(xc,xc) );
    }

    int mxc = (this->_hasOp) ? MVT::GetNumberVecs( *MX ) : xc;
    int mxr = (this->_hasOp) ? MVT::GetVecLength( *MX )  : xr;

    // check size of C, R
    TEST_FOR_EXCEPTION( xc == 0 || xr == 0, OrthoError, 
                        "Anasazi::BasicOrthoManager::findBasis(): X must be non-empty" );
    TEST_FOR_EXCEPTION( R->numRows() != xc || R->numCols() != xc, OrthoError, 
                        "Anasazi::BasicOrthoManager::findBasis(): Size of X not consistant with size of R" );
    TEST_FOR_EXCEPTION( xc != mxc || xr != mxr, OrthoError, 
                        "Anasazi::BasicOrthoManager::findBasis(): Size of X not consistant with size of MX" );
    TEST_FOR_EXCEPTION( xc > xr, OrthoError, 
                        "Anasazi::BasicOrthoManager::findBasis(): Size of X not feasible for normalization" );
    TEST_FOR_EXCEPTION( howMany < 0 || howMany > xc, OrthoError, 
                        "Anasazi::BasicOrthoManager::findBasis(): Invalid howMany parameter" );

    /* xstart is which column we are starting the process with, based on howMany
     * columns before xstart are assumed to be M-orthonormal already
     */
    int xstart = xc - howMany;

    for (int j = xstart; j < xc; j++) {

      // numX is 
      // * number of currently orthonormal columns of X
      // * the index of the current column of X
      int numX = j;

      // Get a view of the vector currently being worked on.
      std::vector<int> index(1);
      index[0] = numX;
      Teuchos::RefCountPtr<MV> Xj = MVT::CloneView( X, index );
      Teuchos::RefCountPtr<MV> MXj;
      if ((this->_hasOp)) {
        // MXj is a view of the current vector in MX
        MXj = MVT::CloneView( *MX, index );
      }
      else {
        // MXj is a pointer to Xj, and MUST NOT be modified
        MXj = Xj;
      }

      // Get a view of the previous vectors.
      std::vector<int> prev_idx( numX );
      Teuchos::RefCountPtr<const MV> prevX, prevMX;

      if (numX > 0) {
        for (int i=0; i<numX; i++) {
          prev_idx[i] = i;
        }
        prevX = MVT::CloneView( X, prev_idx );
        if (this->_hasOp) {
          prevMX = MVT::CloneView( *MX, prev_idx );
        }
      } 

      bool rankDef = true;
      /* numTrials>0 will denote that the current vector was randomized for the purpose
       * of finding a basis vector, and that the coefficients of that vector should
       * not be stored in R
       */
      for (int numTrials = 0; numTrials < 10; numTrials++) {

        // Make storage for these Gram-Schmidt iterations.
        Teuchos::SerialDenseMatrix<int,ScalarType> product(numX, 1);
        std::vector<ScalarType> oldDot( 1 ), newDot( 1 );

        //
        // Save old MXj vector and compute M-norm
        //
        Teuchos::RefCountPtr<MV> oldMXj = MVT::CloneCopy( *MXj ); 
        MVT::MvDot( *Xj, *MXj, &oldDot );
        // Xj^H M Xj should be real and positive, by the hermitian positive definiteness of M
        TEST_FOR_EXCEPTION( SCT::real(oldDot[0]) < ZERO, OrthoError, 
                            "Anasazi::BasicOrthoManager::findBasis(): Negative definiteness discovered in inner product" );

        if (numX > 0) {
          // Apply the first step of Gram-Schmidt

          // product <- prevX^T MXj
          innerProd(*prevX,*Xj,MXj,product);

          // Xj <- Xj - prevX prevX^T MXj   
          //     = Xj - prevX product
          MVT::MvTimesMatAddMv( -ONE, *prevX, product, ONE, *Xj );

          // Update MXj
          if (this->_hasOp) {
            // MXj <- M*Xj_new
            //      = M*(Xj_old - prevX prevX^T MXj)
            //      = MXj - prevMX product
            MVT::MvTimesMatAddMv( -ONE, *prevMX, product, ONE, *MXj );
          }

          // Compute new M-norm
          MVT::MvDot( *Xj, *MXj, &newDot );

          // Check if a correction is needed.
          if ( SCT::magnitude(_kappa*newDot[0]) < SCT::magnitude(oldDot[0]) ) {
            // Apply the second step of Gram-Schmidt
            // This is the same as above
            Teuchos::SerialDenseMatrix<int,ScalarType> P2(numX,1);

            innerProd(*prevX,*Xj,MXj,P2);
            product += P2;
            MVT::MvTimesMatAddMv( -ONE, *prevX, P2, ONE, *Xj );
            if ((this->_hasOp)) {
              MVT::MvTimesMatAddMv( -ONE, *prevMX, P2, ONE, *MXj );
            }
          } // if (_kappa*newDot[0] < oldDot[0])

        } // if (numX > 0)

        // Compute M-norm with old MXj
        MVT::MvDot( *Xj, *oldMXj, &newDot );

        // save the coefficients, if we are working on the original vector and not a randomly generated one
        if (numTrials == 0) {
          for (int i=0; i<numX; i++) {
            (*R)(i,j) = product(i,0);
          }
        }

        // Check if Xj has any directional information left after the orthogonalization.
#ifdef ORTHO_DEBUG
        cout << "olddot: " << SCT::magnitude(oldDot[0]) << "    newdot: " << SCT::magnitude(newDot[0]);
#endif
        if ( SCT::magnitude(newDot[0]) > SCT::magnitude(oldDot[0]*EPS*EPS) && SCT::real(newDot[0]) > ZERO ) {
#ifdef ORTHO_DEBUG
          cout << " ACCEPTED" << endl;
#endif
          // Normalize Xj.
          // Xj <- Xj / sqrt(newDot)
          ScalarType diag = SCT::squareroot(SCT::magnitude(newDot[0]));

          MVT::MvAddMv( ONE/diag, *Xj, ZERO, *Xj, *Xj );
          if (this->_hasOp) {
            // Update MXj.
            MVT::MvAddMv( ONE/diag, *MXj, ZERO, *MXj, *MXj );
          }

          // save it, if it corresponds to the original vector and not a randomly generated one
          if (numTrials == 0) {
            (*R)(j,j) = diag;
          }

          // We are not rank deficient in this vector. Move on to the next vector in X.
          rankDef = false;
          break;
        }
        else {
#ifdef ORTHO_DEBUG
          cout << " REJECTED" << endl;
#endif
          // There was nothing left in Xj after orthogonalizing against previous columns in X.
          // X is rank deficient.
          // reflect this in the coefficients
          (*R)(j,j) = ZERO;

          if (completeBasis) {
            // Fill it with random information and keep going.
#ifdef ORTHO_DEBUG
            cout << "Random for column " << j << endl;
#endif
            MVT::MvRandom( *Xj );
            if (this->_hasOp) {
              OPT::Apply( *(this->_Op), *Xj, *MXj );
            }
          }
          else {
            rankDef = true;
            break;
          }

        } // if (norm > oldDot*EPS*EPS)

      }  // for (numTrials = 0; numTrials < 10; ++numTrials)

      // if rankDef == true, then quit and notify user of rank obtained
      if (rankDef == true) {
        (*R)(j,j) = ZERO;
        MVT::MvInit( *Xj, ZERO );
        if (this->_hasOp) {
          MVT::MvInit( *MXj, ZERO );
        }
        TEST_FOR_EXCEPTION( completeBasis, OrthoError, 
                            "Anasazi::BasicOrthoManager::findBasis(): Unable to complete basis" );
        return j;
      }

    } // for (j = 0; j < xc; ++j)

    return xc;
  }

} // namespace Anasazi

#endif // ANASAZI_BASIC_ORTHOMANAGER_HPP

