/*@HEADER
// ***********************************************************************
// 
//       Ifpack: Object-Oriented Algebraic Preconditioner Package
//                 Copyright (2002) Sandia Corporation
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
//@HEADER
*/

#ifndef _IFP_BRELAX_H_
#define _IFP_BRELAX_H_

#include "ifp_GlobalPrecon.h"

class ifp_LocalMat;
class ifp_BlockMat;

class ifp_None : public ifp_GlobalPrecon
{
public:
    void setup(const ifp_BlockMat&) {};
    void apply (int nr, int nc, const double *u, int ldu, double *v, int ldv);
};

class ifp_BJacobi : public ifp_GlobalPrecon
{
private:
    const ifp_BlockMat *Ap;
    ifp_LocalMat **diag;  // inverse or factors of diagonal blocks

public:
    ifp_BJacobi();
   ~ifp_BJacobi();
 
    void setup(const ifp_BlockMat& A);
    void apply (int, int, const double *, int, double *, int);
    double condest();
};

class ifp_BSOR_Base : public ifp_GlobalPrecon
{
protected:
    const ifp_BlockMat *Ap;
    ifp_LocalMat **diag;  // inverse or factors of diagonal blocks
    int    *idiag;

    double omega_;
    int iterations_;

public:
    ifp_BSOR_Base();
    virtual ~ifp_BSOR_Base();
 
    double& omega() {return omega_;}
    int& iterations() {return iterations_;}

    void setup(const ifp_BlockMat& A, double omega = 1.0, int iterations = 1);
};

class ifp_BSOR : public ifp_BSOR_Base
{
public:
    ifp_BSOR() {};
   ~ifp_BSOR() {};
    void apply (int, int, const double *, int, double *, int);
};

class ifp_BSSOR : public ifp_BSOR_Base
{
public:
    ifp_BSSOR() {};
   ~ifp_BSSOR() {};
    void apply (int, int, const double *, int, double *, int);
};

#endif // _IFP_BRELAX_H_
