#ifndef __TSQR_TBB_TbbMgs_hpp
#define __TSQR_TBB_TbbMgs_hpp

#include <algorithm>
#include <cassert>
#include <cmath>
#include <numeric>
#include <utility> // std::pair

#include <TSQR/Tsqr_MessengerBase.hpp>
#include <TSQR/Tsqr_ScalarTraits.hpp>
#include <TSQR/Tsqr_Util.hpp>

#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_reduce.h>
#include <tbb/partitioner.h>

// #define TBB_MGS_DEBUG 1
#ifdef TBB_MGS_DEBUG
#  include <iostream>
using std::cerr;
using std::endl;
#endif // TBB_MGS_DEBUG

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

namespace TSQR {
  namespace TBB {

    // Forward declaration
    template< class LocalOrdinal, class Scalar >
    class TbbMgs {
    public:
      typedef Scalar scalar_type;
      typedef LocalOrdinal ordinal_type;
      typedef typename ScalarTraits<Scalar>::magnitude_type magnitude_type;

      TbbMgs (MessengerBase< Scalar >* const messenger) : messenger_ (messenger) {}
    
      void 
      mgs (const LocalOrdinal nrows_local, 
	   const LocalOrdinal ncols, 
	   Scalar A_local[], 
	   const LocalOrdinal lda_local,
	   Scalar R[],
	   const LocalOrdinal ldr);

    private:
      MessengerBase< Scalar >* messenger_;
    };

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

    namespace details {

      /// Compute y'*x (where y' means conjugate transpose in the
      /// complex case, and transpose in the real case).
      template< class LocalOrdinal, class Scalar >
      class TbbDot {
      public:
	void operator() (const tbb::blocked_range< LocalOrdinal >& r) {
	  // The TBB book likes this copying of pointers into the local routine.
	  // It probably helps the compiler do optimizations.
	  const Scalar* const x = x_;
	  const Scalar* const y = y_;
	  Scalar local_result = result_;

#ifdef TBB_MGS_DEBUG
	  // cerr << "Range: [" << r.begin() << ", " << r.end() << ")" << endl;
	  // for (LocalOrdinal k = r.begin(); k != r.end(); ++k)
	  //   cerr << "(x[" << k << "], y[" << k << "]) = (" << x[k] << "," << y[k] << ")" << " ";
	  // cerr << endl;
#endif // TBB_MGS_DEBUG

	  for (LocalOrdinal i = r.begin(); i != r.end(); ++i)
	    local_result += x[i] * ScalarTraits< Scalar >::conj (y[i]);

#ifdef TBB_MGS_DEBUG
	  //	  cerr << "-- Final value = " << local_result << endl;
#endif // TBB_MGS_DEBUG

	  result_ = local_result;
	}
	/// Result of the reduction.
	Scalar result() const { return result_; }

	/// Ordinary constructor
	TbbDot (const Scalar* const x, const Scalar* const y) :
	  result_ (Scalar(0)), x_ (x), y_ (y) {}

	/// "Split constructor" for TBB reductions
	TbbDot (TbbDot& d, tbb::split) : 
	  result_ (Scalar(0)), x_ (d.x_), y_ (d.y_)
	{}
	/// "Join" operator for TBB reductions; it tells TBB how to
	/// combine two subproblems.
	void join (const TbbDot& d) { result_ += d.result(); }

      private:
	// Default constructor doesn't make sense.
	TbbDot ();

	Scalar result_;
	const Scalar* const x_;
	const Scalar* const y_;
      };

      template< class LocalOrdinal, class Scalar >
      class TbbScale {
      public:
	TbbScale (Scalar* const x, const Scalar& denom) : x_ (x), denom_ (denom) {}

	// TBB demands that this be a "const" operator, in order for
	// the parallel_for expression to compile.  Strictly speaking,
	// it is const, because it does not change the address of the
	// pointer x_ (only the values stored there).
	void operator() (const tbb::blocked_range< LocalOrdinal >& r) const {
	  // TBB likes arrays to have their pointers copied like this in
	  // the operator() method.  I suspect it has something to do
	  // with compiler optimizations.  If C++ supported the
	  // "restrict" keyword, here would be a good place to add it...
	  Scalar* const x = x_;
	  const Scalar denom = denom_;
	  for (LocalOrdinal i = r.begin(); i != r.end(); ++i)
	    x[i] = x[i] / denom;
	}
      private:
	Scalar* const x_;
	const Scalar denom_;
      };

      template< class LocalOrdinal, class Scalar >
      class TbbAxpy {
      public:
	TbbAxpy (const Scalar& alpha, const Scalar* const x, Scalar* const y) : 
	  alpha_ (alpha), x_ (x), y_ (y) 
	{}
	// TBB demands that this be a "const" operator, in order for
	// the parallel_for expression to compile.  Strictly speaking,
	// it is const, because it does change the address of the
	// pointer y_ (only the values stored there).
	void operator() (const tbb::blocked_range< LocalOrdinal >& r) const {
	  const Scalar alpha = alpha_;
	  const Scalar* const x = x_;
	  Scalar* const y = y_;
	  for (LocalOrdinal i = r.begin(); i != r.end(); ++i)
	    y[i] = y[i] + alpha * x[i];
	}
      private:
	const Scalar alpha_;
	const Scalar* const x_;
	Scalar* const y_;
      };

      template< class LocalOrdinal, class Scalar >
      class TbbNormSquared {
      public:
	void operator() (const tbb::blocked_range< LocalOrdinal >& r) {
	  // Doing the right thing in the complex case requires taking
	  // an absolute value.  We want to avoid this additional cost
	  // in the real case, which is why we check is_complex.
	  if (ScalarTraits< Scalar >::is_complex) 
	    {
	      // The TBB book likes this copying of pointers into the local routine.
	      // It probably helps the compiler do optimizations.
	      const Scalar* const x = x_;
	      for (LocalOrdinal i = r.begin(); i != r.end(); ++i)
		{
		  const Scalar xi = ScalarTraits< Scalar >::abs (x[i]);
		  result_ += xi * xi;
		}
	    }
	  else
	    {
	      const Scalar* const x = x_;
	      for (LocalOrdinal i = r.begin(); i != r.end(); ++i)
		{
		  const Scalar xi = x[i];
		  result_ += xi * xi;
		}
	    }
	}
	Scalar result() const { return result_; }

	TbbNormSquared (const Scalar* const x) :
	  result_ (Scalar(0)), x_ (x) {}
	TbbNormSquared (TbbNormSquared& d, tbb::split) : 
	  result_ (Scalar(0)), x_ (d.x_) {}
	void join (const TbbNormSquared& d) { result_ += d.result(); }

      private:
	// Default constructor doesn't make sense
	TbbNormSquared ();

	Scalar result_;
	const Scalar* const x_;
      };

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
  
      template< class LocalOrdinal, class Scalar >
      class TbbMgsOps {
      private:
	typedef tbb::blocked_range< LocalOrdinal > range_type;
	MessengerBase< Scalar >* const messenger_;

      public:
	TbbMgsOps (MessengerBase< Scalar >* const messenger) : 
	  messenger_ (messenger) {}

	Scalar
	axpy (const LocalOrdinal nrows_local,
	      const Scalar alpha,
	      const Scalar x_local[],
	      Scalar y_local[]) const
	{
	  using tbb::auto_partitioner;
	  using tbb::parallel_for;

	  TbbAxpy< LocalOrdinal, Scalar > axpyer (alpha, x_local, y_local);
	  parallel_for (range_type(0, nrows_local), axpyer, auto_partitioner());
	}

	Scalar
	scale (const LocalOrdinal nrows_local, 
	       Scalar x_local[], 
	       const Scalar denom) const
	{
	  using tbb::auto_partitioner;
	  using tbb::parallel_for;

	  // "scaler" is spelled that way (and not as "scalar") on
	  // purpose.  Think about it.
	  TbbScale< LocalOrdinal, Scalar > scaler (x_local, denom);
	  parallel_for (range_type(0, nrows_local), scaler, auto_partitioner());
	}

	/// $y^* \cdot x$: conjugate transpose when Scalar is complex,
	/// else regular transpose.
	Scalar
	dot (const LocalOrdinal nrows_local, 
	     const Scalar x_local[], 
	     const Scalar y_local[])
	{
	  Scalar local_result (0);
	  if (true)
	    {
	      if (true)
		{
		  TbbDot< LocalOrdinal, Scalar > dotter (x_local, y_local);
		  dotter(range_type(0, nrows_local));
		  local_result = dotter.result();
		}
	      else
		{
		  using tbb::auto_partitioner;
		  using tbb::parallel_reduce;

		  TbbDot< LocalOrdinal, Scalar > dotter (x_local, y_local);
		  parallel_reduce (range_type(0, nrows_local), dotter, auto_partitioner());
		  local_result = dotter.result();
		}
	    }
	  else 
	    {
	      for (LocalOrdinal i = 0; i != nrows_local; ++i)
		local_result += x_local[i] * ScalarTraits< Scalar >::conj(y_local[i]);
	    }
    
	  // FIXME (mfh 23 Apr 2010) Does MPI_SUM do the right thing for
	  // complex or otherwise general MPI data types?  Perhaps an MPI_Op
	  // should belong in the MessengerBase...
	  return messenger_->globalSum (local_result);
	}

	typename ScalarTraits< Scalar >::magnitude_type 
	norm2 (const LocalOrdinal nrows_local, 
	       const Scalar x_local[])
	{
	  using tbb::auto_partitioner;
	  using tbb::parallel_reduce;
	  typename ScalarTraits< Scalar >::magnitude_type magnitude_type;

	  TbbNormSquared< LocalOrdinal, Scalar > normer (x_local);
	  parallel_reduce (range_type(0, nrows_local), normer, auto_partitioner());
	  const Scalar local_result = normer.result();
	  const Scalar global_result = messenger_->globalSum (local_result);
	  // sqrt doesn't make sense if the type of Scalar is complex,
	  // even if the imaginary part of global_result is zero.
	  return sqrt (ScalarTraits< Scalar >::abs (global_result));
	}

	Scalar
	project (const LocalOrdinal nrows_local, 
		 const Scalar q_local[], 
		 Scalar v_local[])
	{
	  const Scalar coeff = this->dot (nrows_local, v_local, q_local);
	  this->axpy (nrows_local, -coeff, q_local, v_local);
	  return coeff;
	}
      };
    } // namespace details

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

    template< class LocalOrdinal, class Scalar >
    void
    TbbMgs< LocalOrdinal, Scalar >::mgs (const LocalOrdinal nrows_local, 
					 const LocalOrdinal ncols, 
					 Scalar A_local[], 
					 const LocalOrdinal lda_local,
					 Scalar R[],
					 const LocalOrdinal ldr)
    {
      const int my_rank = messenger_->rank ();
      details::TbbMgsOps< LocalOrdinal, Scalar > ops (messenger_);
      
      for (LocalOrdinal j = 0; j < ncols; ++j)
	{
	  Scalar* const v = &A_local[j*lda_local];
	  for (LocalOrdinal i = 0; i < j; ++i)
	    {
	      const Scalar* const q = &A_local[i*lda_local];
	      R[i + j*ldr] = ops.project (nrows_local, q, v);
#ifdef TBB_MGS_DEBUG
	      if (my_rank == 0)
		cerr << "(i,j) = (" << i << "," << j << "): coeff = " << R[i + j*ldr] << endl;
#endif // TBB_MGS_DEBUG
	    }
	  const magnitude_type denom = ops.norm2 (nrows_local, v);
#ifdef TBB_MGS_DEBUG
	  if (my_rank == 0)
	    cerr << "j = " << j << ": denom = " << denom << endl;
#endif // TBB_MGS_DEBUG

	  // FIXME (mfh 29 Apr 2010)
	  //
	  // NOTE IMPLICIT CAST.  This should work for complex numbers.
	  // If it doesn't work for your Scalar data type, it means that
	  // you need a different data type for the diagonal elements of
	  // the R factor, than you need for the other elements.  This
	  // is unlikely if we're comparing MGS against a Householder QR
	  // factorization; I don't really understand how the latter
	  // would work (not that it couldn't be given a sensible
	  // interpretation) in the case of Scalars that aren't plain
	  // old real or complex numbers.
	  R[j + j*ldr] = Scalar (denom);
	  ops.scale (nrows_local, v, denom);
	}
    }
  } // namespace TBB
} // namespace TSQR

#endif // __TSQR_TBB_TbbMgs_hpp

