// @HEADER
//
// ***********************************************************************
//
//        MueLu: A package for multigrid based preconditioning
//                  Copyright 2012 Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact
//                    Jeremie Gaidamour (jngaida@sandia.gov)
//                    Jonathan Hu       (jhu@sandia.gov)
//                    Ray Tuminaro      (rstumin@sandia.gov)
//
// ***********************************************************************
//
// @HEADER
#ifndef MUELU_REBALANCEBLOCKRESTRICTIONFACTORY_DEF_HPP_
#define MUELU_REBALANCEBLOCKRESTRICTIONFACTORY_DEF_HPP_

#include <Teuchos_Tuple.hpp>

#include "Xpetra_Vector.hpp"
#include "Xpetra_VectorFactory.hpp"
#include "Xpetra_MultiVector.hpp"
#include "Xpetra_MultiVectorFactory.hpp"
#include <Xpetra_Matrix.hpp>
#include <Xpetra_BlockedCrsMatrix.hpp>
#include <Xpetra_MapFactory.hpp>
#include <Xpetra_MapExtractor.hpp>
#include <Xpetra_MapExtractorFactory.hpp>
#include <Xpetra_MatrixFactory.hpp>
#include <Xpetra_Import.hpp>
#include <Xpetra_ImportFactory.hpp>

#include "MueLu_RebalanceBlockRestrictionFactory_decl.hpp"
#include "MueLu_Utilities.hpp"
#include "MueLu_HierarchyHelpers.hpp"
#include "MueLu_FactoryManagerBase.hpp"

#include "MueLu_Level.hpp"
#include "MueLu_Monitor.hpp"

namespace MueLu {

template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
RCP<const ParameterList> RebalanceBlockRestrictionFactory<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::GetValidParameterList(const ParameterList& paramList) const {
  RCP<ParameterList> validParamList = rcp(new ParameterList());

  validParamList->set< RCP<const FactoryBase> >("R",              Teuchos::null, "Factory of the restriction operator that need to be rebalanced (only used if type=Restriction)");

  return validParamList;
}

template <class Scalar,class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
void RebalanceBlockRestrictionFactory<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::AddFactoryManager(RCP<const FactoryManagerBase> FactManager) {
  FactManager_.push_back(FactManager);
}

template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
void RebalanceBlockRestrictionFactory<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::DeclareInput(Level &fineLevel, Level &coarseLevel) const {
  const Teuchos::ParameterList & pL = GetParameterList();

  Input(coarseLevel, "R");

  std::vector<Teuchos::RCP<const FactoryManagerBase> >::const_iterator it;
  for(it = FactManager_.begin(); it!=FactManager_.end(); ++it) {
    SetFactoryManager fineSFM  (rcpFromRef(fineLevel),   *it);
    SetFactoryManager coarseSFM(rcpFromRef(coarseLevel), *it);

    coarseLevel.DeclareInput("Importer",(*it)->GetFactory("Importer").get(), this);
    coarseLevel.DeclareInput("Nullspace",(*it)->GetFactory("Nullspace").get(), this);
  }
}

template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
void RebalanceBlockRestrictionFactory<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::Build(Level &fineLevel, Level &coarseLevel) const {
  FactoryMonitor m(*this, "Build", coarseLevel);
  const Teuchos::ParameterList & pL = GetParameterList();

  RCP<Teuchos::FancyOStream> out = Teuchos::fancyOStream(Teuchos::rcpFromRef(std::cout));

  Teuchos::RCP<Matrix> originalTransferOp = Teuchos::null;
  originalTransferOp = Get< RCP<Matrix> >(coarseLevel, "R");

  RCP<Xpetra::BlockedCrsMatrix<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps> > bOriginalTransferOp = Teuchos::rcp_dynamic_cast<Xpetra::BlockedCrsMatrix<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps> >(originalTransferOp);
  TEUCHOS_TEST_FOR_EXCEPTION(bOriginalTransferOp==Teuchos::null, Exceptions::BadCast, "MueLu::RebalanceBlockTransferFactory::Build: input matrix P or R is not of type BlockedCrsMatrix! error.");

  // plausibility check
  TEUCHOS_TEST_FOR_EXCEPTION(bOriginalTransferOp->Rows() != 2,Exceptions::RuntimeError, "MueLu::RebalanceBlockTransferFactory::Build: number of block rows of transfer operator is not equal 2. error.");
  TEUCHOS_TEST_FOR_EXCEPTION(bOriginalTransferOp->Cols() != 2,Exceptions::RuntimeError, "MueLu::RebalanceBlockTransferFactory::Build: number of block columns of transfer operator is not equal 2. error.");

  // rebuild rebalanced blocked P operator
  std::vector<GO> fullRangeMapVector;
  std::vector<GO> fullDomainMapVector;
  std::vector<RCP<const Map> > subBlockRRangeMaps;
  std::vector<RCP<const Map> > subBlockRDomainMaps;
  subBlockRRangeMaps.reserve(bOriginalTransferOp->Rows());       // reserve size for block P operators
  subBlockRDomainMaps.reserve(bOriginalTransferOp->Cols());       // reserve size for block P operators

  std::vector<Teuchos::RCP<Matrix> > subBlockRebR;
  subBlockRebR.reserve(bOriginalTransferOp->Cols());

  int curBlockId = 0;
  Teuchos::RCP<const Import> rebalanceImporter = Teuchos::null;
  std::vector<Teuchos::RCP<const FactoryManagerBase> >::const_iterator it;
  for (it = FactManager_.begin(); it != FactManager_.end(); ++it) {
    // begin SubFactoryManager environment
    SetFactoryManager fineSFM  (rcpFromRef(fineLevel),   *it);
    SetFactoryManager coarseSFM(rcpFromRef(coarseLevel), *it);

    rebalanceImporter = coarseLevel.Get<Teuchos::RCP<const Import> >("Importer", (*it)->GetFactory("Importer").get());

    // extract matrix block
    Teuchos::RCP<CrsMatrix> Rmii = bOriginalTransferOp->getMatrix(curBlockId, curBlockId);
    Teuchos::RCP<CrsMatrixWrap> Rwii = Teuchos::rcp(new CrsMatrixWrap(Rmii));
    Teuchos::RCP<Matrix> Rii = Teuchos::rcp_dynamic_cast<Matrix>(Rwii);

    Teuchos::RCP<Matrix> rebRii;
    if(rebalanceImporter != Teuchos::null) {
      std::stringstream ss; ss << "Rebalancing restriction block R(" << curBlockId << "," << curBlockId << ")";
      SubFactoryMonitor m1(*this, ss.str(), coarseLevel);
      {
        SubFactoryMonitor subM(*this, "Rebalancing restriction -- fusedImport", coarseLevel);
        // Note: The 3rd argument says to use originalR's domain map.

        RCP<Map> dummy;
        rebRii = MatrixFactory::Build(Rii,*rebalanceImporter,dummy,rebalanceImporter->getTargetMap());
      }

      RCP<ParameterList> params = rcp(new ParameterList());
      params->set("printLoadBalancingInfo", true);
      std::stringstream ss2; ss2 << "R(" << curBlockId << "," << curBlockId << ") rebalanced:";
      GetOStream(Statistics0, 0) << Utils::PrintMatrixInfo(*rebRii, ss2.str(), params);
    } else {
      rebRii = Rii;
      RCP<ParameterList> params = rcp(new ParameterList());
      params->set("printLoadBalancingInfo", true);
      std::stringstream ss2; ss2 << "R(" << curBlockId << "," << curBlockId << ") not rebalanced:";
      GetOStream(Statistics0, 0) << Utils::PrintMatrixInfo(*rebRii, ss2.str(), params);
    }

    // store rebalanced subblock
    subBlockRebR.push_back(rebRii);

    // append strided row map (= range map) to list of range maps.
    Teuchos::RCP<const Map> rangeMapii = rebRii->getRangeMap();
    subBlockRRangeMaps.push_back(rangeMapii);
    Teuchos::ArrayView< const GlobalOrdinal > nodeRangeMapii = rangeMapii->getNodeElementList();
    fullRangeMapVector.insert(fullRangeMapVector.end(), nodeRangeMapii.begin(), nodeRangeMapii.end());
    sort(fullRangeMapVector.begin(), fullRangeMapVector.end());

    // append strided col map (= domain map) to list of range maps.
    Teuchos::RCP<const Map> domainMapii = rebRii->getDomainMap();
    subBlockRDomainMaps.push_back(domainMapii);
    Teuchos::ArrayView< const GlobalOrdinal > nodeDomainMapii = domainMapii->getNodeElementList();
    fullDomainMapVector.insert(fullDomainMapVector.end(), nodeDomainMapii.begin(), nodeDomainMapii.end());
    sort(fullDomainMapVector.begin(), fullDomainMapVector.end());

    ////////////////////////////////////////////////////////////

    // rebalance null space
    if(rebalanceImporter != Teuchos::null)
    { // rebalance null space
      std::stringstream ss2; ss2 << "Rebalancing nullspace block(" << curBlockId << "," << curBlockId << ")";
      SubFactoryMonitor subM(*this, ss2.str(), coarseLevel);

      RCP<MultiVector> nullspace = coarseLevel.Get<RCP<MultiVector> >("Nullspace", (*it)->GetFactory("Nullspace").get());
      RCP<MultiVector> permutedNullspace = MultiVectorFactory::Build(rebalanceImporter->getTargetMap(), nullspace->getNumVectors());
      permutedNullspace->doImport(*nullspace, *rebalanceImporter, Xpetra::INSERT);

      // TODO think about this
      //if (pL.get<bool>("useSubcomm") == true) // TODO either useSubcomm is enabled everywhere or nowhere
      //permutedNullspace->replaceMap(permutedNullspace->getMap()->removeEmptyProcesses());

      coarseLevel.Set<RCP<MultiVector> >("Nullspace", permutedNullspace, (*it)->GetFactory("Nullspace").get());

    } // end rebalance null space
    else { // do nothing
      RCP<MultiVector> nullspace = coarseLevel.Get<RCP<MultiVector> >("Nullspace", (*it)->GetFactory("Nullspace").get());
      coarseLevel.Set<RCP<MultiVector> >("Nullspace", nullspace, (*it)->GetFactory("Nullspace").get());
    }

    ////////////////////////////////////////////////////////////

    curBlockId++;
  } // end for loop

  // extract map index base from maps of blocked P
  GO rangeIndexBase = originalTransferOp->getRangeMap()->getIndexBase();
  GO domainIndexBase= originalTransferOp->getDomainMap()->getIndexBase();

  // check this
  RCP<const Xpetra::MapExtractor<Scalar, LocalOrdinal, GlobalOrdinal, Node> > rangeRMapExtractor = bOriginalTransferOp->getRangeMapExtractor(); // original map extractor
  Teuchos::ArrayView<GO> fullRangeMapGIDs(&fullRangeMapVector[0],fullRangeMapVector.size());
  Teuchos::RCP<const StridedMap> stridedRgFullMap = Teuchos::rcp_dynamic_cast<const StridedMap>(rangeRMapExtractor->getFullMap());
  std::vector<size_t> stridedData = stridedRgFullMap->getStridingData();

  Teuchos::RCP<const StridedMap > fullRangeMap =
      StridedMapFactory::Build(
          originalTransferOp->getRangeMap()->lib(),
          Teuchos::OrdinalTraits<Xpetra::global_size_t>::invalid(),
          fullRangeMapGIDs,
          rangeIndexBase,
          stridedData,
          originalTransferOp->getRangeMap()->getComm(),
          stridedRgFullMap->getStridedBlockId(),
          stridedRgFullMap->getOffset());

  RCP<const Xpetra::MapExtractor<Scalar, LocalOrdinal, GlobalOrdinal, Node> > domainAMapExtractor = bOriginalTransferOp->getDomainMapExtractor();
  Teuchos::ArrayView<GO> fullDomainMapGIDs(&fullDomainMapVector[0],fullDomainMapVector.size());
  Teuchos::RCP<const StridedMap> stridedDoFullMap = Teuchos::rcp_dynamic_cast<const StridedMap>(domainAMapExtractor->getFullMap());
  std::vector<size_t> stridedData2 = stridedDoFullMap->getStridingData();

  Teuchos::RCP<const StridedMap > fullDomainMap =
      StridedMapFactory::Build(
          originalTransferOp->getDomainMap()->lib(),
          Teuchos::OrdinalTraits<Xpetra::global_size_t>::invalid(),
          fullDomainMapGIDs,
          domainIndexBase,
          stridedData2,
          originalTransferOp->getDomainMap()->getComm(),
          stridedDoFullMap->getStridedBlockId(),
          stridedDoFullMap->getOffset());

  // build map extractors
  Teuchos::RCP<const Xpetra::MapExtractor<Scalar, LocalOrdinal, GlobalOrdinal, Node> > rangeMapExtractor  =
      Xpetra::MapExtractorFactory<Scalar, LocalOrdinal, GlobalOrdinal, Node>::Build(fullRangeMap,  subBlockRRangeMaps);
  Teuchos::RCP<const Xpetra::MapExtractor<Scalar, LocalOrdinal, GlobalOrdinal, Node> > domainMapExtractor =
      Xpetra::MapExtractorFactory<Scalar, LocalOrdinal, GlobalOrdinal, Node>::Build(fullDomainMap, subBlockRDomainMaps);

  Teuchos::RCP<BlockedCrsMatrix> bRebR = Teuchos::rcp(new BlockedCrsMatrix(rangeMapExtractor,domainMapExtractor,10));
  for(size_t i = 0; i<subBlockRRangeMaps.size(); i++) {
    Teuchos::RCP<const CrsMatrixWrap> crsOpii = Teuchos::rcp_dynamic_cast<const CrsMatrixWrap>(subBlockRebR[i]);
    Teuchos::RCP<CrsMatrix> crsMatii = crsOpii->getCrsMatrix();
    bRebR->setMatrix(i,i,crsMatii);
  }

  bRebR->fillComplete();

  Set(coarseLevel, "R", Teuchos::rcp_dynamic_cast<Matrix>(bRebR)); // do nothing  // TODO remove this!

} // Build

} // namespace MueLu

#endif /* MUELU_REBALANCEBLOCKRESTRICTIONFACTORY_DEF_HPP_ */
