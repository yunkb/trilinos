// @HEADER
// ***********************************************************************
//
//           Panzer: A partial differential equation assembly
//       engine for strongly coupled complex multiphysics systems
//                 Copyright (2011) Sandia Corporation
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
// Questions? Contact Roger P. Pawlowski (rppawlo@sandia.gov) and
// Eric C. Cyr (eccyr@sandia.gov)
// ***********************************************************************
// @HEADER

#ifndef USER_APP_CLOSURE_MODEL_FACTORY_T_HPP
#define USER_APP_CLOSURE_MODEL_FACTORY_T_HPP

#include <iostream>
#include <sstream>
#include <typeinfo>
#include "Panzer_InputEquationSet.hpp"
#include "Panzer_IntegrationRule.hpp"
#include "Panzer_BasisIRLayout.hpp"
#include "Panzer_Integrator_Scalar.hpp"
#include "Phalanx_FieldTag_Tag.hpp"
#include "Teuchos_ParameterEntry.hpp"
#include "Teuchos_TypeNameTraits.hpp"

// User application evaluators for this factory
#include "user_app_ConstantModel.hpp"
#include "Panzer_Parameter.hpp"
#include "Panzer_GlobalStatistics.hpp"

// ********************************************************************
// ********************************************************************
template<typename EvalT>
Teuchos::RCP< std::vector< Teuchos::RCP<PHX::Evaluator<panzer::Traits> > > > 
user_app::MyModelFactory<EvalT>::
buildClosureModels(const std::string& model_id,
		   const panzer::InputEquationSet& set,
		   const Teuchos::ParameterList& models, 
		   const Teuchos::ParameterList& default_params,
		   const Teuchos::ParameterList& user_data,
		   const Teuchos::RCP<panzer::GlobalData>& global_data,
		   PHX::FieldManager<panzer::Traits>& fm) const
{

  using std::string;
  using std::vector;
  using Teuchos::RCP;
  using Teuchos::rcp;
  using Teuchos::ParameterList;
  using PHX::Evaluator;

  RCP< vector< RCP<Evaluator<panzer::Traits> > > > evaluators = 
    rcp(new vector< RCP<Evaluator<panzer::Traits> > > );

  if (!models.isSublist(model_id)) {
    models.print(std::cout);
    std::stringstream msg;
    msg << "Falied to find requested model, \"" << model_id 
	<< "\", for equation set:\n" << std::endl;
    TEUCHOS_TEST_FOR_EXCEPTION(!models.isSublist(model_id), std::logic_error, msg.str());
  }

  const ParameterList& my_models = models.sublist(model_id);

  for (ParameterList::ConstIterator model_it = my_models.begin(); 
       model_it != my_models.end(); ++model_it) {
    
    bool found = false;
    
    const std::string key = model_it->first;
    ParameterList input;
    const Teuchos::ParameterEntry& entry = model_it->second;
    const ParameterList& plist = Teuchos::getValue<Teuchos::ParameterList>(entry);

    #ifdef HAVE_STOKHOS
    if (plist.isType<double>("Value") && plist.isType<double>("UQ") 
                           && plist.isParameter("Expansion")
                           && (typeid(EvalT)==typeid(panzer::Traits::SGResidual) || 
                               typeid(EvalT)==typeid(panzer::Traits::SGJacobian)) ) {
      { // at IP
	input.set("Name", key);
	input.set("Value", plist.get<double>("Value"));
	input.set("UQ", plist.get<double>("UQ"));
	input.set("Expansion", plist.get<Teuchos::RCP<Stokhos::OrthogPolyExpansion<int,double> > >("Expansion"));
	input.set("Data Layout", default_params.get<RCP<panzer::IntegrationRule> >("IR")->dl_scalar);
	RCP< Evaluator<panzer::Traits> > e = 
	  rcp(new user_app::ConstantModel<EvalT,panzer::Traits>(input));
	evaluators->push_back(e);
      }
      { // at BASIS
	input.set("Name", key);
	input.set("Value", plist.get<double>("Value"));
	input.set("UQ", plist.get<double>("UQ"));
	input.set("Expansion", plist.get<Teuchos::RCP<Stokhos::OrthogPolyExpansion<int,double> > >("Expansion"));
	input.set("Data Layout", default_params.get<RCP<panzer::BasisIRLayout> >("Basis")->functional);
	RCP< Evaluator<panzer::Traits> > e = 
	  rcp(new user_app::ConstantModel<EvalT,panzer::Traits>(input));
	evaluators->push_back(e);
      }
      found = true;
    }
    else 
    #endif
    if (plist.isType<std::string>("Type")) {
      
      if (plist.get<std::string>("Type") == "Parameter") {
	{ // at IP
	  RCP< Evaluator<panzer::Traits> > e = 
	    rcp(new panzer::Parameter<EvalT,panzer::Traits>(key,default_params.get<RCP<panzer::IntegrationRule> >("IR")->dl_scalar,plist.get<double>("Value"),*global_data->pl));
	  evaluators->push_back(e);
	}
	{ // at BASIS
	  RCP< Evaluator<panzer::Traits> > e = 
	    rcp(new panzer::Parameter<EvalT,panzer::Traits>(key,default_params.get<RCP<panzer::BasisIRLayout> >("Basis")->functional,plist.get<double>("Value"),*global_data->pl));
	  evaluators->push_back(e);
	}
	
	found = true;
      }
  
    }
    else if (plist.isType<double>("Value")) {
      { // at IP
	input.set("Name", key);
	input.set("Value", plist.get<double>("Value"));
	input.set("Data Layout", default_params.get<RCP<panzer::IntegrationRule> >("IR")->dl_scalar);
	RCP< Evaluator<panzer::Traits> > e = 
	  rcp(new user_app::ConstantModel<EvalT,panzer::Traits>(input));
	evaluators->push_back(e);
      }
      { // at BASIS
	input.set("Name", key);
	input.set("Value", plist.get<double>("Value"));
	input.set("Data Layout", default_params.get<RCP<panzer::BasisIRLayout> >("Basis")->functional);
	RCP< Evaluator<panzer::Traits> > e = 
	  rcp(new user_app::ConstantModel<EvalT,panzer::Traits>(input));
	evaluators->push_back(e);
      }
      found = true;
    }

    if (plist.isType<std::string>("Value")) {
    
      const std::string value = plist.get<std::string>("Value");

      if (key == "Global Statistics") {
	if (typeid(EvalT) == typeid(panzer::Traits::Residual)) {
	  input.set("Comm", user_data.get<Teuchos::RCP<const Teuchos::Comm<int> > >("Comm"));
	  input.set("Names", value);
	  input.set("IR", default_params.get<RCP<panzer::IntegrationRule> >("IR"));
	  input.set("Global Data", global_data);
	  RCP< panzer::GlobalStatistics<EvalT,panzer::Traits> > e = 
	    rcp(new panzer::GlobalStatistics<EvalT,panzer::Traits>(input));
	  evaluators->push_back(e);
	  
	  // Require certain fields be evaluated
	  fm.template requireField<EvalT>(e->getRequiredFieldTag());
	}
	found = true;
      }

    }

    if (key == "Volume Integral") {

        {
           ParameterList input;
	   input.set("Name", "Unit Value");
	   input.set("Value", 1.0);
	   input.set("Data Layout", default_params.get<RCP<panzer::IntegrationRule> >("IR")->dl_scalar);
	   RCP< Evaluator<panzer::Traits> > e = 
   	     rcp(new user_app::ConstantModel<EvalT,panzer::Traits>(input));
   	   evaluators->push_back(e);
        }

        {
           ParameterList input;
	   input.set("Integral Name", "Volume_Integral");
	   input.set("Integrand Name", "Unit Value");
	   input.set("IR", default_params.get<RCP<panzer::IntegrationRule> >("IR"));

	   RCP< Evaluator<panzer::Traits> > e = 
   	     rcp(new panzer::Integrator_Scalar<EvalT,panzer::Traits>(input));
   	   evaluators->push_back(e);
        }

	found = true;
    }

    if (!found) {
      std::stringstream msg;
      msg << "ClosureModelFactory failed to build evaluator for key \"" << key 
	  << "\"\nin model \"" << model_id 
	  << "\".  Please correct the type or add support to the \nfactory." <<std::endl;
      TEUCHOS_TEST_FOR_EXCEPTION(!found, std::logic_error, msg.str());
    }

  }

  return evaluators;
}

#endif
