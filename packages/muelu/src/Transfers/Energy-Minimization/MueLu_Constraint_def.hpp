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
//                    Jonathan Hu       (jhu@sandia.gov)
//                    Andrey Prokopenko (aprokop@sandia.gov)
//                    Ray Tuminaro      (rstumin@sandia.gov)
//
// ***********************************************************************
//
// @HEADER
#ifndef MUELU_CONSTRAINT_DEF_HPP
#define MUELU_CONSTRAINT_DEF_HPP

#include <Teuchos_BLAS.hpp>
#include <Teuchos_LAPACK.hpp>
#include <Teuchos_SerialDenseVector.hpp>
#include <Teuchos_SerialDenseMatrix.hpp>
#include <Teuchos_SerialDenseHelpers.hpp>

#include <Xpetra_Import_fwd.hpp>
#include <Xpetra_ImportFactory.hpp>
#include <Xpetra_Map.hpp>
#include <Xpetra_Matrix.hpp>
#include <Xpetra_MultiVectorFactory.hpp>
#include <Xpetra_MultiVector.hpp>
#include <Xpetra_CrsGraph.hpp>

#include "MueLu_Constraint_decl.hpp"
#include "MueLu_Utilities.hpp"

namespace MueLu {

  template<class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
  void Constraint<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::Setup(const MultiVector& B, const MultiVector& Bc, RCP<const CrsGraph> Ppattern) {
    Ppattern_ = Ppattern;

    const RCP<const Map> uniqueMap    = Ppattern_->getDomainMap();
    const RCP<const Map> nonUniqueMap = Ppattern_->getColMap();
    RCP<const Import> importer = ImportFactory::Build(uniqueMap, nonUniqueMap);

    const size_t NSDim = Bc.getNumVectors();
    X_ = MultiVectorFactory::Build(nonUniqueMap, NSDim);
    X_->doImport(Bc, *importer, Xpetra::INSERT);

    size_t numRows = Ppattern_->getNodeNumRows();
    XXtInv_.resize(numRows);
    Teuchos::SerialDenseVector<LO,SC> BcRow(NSDim, false);
    for (size_t i = 0; i < numRows; i++) {
      Teuchos::ArrayView<const LO> indices;
      Ppattern_->getLocalRowView(i, indices);

      size_t nnz = indices.size();

      Teuchos::SerialDenseMatrix<LO,SC> locX(NSDim, nnz, false);
      for (size_t j = 0; j < nnz; j++) {
        for (size_t k = 0; k < NSDim; k++)
          BcRow[k] = X_->getData(k)[indices[j]];

        Teuchos::setCol(BcRow, (LO)j, locX);
      }

      XXtInv_[i] = Teuchos::SerialDenseMatrix<LO,SC>(NSDim, NSDim, false);

      Teuchos::BLAS<LO,SC> blas;
      blas.GEMM(Teuchos::NO_TRANS, Teuchos::CONJ_TRANS, NSDim, NSDim, nnz,
                Teuchos::ScalarTraits<SC>::one(), locX.values(), locX.stride(),
                locX.values(), locX.stride(), Teuchos::ScalarTraits<SC>::zero(),
                XXtInv_[i].values(), XXtInv_[i].stride());

      Teuchos::LAPACK<LO,SC> lapack;
      LO info, lwork = 3*NSDim;
      ArrayRCP<LO> IPIV(NSDim);
      ArrayRCP<SC> WORK(lwork);
      lapack.GETRF(NSDim, NSDim, XXtInv_[i].values(), XXtInv_[i].stride(), IPIV.get(), &info);
      lapack.GETRI(NSDim, XXtInv_[i].values(), XXtInv_[i].stride(), IPIV.get(), WORK.get(), lwork, &info);
    }
  }

  //! \note We assume that the graph of Projected is the same as Ppattern_
  template<class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
  void Constraint<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::Apply(const Matrix& P, Matrix& Projected) const {
    // We check only row maps. Column may be different.
    TEUCHOS_TEST_FOR_EXCEPTION(!P.getRowMap()->isSameAs(*Projected.getRowMap()),   Exceptions::Incompatible, "Row maps are incompatible");
    const size_t NSDim   = X_->getNumVectors();
    const size_t numRows = P.getNodeNumRows();

    const Map& ColMap  = *P.getColMap();
    const Map& PColMap = *Projected.getColMap();

    Projected.resumeFill();

    Teuchos::ArrayView<const LO> indices, pindices;
    Teuchos::ArrayView<const SC> values,  pvalues;
    Teuchos::Array<SC> valuesAll(ColMap.getNodeNumElements()), newValues;

    LO invalid = Teuchos::OrdinalTraits<LO>::invalid();
    SC zero    = Teuchos::ScalarTraits<SC> ::zero();

    Teuchos::SerialDenseVector<LO,SC> BcRow(NSDim, false);
    for (size_t i = 0; i < numRows; i++) {
      P        .getLocalRowView(i,  indices,  values);
      Projected.getLocalRowView(i, pindices, pvalues);

      size_t nnz  = indices.size();     // number of nonzeros in the supplied matrix
      size_t pnnz = pindices.size();    // number of nonzeros in the constrained matrix

      newValues.resize(pnnz);

      // step 1: fix stencil
      // Projected *must* already have the correct stencil

      // step 2: copy correct stencil values
      // The algorithm is very similar to the one used in the calculation of
      // Frobenius dot product, see src/Transfers/Energy-Minimization/Solvers/MueLu_CGSolver_def.hpp
      for (size_t j = 0; j < nnz; j++)
        valuesAll[indices[j]] = values[j];
      for (size_t j = 0; j < pnnz; j++) {
        LO ind = ColMap.getLocalElement(PColMap.getGlobalElement(pindices[j]));
        if (ind != invalid)
          // index indices[j] is part of template, copy corresponding value
          newValues[j] = valuesAll[ind];
        else
          newValues[j] = zero;
      }
      for (size_t j = 0; j < nnz; j++)
        valuesAll[indices[j]] = zero;

      // step 3: project to the space
      Teuchos::SerialDenseMatrix<LO,SC> locX(NSDim, pnnz, false);
      for (size_t j = 0; j < pnnz; j++) {
        for (size_t k = 0; k < NSDim; k++)
          BcRow[k] = X_->getData(k)[pindices[j]];

        Teuchos::setCol(BcRow, (LO)j, locX);
      }

      Teuchos::SerialDenseVector<LO,SC> val(pnnz, false), val1(NSDim, false), val2(NSDim, false);
      for (size_t j = 0; j < pnnz; j++)
        val[j] = newValues[j];

      Teuchos::BLAS<LO,SC> blas;
      blas.GEMV(Teuchos::NO_TRANS, NSDim, pnnz, Teuchos::ScalarTraits<SC>::one(), locX.values(),
                locX.stride(), val.values(), (LO)1, Teuchos::ScalarTraits<SC>::zero(), val1.values(), (LO)1);
      blas.GEMV(Teuchos::NO_TRANS, NSDim, NSDim, Teuchos::ScalarTraits<SC>::one(), XXtInv_[i].values(),
                XXtInv_[i].stride(), val1.values(), (LO)1, Teuchos::ScalarTraits<SC>::zero(), val2.values(), (LO)1);
      blas.GEMV(Teuchos::CONJ_TRANS, NSDim, pnnz, Teuchos::ScalarTraits<SC>::one(), locX.values(),
                locX.stride(), val2.values(), (LO)1, Teuchos::ScalarTraits<SC>::zero(), val.values(), (LO)1);

      for (size_t j = 0; j < pnnz; j++)
        newValues[j] -= val[j];

      Projected.replaceLocalValues(i, pindices, newValues);
    }

    Projected.fillComplete(Projected.getDomainMap(), Projected.getRangeMap()); //FIXME: maps needed?
  }

} // namespace MueLu

#endif //ifndef MUELU_CONSTRAINT_DEF_HPP

// FIXME: faster way to do things?
