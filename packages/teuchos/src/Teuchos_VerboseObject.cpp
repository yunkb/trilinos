// @HEADER
// ***********************************************************************
// 
//                    Teuchos: Common Tools Package
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

#include "Teuchos_VerboseObject.hpp"

namespace Teuchos {

// Private static data members

RefCountPtr<FancyOStream>& VerboseObjectBase::privateDefaultOStream()
{
  static RefCountPtr<FancyOStream> defaultOStream = rcp(new FancyOStream(rcp(&std::cout,false)));
  return defaultOStream;
}

// Public static member functions

void VerboseObjectBase::setDefaultOStream( const RefCountPtr<FancyOStream> &defaultOStream )
{
  privateDefaultOStream() = defaultOStream;
}

RefCountPtr<FancyOStream>
VerboseObjectBase::getDefaultOStream()
{
  return privateDefaultOStream();
}

// Constructors/Initializers

VerboseObjectBase::VerboseObjectBase(
  const RefCountPtr<FancyOStream>   &oStream
  )
{
  this->initializeVerboseObjectBase(oStream);
}

void VerboseObjectBase::initializeVerboseObjectBase(
  const RefCountPtr<FancyOStream>   &oStream
  )
{
  thisOStream_ = oStream;
}

VerboseObjectBase& VerboseObjectBase::setOStream(const RefCountPtr<FancyOStream> &oStream)
{
  thisOStream_ = oStream;
  return *this;
}

VerboseObjectBase& VerboseObjectBase::setLinePrefix(const std::string &linePrefix)
{
  thisLinePrefix_ = linePrefix;
  return *this;
}

// Query functions

RefCountPtr<FancyOStream>
VerboseObjectBase::getOStream() const
{
  if(!thisOStream_.get())
    return getDefaultOStream();
  return thisOStream_;
}

std::string VerboseObjectBase::getLinePrefix() const
{
  return thisLinePrefix_;
}

// Utility functions

OSTab VerboseObjectBase::getOSTab(const int tabs,const std::string &linePrefix) const
{
  return OSTab( this->getOStream(), tabs, linePrefix.length() ? linePrefix : this->getLinePrefix() );
}

} // namespace Teuchos
