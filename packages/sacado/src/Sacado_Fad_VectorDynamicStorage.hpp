// $Id$ 
// $Source$ 
// @HEADER
// ***********************************************************************
// 
//                           Sacado Package
//                 Copyright (2006) Sandia Corporation
// 
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
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
// Questions? Contact David M. Gay (dmgay@sandia.gov) or Eric T. Phipps
// (etphipp@sandia.gov).
// 
// ***********************************************************************
// @HEADER

#ifndef SACADO_FAD_VECTORDYNAMICSTORAGE_HPP
#define SACADO_FAD_VECTORDYNAMICSTORAGE_HPP

#include "Sacado_Traits.hpp"
#include "Sacado_DynamicArrayTraits.hpp"

namespace Sacado {

  namespace Fad {

    //! Derivative array storage class using dynamic memory allocation
    template <typename T, typename S = T> 
    class VectorDynamicStorage {

    public:

      //! Default constructor
      VectorDynamicStorage(const T & x) : 
	v_(x), owns_mem(true), sz_(0), len_(0), stride_(1), val_(&v_), dx_(NULL)
      {}

      //! Constructor with size \c sz
      /*!
       * Initializes derivative array 0 of length \c sz
       */
      VectorDynamicStorage(const int sz, const T & x) : 
	v_(x), owns_mem(true), sz_(sz), len_(sz), stride_(1), val_(&v_) {
	dx_ = ds_array<S>::get_and_fill(sz_);
      }

      //! Constructor with supplied memory
      VectorDynamicStorage(const int sz, T* x, S* dx_p, const int stride,
			   bool zero_out) : 
	v_(), owns_mem(false), sz_(sz), len_(sz), stride_(stride), 
	val_(x), dx_(dx_p) {
	if (zero_out)
	  zero();
      }

      //! Copy constructor
      VectorDynamicStorage(const VectorDynamicStorage& x) : 
	v_(*x.val_), owns_mem(true), sz_(x.sz_), len_(x.sz_), 
	stride_(1), val_(&v_)  {
	dx_ = ds_array<S>::strided_get_and_fill(x.dx_, x.stride_, sz_);
      }
      
      //! Destructor
      ~VectorDynamicStorage() {
	if (owns_mem) {
	  if (len_ != 0)
	    ds_array<S>::destroy_and_release(dx_, len_);
	}
      }

      //! Assignment
      VectorDynamicStorage& operator=(const VectorDynamicStorage& x) { 
	*val_ = *x.val_;
	if (sz_ != x.sz_) {
	  sz_ = x.sz_;
	  if (x.sz_ > len_) {
	    if (!owns_mem)
	      throw "Can\'t resize beyond original size when memory isn't owned!";
	    if (len_ != 0)
	      ds_array<S>::destroy_and_release(dx_, len_);
	    len_ = x.sz_;
	    dx_ = ds_array<S>::strided_get_and_fill(x.dx_, x.stride_, sz_);
	  }
	  else 
	    ds_array<S>::strided_copy(x.dx_, x.stride_, dx_, stride_, sz_);
	}
	else 
	  ds_array<S>::strided_copy(x.dx_, x.stride_, dx_, stride_, sz_);

	return *this; 
      } 

      //! Returns number of derivative components
      int size() const { return sz_;}

      //! Returns array length
      int length() const { return len_; }

      //! Resize the derivative array to sz
      /*!
       * Note:  This does not necessarily preserve derivative components.
       */
      void resize(int sz) { 
	if (sz > len_) {
	  if (!owns_mem)
	      throw "Can\'t resize beyond original size when memory isn't owned!";
	  if (len_ != 0)
	    ds_array<S>::destroy_and_release(dx_, len_);
	  len_ = sz;
	  dx_ = ds_array<S>::get_and_fill(len_);
	}
	sz_ = sz;
      }

      //! Expand derivative array to size sz
      /*!
       * This method preserves any existing derivative components and
       * sets any that are added to zero.
       */
      void expand(int sz) {
        if (sz > len_) {
          if (!owns_mem)
	      throw "Can\'t resize beyond original size when memory isn't owned!";
          S* dx_new = ds_array<S>::get_and_fill(sz);
          ds_array<S>::copy(dx_, dx_new, sz_);
          if (len_ > 0)
            ds_array<S>::destroy_and_release(dx_, len_);
          dx_ = dx_new;
          len_ = sz;
        }
        else if (sz > sz_) 
          ds_array<S>::strided_zero(dx_+stride_*sz_, stride_, sz-sz_);
        sz_ = sz;
      }

      //! Zero out derivative array
      void zero() { 
	ds_array<S>::strided_zero(dx_, stride_, sz_);
      }

      //! Set value/derivative array memory
      void setMemory(int sz, T* x, S* dx_p, int stride) {

	// Destroy old memory
	if (owns_mem) {
	  if (len_ != 0)
	    ds_array<S>::destroy_and_release(dx_, len_);
	}

	// Set new values
	owns_mem = false;
	sz_ = sz;
	len_ = sz;
	stride_ = stride;
	val_ = x;
	dx_ = dx_p;
      }

      //! Returns value
      const T& val() const { return *val_; }

      //! Returns value
      T& val() { return *val_; }

      //! Returns derivative array
      const S* dx() const { return dx_;}

      //! Returns derivative component \c i with bounds checking
      S dx(int i) const { return sz_ ? dx_[i*stride_] : T(0.); }
    
      //! Returns derivative component \c i without bounds checking
      S& fastAccessDx(int i) { return dx_[i*stride_];}

      //! Returns derivative component \c i without bounds checking
      const S& fastAccessDx(int i) const { return dx_[i*stride_];}

    private:

      T v_;

    private:

      //! Do we own the val/dx storage
      bool owns_mem;

      //! Derivative array size
      int sz_;

      //! Derivative array length
      int len_;

      //! Derivative array stride
      int stride_;

      //! Value
      T* val_;

      //! Derivative array
      S* dx_;

    }; // class VectorDynamicStorage

  } // namespace Fad

} // namespace Sacado

#endif // SACADO_FAD_VECTORDYNAMICSTORAGE_HPP
