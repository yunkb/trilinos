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
/*
 * SundancePeanoMesh.cpp
 *
 *  Created on: Sep 8, 2009
 *      Author: benk
 */

#include "SundancePeanoMesh.hpp"

#include "SundanceMeshType.hpp"
#include "SundanceCellJacobianBatch.hpp"
#include "SundanceMaximalCofacetBatch.hpp"
#include "SundanceMeshSource.hpp"
#include "SundanceDebug.hpp"
#include "SundanceOut.hpp"
#include "Teuchos_MPIContainerComm.hpp"
#include "Teuchos_Time.hpp"
#include "Teuchos_TimeMonitor.hpp"
#include "SundanceObjectWithVerbosity.hpp"
#include "SundanceCollectiveExceptionCheck.hpp"

using namespace SundanceStdMesh::Internal;
using namespace SundanceStdMesh;
using namespace Teuchos;
using namespace SundanceUtils;

//#define printf(msg)
//#define SUNDANCE_VERB_HIGH(msg) printf(msg);printf("\n");

Point PeanoMesh::returnPoint(0.0 , 0.0);

PeanoMesh::PeanoMesh(int dim, const MPIComm& comm)
: MeshBase(dim, comm),_dimension(dim), _comm(comm)
 ,_peanoMesh(NULL)
{
	_uniqueResolution = 1.0;
}

void PeanoMesh::createMesh(
                      double position_x,
			          double position_y,
			          double offset_x,
			          double offset_y,
			          double resolution
){
      double position[PEANO_DIMENSIONS];
      double offset[PEANO_DIMENSIONS];
      double res[PEANO_DIMENSIONS];

      // setting the values of the ctor argument
      position[0] = position_x; position[1] = position_y;
      offset[0] = offset_x;     offset[1] = offset_y;
      res[0] = resolution;      res[1] = resolution;


      // this is a 2D case
      // call the ctor for the Peano mesh
      SUNDANCE_VERB_LOW(" create Peano Mesh ... ");
      _dimension = PEANO_DIMENSIONS;
      // here we create the Peano grid
      _peanoMesh = new SundancePeanoInterface( position , offset , res );
      _uniqueResolution = _peanoMesh->returnUniqueResolution();

      SUNDANCE_VERB_LOW(" Peano Mesh created ... \n");
      _peanoMesh->plotVTK("Peano.vtk");
      SUNDANCE_VERB_LOW(" After Plot ... \n");
}

PeanoMesh::~PeanoMesh() {
	//delete _peanoMesh;
}


int PeanoMesh::numCells(int dim) const  {
	//printf("PeanoMesh::numCells(int dim):%d   dim:%d \n",_peanoMesh->numCells(dim),dim);
	return _peanoMesh->numCells(dim);
}

Point PeanoMesh::nodePosition(int i) const {
	//SUNDANCE_VERB_HIGH("nodePosition(int i)");
	//printf("PeanoMesh::nodePosition(int i)   i:%d \n", i);
	double* coords;
	coords = _peanoMesh->nodePositionView(i);
	PeanoMesh::returnPoint[0] = coords[0];
	PeanoMesh::returnPoint[1] = coords[1];
	// set the values
	if (PEANO_DIMENSIONS == 3){
		PeanoMesh::returnPoint[2] = coords[2];
	}
	return PeanoMesh::returnPoint;
}

const double* PeanoMesh::nodePositionView(int i) const {
	//printf("PeanoMesh::nodePositionView(int i)   i:%d \n", i);
	//SUNDANCE_VERB_HIGH("nodePosition(int i)");
	nodePosition(i);
	return &(PeanoMesh::returnPoint[0]);
}

void PeanoMesh::getJacobians(int cellDim, const Array<int>& cellLID,
                          CellJacobianBatch& jBatch) const
{
	  //printf("cellDim:%d  _uniqueResolution:%f ",cellDim, _uniqueResolution);
	  SUNDANCE_VERB_HIGH("getJacobians()");
	  TEST_FOR_EXCEPTION(cellDim < 0 || cellDim > spatialDim(), InternalError,
	    "cellDim=" << cellDim << " is not in expected range [0, " << spatialDim() << "]");
	  int nCells = cellLID.size();
	  jBatch.resize(cellLID.size(), spatialDim(), cellDim);
	  if (cellDim < spatialDim()) // they need the Jacobian of a lower dinemsional element
	  {
		  //printf("PeanoMesh::getJacobians() cellDim < spatialDim() \n");
		   for (int i=0; i<nCells; i++)
		    {
		     //printf("PeanoMesh::getJacobian() cellDim < spatialDim() cellDim:%d , ret:%f \n",cellDim , _uniqueResolution);
		      double* detJ = jBatch.detJ(i);
		      switch(cellDim)
		      {
		        case 0: *detJ = 1.0;
		          break;
		        case 1:
		          *detJ = _uniqueResolution; // the length of (each) edge
		        break;
		        case 2:
		          *detJ = _uniqueResolution*_uniqueResolution; //TODO: the are of the face (not twice the area as it is in the BSM!!)
		        break;
		        default:
		          TEST_FOR_EXCEPTION(true, InternalError, "impossible switch value "
		            "cellDim=" << cellDim << " in BasicSimplicialMesh::getJacobians()");
		      }
		    }
	  }else{ // they request the complete Jacoby matrix for this bunch of elements
		    //Array<double> J(cellDim*cellDim);
		    SUNDANCE_VERB_HIGH("cellDim == spatialDim()");
		    for (unsigned int i=0; i<cellLID.size(); i++)
		    {
			  //printf("PeanoMesh::getJacobian() cellDim == spatialDim() cellDim:%d , ret:%f \n",cellDim , _uniqueResolution);
		      double* J = jBatch.jVals(i);
		      switch(cellDim)
		      {
		        case 0:
		          J[0] = 1.0; // point
		          break;
		        case 1:
		          J[0] = _uniqueResolution;  // length in x direction of the line
		        break;
		        case 2:
		          J[0] = _uniqueResolution;
		          J[1] = 0.0;  J[2] = 0.0;   //
		          J[3] = _uniqueResolution; // the Jacobi of the quad,
		        break;
		        case 3:
		          J[0] = _uniqueResolution;
		          J[1] = 0.0; J[2] = 0.0; J[3] = 0.0;
		          J[4] = _uniqueResolution;
		          J[5] = 0.0; J[6] = 0.0; J[7] = 0.0;
		          J[8] = _uniqueResolution; // the Jacobi of the tet
		        break;
		        default:
		          TEST_FOR_EXCEPTION(true, InternalError, "impossible switch value "
		            "cellDim=" << cellDim
		            << " in BasicSimplicialMesh::getJacobians()");
		      }
		    }
	  }
}

void PeanoMesh::getCellDiameters(int cellDim, const Array<int>& cellLID,
                              Array<double>& cellDiameters) const {
	 TEST_FOR_EXCEPTION(cellDim < 0 || cellDim > spatialDim(), InternalError,
	    "cellDim=" << cellDim << " is not in expected range [0, " << spatialDim() << "]");
	 SUNDANCE_VERB_HIGH("getCellDiameters()");
	  cellDiameters.resize(cellLID.size());
	  if (cellDim < spatialDim())
	  {
		//printf("PeanoMesh::getCellDiameters(), cellDim < spatialDim() \n ");
	    for (unsigned int i=0; i<cellLID.size(); i++)
	    {
	      switch(cellDim)
	      {
	        case 0:
	          cellDiameters[i] = 1.0;
	          break;
	        case 1:
	          cellDiameters[i] = _uniqueResolution; //length of the edge
	        break;
	        case 2:
	          cellDiameters[i] = _uniqueResolution * _uniqueResolution; //area of the cell
	        break;
	        default:
	          TEST_FOR_EXCEPTION(true, InternalError, "impossible switch value "
	            "cellDim=" << cellDim << " in BasicSimplicialMesh::getCellDiameters()");
	      }
	    }
	  }
	  else
	  {
		//printf("PeanoMesh::getCellDiameters(), cellDim == spatialDim() \n ");
	    for (unsigned int i=0; i<cellLID.size(); i++)
	    {
	      switch(cellDim)
	      {
	        case 0:
	          cellDiameters[i] = 1.0;
	          break;
	        case 1: case 2: case 3:
	          cellDiameters[i] = _uniqueResolution;
	        break;
	        default:
	          TEST_FOR_EXCEPTION(true, InternalError, "impossible switch value "
	            "cellDim=" << cellDim
	            << " in BasicSimplicialMesh::getCellDiameters()");
	      }
	    }
	  }
}

void PeanoMesh::pushForward(int cellDim, const Array<int>& cellLID,
                         const Array<Point>& refQuadPts,
                         Array<Point>& physQuadPts) const {

	  //printf("PeanoMesh::pushForward cellDim:%d\n",cellDim);
	  TEST_FOR_EXCEPTION(cellDim < 0 || cellDim > spatialDim(), InternalError,
	    "cellDim=" << cellDim
	    << " is not in expected range [0, " << spatialDim()
	    << "]");

	  int nQuad = refQuadPts.size();

	  Array<double> J(cellDim*cellDim);

	  if (physQuadPts.size() > 0) physQuadPts.resize(0);
	  physQuadPts.reserve(cellLID.size() * refQuadPts.size());
	  for (unsigned int i=0; i<cellLID.size(); i++)
	  {
    	int tmp , tmp_index;
	    int lid = cellLID[i];
	    switch(cellDim)
	    {
	      case 0: // integrate one point
	         physQuadPts.append(nodePosition(lid));
	        break;
	      case 1:{ // integrate on one line
	    	 tmp_index = this->facetLID(cellDim,  lid , 0 , 0 , tmp );
	    	 Point pnt(nodePosition(tmp_index));
	         for (int q=0; q<nQuad; q++) {
	           physQuadPts.append(pnt + refQuadPts[q][0]*_uniqueResolution);
	         }
	      break;}
	      case 2:{
		     tmp_index = this->facetLID(cellDim,  lid , 0 , 0 , tmp );
		     Point pnt(nodePosition(tmp_index));
	         for (int q=0; q<nQuad; q++) {
	          	      physQuadPts.append( pnt
	           	        + Point(refQuadPts[q][0]*_uniqueResolution,refQuadPts[q][1]*_uniqueResolution));
	         }
	      break;}
	      case 3:{
			  tmp_index = this->facetLID(cellDim,  lid , 0 , 0 , tmp );
			  Point pnt(nodePosition(tmp_index));
	          for (int q=0; q<nQuad; q++) {
	          	      physQuadPts.append( pnt
	           	        + Point(_uniqueResolution*refQuadPts[q][0],_uniqueResolution*refQuadPts[q][1],
	           	        		_uniqueResolution*refQuadPts[q][2]));
	          }
	      break;}
	      default:
	        TEST_FOR_EXCEPTION(true, InternalError, "impossible switch value "
	          "in BasicSimplicialMesh::getJacobians()");
	    }
	  }
}

int PeanoMesh::ownerProcID(int cellDim, int cellLID) const  {
	 SUNDANCE_VERB_HIGH("ownerProcID()"); return 0; }


int PeanoMesh::numFacets(int cellDim, int cellLID,
                      int facetDim) const  {
	SUNDANCE_VERB_HIGH("numFacets()");
    return _peanoMesh->numFacets(cellDim, cellLID, facetDim);
}


int PeanoMesh::facetLID(int cellDim, int cellLID,
                     int facetDim, int facetIndex,
                     int& facetOrientation) const  {
    int LID;
    LID = _peanoMesh->facetLID( cellDim,cellLID, facetDim, facetIndex, facetOrientation);
  	//printf("PeanoMesh::facetLID  cellDim: %d , cellLID: %d , facetDim %d , facetIndex:%d  %d\n" , cellDim , cellLID , facetDim , facetIndex , LID );
	return LID;
}

void PeanoMesh::getFacetLIDs(int cellDim,
                          const Array<int>& cellLID,
                          int facetDim,
                          Array<int>& facetLID,
                          Array<int>& facetSign) const {
	  SUNDANCE_VERB_HIGH("getFacetLIDs()");
	  //printf("PeanoMesh::getFacetLIDs()  cellDim:%d  cellLID.size():%d  facetDim:%d\n" , cellDim, (int)cellLID.size() , facetDim);
      int LID = 0 , cLID , facetOrientation ;
      int ptr = 0;

      int nf = numFacets(cellDim, cellLID[0], facetDim);
      facetLID.resize(cellLID.size() * nf);
      facetSign.resize(cellLID.size() * nf);
	  // At this moment we just use the previous function
	  for (unsigned int i = 0 ; i < cellLID.size() ; i++){
		  cLID = cellLID[i];
	      for (int f=0; f<nf; f++, ptr++) {
	    	  // we use this->facetLID caz facetLID is already used as variable
			  LID = this->facetLID( cellDim, cLID, facetDim, f , facetOrientation);
			  //printf("LID:%d , cellDim:%d , cLID:%d , facetDim:%d , f:%d , facetOrientation:%d \n"
			  //	  ,LID , cellDim, cLID, facetDim, f , facetOrientation );
	          facetLID[ptr] = LID;
	          facetSign[ptr] = facetOrientation;
	      }
	  }
}

const int* PeanoMesh::elemZeroFacetView(int cellLID) const {
	return _peanoMesh->elemZeroFacetView(cellLID);
}

int PeanoMesh::numMaxCofacets(int cellDim, int cellLID) const  {
	  //SUNDANCE_VERB_HIGH("numMaxCofacets()");
      int coFacetCounter;
      coFacetCounter = _peanoMesh->numMaxCofacets( cellDim, cellLID);
	  //printf("numMaxCofacets:  cellDim:%d cellLID:%d ret:%d\n",cellDim, cellLID, coFacetCounter);
	  return coFacetCounter;
}

int PeanoMesh::maxCofacetLID(int cellDim, int cellLID,
                       int cofacetIndex,
                       int& facetIndex) const  {
	  int rtn;
	  rtn = _peanoMesh->maxCofacetLID(cellDim, cellLID, cofacetIndex, facetIndex);
	  //printf("maxCofacetLID() cellDim:%d,  cellLID:%d, cofacetIndex:%d , rtn:%d , facetIndex:%d\n",
		//	  cellDim,  cellLID, cofacetIndex , rtn , facetIndex);
	  return rtn;
}

void PeanoMesh::getMaxCofacetLIDs(const Array<int>& cellLIDs,
  MaximalCofacetBatch& cofacets) const {
    TEST_FOR_EXCEPTION(true, InternalError," PeanoMesh::getMaxCofacetLIDs() not implemented yet");
	//TODO: Implement this
}


void PeanoMesh::getCofacets(int cellDim, int cellLID,
                 int cofacetDim, Array<int>& cofacetLIDs) const {
    TEST_FOR_EXCEPTION(true, InternalError," PeanoMesh::getCofacets() not implemented yet");
	//TODO: Implement this
}


int PeanoMesh::mapGIDToLID(int cellDim, int globalIndex) const  {
	SUNDANCE_VERB_HIGH("mapGIDToLID()");
	// in the serial implementation GID = LID
	// in the parallel version this should be done differently
	return globalIndex;
}

bool PeanoMesh::hasGID(int cellDim, int globalIndex) const {
	SUNDANCE_VERB_HIGH("hasGID()");
	// since currently we have a serial implementation , this is always true
	// in the parallel version this function has to be implemented differetly
	return true;
}

int PeanoMesh::mapLIDToGID(int cellDim, int localIndex) const  {
	SUNDANCE_VERB_HIGH("mapLIDToGID()");
	// at the current stage we have only serial implementation,
	// parallel implementation will(should) come soon
	return localIndex;
}

CellType PeanoMesh::cellType(int cellDim) const  {
	//printf("cellType() cellDim:%d\n",cellDim);
	 switch(cellDim)
	  {
	    case 0:  return PointCell;
	    case 1:  return LineCell;
	    case 2:  return QuadCell;
	    case 3:  return BrickCell;
	    default:
	      return NullCell; // -Wall
	  }
}

int PeanoMesh::label(int cellDim, int cellLID) const {
   return _peanoMesh->label( cellDim, cellLID);
}

void PeanoMesh::getLabels(int cellDim, const Array<int>& cellLID,
		Array<int>& labels) const {
    int tmpIndex;
    SUNDANCE_VERB_HIGH("getLabels()");
    // resize the array
	labels.resize(cellLID.size());

    for (tmpIndex = 0 ; tmpIndex < (int)cellLID.size() ; tmpIndex++){
    	labels[tmpIndex] = _peanoMesh->label( cellDim, cellLID[tmpIndex]);
    }
}

Set<int> PeanoMesh::getAllLabelsForDimension(int cellDim) const {
	  Set<int>                 rtn;
	  int                      tmpIndex;
	  SUNDANCE_VERB_HIGH("getAllLabelsForDimension()");

	  for (tmpIndex = 0 ; tmpIndex < _peanoMesh->numCells(cellDim) ; tmpIndex++){
		  rtn.put( _peanoMesh->label( cellDim, tmpIndex) );
	  }
	  return rtn;
}

void PeanoMesh::getLIDsForLabel(int cellDim, int label, Array<int>& cellLIDs) const {
    int                      tmpIndex , tmpLabel;
	SUNDANCE_VERB_HIGH("getLIDsForLabel()");
    for (tmpIndex = 0 ; tmpIndex < _peanoMesh->numCells(cellDim) ; tmpIndex++){
    	tmpLabel = this->label( cellDim , tmpIndex);
    	if (tmpLabel == label) cellLIDs.append( tmpIndex );
    }
}

void PeanoMesh::setLabel(int cellDim, int cellLID, int label) {
	_peanoMesh->setLabel(cellDim, cellLID, label);
}


void PeanoMesh::assignIntermediateCellGIDs(int cellDim) {
	SUNDANCE_VERB_HIGH("assignIntermediateCellGIDs()");
	// in this method we could do synchronization between processors, not usede now
}


bool PeanoMesh::hasIntermediateGIDs(int dim) const {
	SUNDANCE_VERB_HIGH("hasIntermediateGIDs()");
	return true; // true means they have been synchronized ... not used now
}
