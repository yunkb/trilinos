
#ifndef _ZOLTAN2_PARTITIONINGPROBLEM_HPP_
#define _ZOLTAN2_PARTITIONINGPROBLEM_HPP_

#include <Zoltan2_Problem.hpp>
#include <Zoltan2_PartitioningAlgorithms.hpp>
#include <Zoltan2_PartitioningSolution.hpp>

#include <Zoltan2_GraphModel.hpp>
#ifdef HAVE_OVIS
#include <ovis.h>
#endif

/*! \file Zoltan2_PartitioningProblem.hpp

  This file contains the PartitioningProblem class, which derives from 
  the Problem class.
*/

namespace Zoltan2{

////////////////////////////////////////////////////////////////////////
template<typename Adapter>
class PartitioningProblem : public Problem<Adapter>
{
public:

  typedef typename Adapter::gid_t gid_t;
  typedef typename Adapter::lid_t lid_t;
  typedef typename Adapter::lno_t lno_t;

  // Destructor
  virtual ~PartitioningProblem() {};

#if 0  // KDDKDD Don't know how to use shortcut with Adapter template
  //! Constructor with Tpetra Matrix interface.
  PartitioningProblem(Tpetra::CrsMatrix<Scalar,LNO,GNO,Node> &A,
    ParameterList &p
  ) : Problem<Adapter>(A, p) 
  {
    HELLO;
    createPartitioningProblem();
  }
#endif

  //! Constructor with InputAdapter Interface
  PartitioningProblem(Adapter *A, Teuchos::ParameterList *p) 
                      : Problem<Adapter>(A, p) 
  {
    HELLO;
    createPartitioningProblem();
  };

  // Other methods
  //   LRIESEN - Do we restate virtual in the concrete class?  I
  //    don't think I've seen this style before.
  virtual void solve();

  PartitioningSolution<gid_t, lid_t, lno_t> &getSolution() {
    return *(solution_.getRawPtr());
  };

private:
  void createPartitioningProblem();

  RCP<PartitioningSolution<gid_t, lid_t, lno_t> > solution_;

};

////////////////////////////////////////////////////////////////////////
template <typename Adapter>
void PartitioningProblem<Adapter>::solve()
{
  HELLO;

  typedef typename Adapter::gid_t gid_t;
  typedef typename Adapter::lid_t lid_t;
  typedef typename Adapter::gno_t gno_t;
  typedef typename Adapter::lno_t lno_t;

  // Create the solution.
  // TODO:  For now, assume nParts = nProcessors. Should read from params.
  size_t nParts = this->comm_->getSize();  // TODO read from params later.

  try {

    // Determine which algorithm to use based on defaults and parameters.

    // For now, assuming Scotch graph partitioning.
    size_t nVtx = this->graphModel_->getLocalNumVertices();
    this->solution_ = rcp(new PartitioningSolution<gid_t,lid_t,lno_t>(nParts, nVtx, 0));

    AlgPTScotch<Adapter>(nParts, this->graphModel_, this->solution_, this->params_,
                         this->comm_, this->env_);
  }
  Z2_FORWARD_EXCEPTIONS;
}

////////////////////////////////////////////////////////////////////////
//! createPartitioningProblem 
//  Method with common functionality for creating a PartitioningProblem.
//  Individual constructors do appropriate conversions of input, etc.
//  This method does everything that all constructors must do.

template <typename Adapter>
void PartitioningProblem<Adapter>::createPartitioningProblem()
{
  HELLO;
//  cout << __func__ << " input adapter type " 
//       << this->inputAdapter_->inputAdapterType() << " " 
//       << this->inputAdapter_->inputAdapterName() << endl;

#ifdef HAVE_OVIS
  ovis_enabled(this->comm_->getRank());
#endif

  // Determine which parameters are relevant here.
  // For now, assume parameters similar to Zoltan:
  //   MODEL = graph, hypergraph, geometric, ids
  //   APPROACH = partition, repartition
  //   ALGORITHM = metis, parmetis, scotch, ptscotch, patoh, 
  //               phg, rcb, rib, hsfc, block, cyclic, random
  // TODO: I will need help from Lee Ann understanding how to use the parameter
  // functionality in Zoltan2.  For now, I will set a few parameters and
  // continue computing.
  ModelType modelType = GraphModelType;

  // Select Model based on parameters and InputAdapter type
  switch (modelType) {

  case GraphModelType:

    this->graphModel_ = RCP<GraphModel<Adapter> > 
                        (new GraphModel<Adapter>(this->inputAdapter_,
                                                 this->comm_, this->env_,
                                                 false, true));
    break;

  case HypergraphModelType:
  case GeometryModelType:
  case IdModelType:
    cout << __func__ << " Model type " << modelType << " not yet supported." 
         << endl;
    break;

  default:
    cout << __func__ << " Invalid model" << modelType << endl;
    break;
  }
}

}
#endif
