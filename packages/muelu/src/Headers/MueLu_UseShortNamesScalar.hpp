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
// New definition of types using the types Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps of the current context.

#include <Xpetra_UseShortNamesScalar.hpp>

#ifdef MUELU_HIERARCHY_SHORT
typedef MueLu::Hierarchy<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> Hierarchy;
#endif

#ifdef MUELU_HIERARCHYMANAGER_SHORT
typedef MueLu::HierarchyManager<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> HierarchyManager;
#endif

#ifdef MUELU_PARAMETERLISTINTERPRETER_SHORT
typedef MueLu::ParameterListInterpreter<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> ParameterListInterpreter;
#endif

#ifdef MUELU_FACTORYFACTORY_SHORT
typedef MueLu::FactoryFactory<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> FactoryFactory;
#endif

#ifdef MUELU_SAPFACTORY_SHORT
typedef MueLu::SaPFactory<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> SaPFactory;
#endif

#ifdef MUELU_PGPFACTORY_SHORT
typedef MueLu::PgPFactory<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> PgPFactory;
#endif

#ifdef MUELU_GENERICRFACTORY_SHORT
typedef MueLu::GenericRFactory<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> GenericRFactory;
#endif

#ifdef MUELU_TRANSPFACTORY_SHORT
typedef MueLu::TransPFactory<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> TransPFactory;
#endif

#ifdef MUELU_RAPFACTORY_SHORT
typedef MueLu::RAPFactory<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> RAPFactory;
#endif

#ifdef MUELU_SMOOTHERPROTOTYPE_SHORT
typedef MueLu::SmootherPrototype<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> SmootherPrototype;
#endif

#ifdef MUELU_FAKESMOOTHERPROTOTYPE_SHORT
typedef MueLu::FakeSmootherPrototype<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> FakeSmootherPrototype;
#endif

#ifdef MUELU_SMOOTHERBASE_SHORT
typedef MueLu::SmootherBase<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> SmootherBase;
#endif

#ifdef MUELU_SMOOTHERFACTORY_SHORT
typedef MueLu::SmootherFactory<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> SmootherFactory;
#endif

#ifdef MUELU_TENTATIVEPFACTORY_SHORT
typedef MueLu::TentativePFactory<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> TentativePFactory;
#endif

#ifdef MUELU_SMOOTHER_SHORT
typedef MueLu::Smoother<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> Smoother;
#endif

#ifdef MUELU_UTILS_SHORT
typedef MueLu::Utils<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> Utils;
#endif

#ifdef MUELU_GAUSSSEIDELSMOOTHER_SHORT
typedef MueLu::GaussSeidelSmoother<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> GaussSeidelSmoother;
#endif

#ifdef MUELU_IFPACK2SMOOTHER_SHORT
typedef MueLu::Ifpack2Smoother<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> Ifpack2Smoother;
#endif

#ifdef MUELU_AMESOS2SMOOTHER_SHORT
typedef MueLu::Amesos2Smoother<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> Amesos2Smoother;
#endif

#ifdef MUELU_DIRECTSOLVER_SHORT
typedef MueLu::DirectSolver<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> DirectSolver;
#endif

#ifdef MUELU_TRILINOSSMOOTHER_SHORT
typedef MueLu::TrilinosSmoother<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> TrilinosSmoother;
#endif

#ifdef MUELU_MERGEDSMOOTHER_SHORT
typedef MueLu::MergedSmoother<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> MergedSmoother;
#endif

#ifdef MUELU_COALESCEDROPFACTORY_SHORT
typedef MueLu::CoalesceDropFactory<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> CoalesceDropFactory;
#endif

#ifdef MUELU_PREDROPFUNCTIONBASECLASS_SHORT
typedef MueLu::PreDropFunctionBaseClass<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> PreDropFunctionBaseClass;
#endif

#ifdef MUELU_PREDROPFUNCTIONCONSTVAL_SHORT
typedef MueLu::PreDropFunctionConstVal<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> PreDropFunctionConstVal;
#endif

#ifdef MUELU_NULLSPACEFACTORY_SHORT
typedef MueLu::NullspaceFactory<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> NullspaceFactory;
#endif

#ifdef MUELU_FACTORYMANAGER_SHORT
typedef MueLu::FactoryManager<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> FactoryManager;
#endif

#ifdef MUELU_THRESHOLDAFILTERFACTORY_SHORT
typedef MueLu::ThresholdAFilterFactory<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> ThresholdAFilterFactory;
#endif

#ifdef MUELU_SEGREGATIONAFILTERFACTORY_SHORT
typedef MueLu::SegregationAFilterFactory<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> SegregationAFilterFactory;
#endif

#ifdef MUELU_SEGREGATIONATRANSFERFACTORY_SHORT
typedef MueLu::SegregationATransferFactory<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> SegregationATransferFactory;
#endif

#ifdef MUELU_REPARTITIONFACTORY_SHORT
typedef MueLu::RepartitionFactory<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> RepartitionFactory;
#endif

#ifdef MUELU_MLPARAMETERLISTINTERPRETER_SHORT
typedef MueLu::MLParameterListInterpreter<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> MLParameterListInterpreter;
#endif

#ifdef MUELU_BLOCKEDPFACTORY_SHORT
typedef MueLu::BlockedPFactory<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> BlockedPFactory;
#endif

#ifdef MUELU_SUBBLOCKAFACTORY_SHORT
typedef MueLu::SubBlockAFactory<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> SubBlockAFactory;
#endif

#ifdef MUELU_SCHURCOMPLEMENTFACTORY_SHORT
typedef MueLu::SchurComplementFactory<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> SchurComplementFactory;
#endif

#ifdef MUELU_BLOCKEDGAUSSSEIDELSMOOTHER_SHORT
typedef MueLu::BlockedGaussSeidelSmoother<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> BlockedGaussSeidelSmoother;
#endif

#ifdef MUELU_BRAESSSARAZINSMOOTHER_SHORT
typedef MueLu::BraessSarazinSmoother<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> BraessSarazinSmoother;
#endif

#ifdef MUELU_AGGREGATIONEXPORTFACTORY_SHORT
typedef MueLu::AggregationExportFactory<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> AggregationExportFactory;
#endif

#ifdef MUELU_MULTIVECTORTRANSFERFACTORY_SHORT
typedef MueLu::MultiVectorTransferFactory<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> MultiVectorTransferFactory;
#endif

#ifdef MUELU_PERMUTEDTRANSFERFACTORY_SHORT
typedef MueLu::PermutedTransferFactory<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> PermutedTransferFactory;
#endif

#ifdef MUELU_AMALGAMATIONFACTORY_SHORT
typedef MueLu::AmalgamationFactory<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> AmalgamationFactory;
#endif

#ifdef MUELU_AGGSTATTRANSFERFACTORY_SHORT
typedef MueLu::AggStatTransferFactory<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> AggStatTransferFactory;
#endif

#ifdef MUELU_UTILITIES_SHORT
typedef MueLu::Utils<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> Utils;
typedef MueLu::Utils2<Scalar,LocalOrdinal,GlobalOrdinal,Node,LocalMatOps> Utils2;
#endif