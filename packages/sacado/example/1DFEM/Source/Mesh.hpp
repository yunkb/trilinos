// $Id$ 
// $Source$ 
// @HEADER
// ***********************************************************************
// 
//                           Sacado Package
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
// Questions? Contact David M. Gay (dmgay@sandia.gov) or Eric T. Phipps
// (etphipp@sandia.gov).
// 
// ***********************************************************************
// @HEADER

#ifndef MESH_HPP
#define MESH_HPP

#include <list>

#include "Teuchos_RefCountPtr.hpp"

#include "AbstractElement.hpp"

class Mesh {
public:

  typedef std::list< Teuchos::RefCountPtr<AbstractElement> > MeshList;

  typedef MeshList::const_iterator const_iterator;

  typedef MeshList::iterator iterator;

  //! Constructor 
  Mesh();

  //! Destructor
  ~Mesh();

  //! Add a new element
  void addElement(const Teuchos::RefCountPtr<AbstractElement>& element);

  //! Return number of elements
  unsigned int numElements() const;

  //! First element
  iterator begin();

  //! First element
  const_iterator begin() const;

   //! Last element
  iterator end();

  //! Last element
  const_iterator end() const;

private:

  //! Private to prohibit copying
  Mesh(const Mesh&);

  //! Private to prohibit copying
  Mesh& operator=(const Mesh&);

protected:

  //! List of elements
  MeshList elements;

};

#endif // MESH_HPP
