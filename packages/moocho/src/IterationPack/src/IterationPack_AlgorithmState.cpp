// //////////////////////////////////////////////////////////////////
// AlgorithmState.cpp

#pragma warning(disable : 4786)	// too long class name for debugger warning

#include <ostream>
#include <iomanip>
#include <typeinfo>

#include "../include/AlgorithmState.h"

namespace {
inline void output_spaces(std::ostream& out, int spaces)
{	for(int i = 0; i < spaces; ++i) out << ' '; }
}

namespace GeneralIterationPack {

AlgorithmState::iq_id_type AlgorithmState::set_iter_quant(
	const std::string& iq_name, const IQ_ptr& iq)
{
	iq_id_type new_id = iq_.size();
	std::pair<iq_name_to_id_t::iterator,bool>
		r = iq_name_to_id_.insert(iq_name_to_id_t::value_type(iq_name,new_id));
	if( !r.second ) {	// an insert did not take place, key = iq_name already existed.
		std::ostringstream omsg;
		omsg	<< "AlgorithmState::set_iter_quant(...) : An iteration quantity with the name \""
				<< iq_name << "\" already exists with the iq_id = " << r.first->second;
		throw AlreadyExists(omsg.str());
	}
	iq_.push_back(iq);
	return new_id;
}

void AlgorithmState::erase_iter_quant(const std::string& iq_name) {
	iq_name_to_id_t::iterator itr = find_and_assert(iq_name);
	const iq_id_type iq_id = itr->second;
	iq_[iq_id] = IQ_ptr(0);	// set the pointer to null
	iq_name_to_id_.erase( itr );
}

IterQuantity& AlgorithmState::iter_quant(iq_id_type iq_id) {
	bool exists = true;
	try {
		IQ_ptr &_iq = iq_.at(iq_id);
		if( _iq.get() )
			return *_iq;
		else
			exists = false;
	}
	catch(const std::out_of_range& excpt) {	// Thrown by MS VC++ 6.0
		exists = false;
	}
	catch(const std::range_error& excpt) {	// Thrown by libstdc++ v3 in g++ 2.95.2
		exists = false;
	}
	if( !exists ) {
		std::ostringstream omsg;
		omsg
			<< "AlgorithmState::iter_quant(iq_id) : Error, the iteration quantity iq_id = "
			<< iq_id << " does not exist.  ";
		if( iq_id < iq_.size() )
			omsg
				<< "This iteration quantity was set and then erased.";
		else
			omsg
				<< "This iteration quantity was never set by the client.";
																		
		throw DoesNotExist( omsg.str() );
	}																								
	return *iq_.at(0);	// Will never be executed.
}

const IterQuantity& AlgorithmState::iter_quant(iq_id_type iq_id) const {
	return const_cast<AlgorithmState*>(this)->iter_quant(iq_id);
}

void AlgorithmState::next_iteration(bool incr_k) {
	if(incr_k) this->incr_k();
	for(iq_t::iterator itr = iq_.begin(); itr != iq_.end(); ++itr)
		if(itr->get()) (*itr)->next_iteration();
}

void AlgorithmState::dump_iter_quant(std::ostream& out) const {
	using std::setw;
	using std::endl;

	const int name_w = 30, id_w = 6;
	char gap[] = "     ";

	out		<< "\n\n"
			<< std::left	<< setw(name_w)		<< "iq_name"
			<< std::right	<< setw(id_w)		<< "iq_id"
			<< gap			<< std::left		<< "concrete type\n";

	out		<< std::left	<< setw(name_w)	<< "-----"
			<< std::right	<< setw(id_w)	<< "-------"
			<< gap			<< std::left	<< "-------------\n";

	for(	iq_name_to_id_t::const_iterator itr = iq_name_to_id_.begin();
			itr !=  iq_name_to_id_.end(); ++itr )
	{
		out		<< std::left					<< itr->first;

		output_spaces( out, name_w - itr->first.size() );

		out		<< std::right	<< setw(id_w)	<< itr->second
				<< gap			<< std::left	<< typeid(*iq_[itr->second]).name() << endl;
	}	
}

// private

AlgorithmState::iq_name_to_id_t::iterator AlgorithmState::find_and_assert(
	const std::string& iq_name)
{
	iq_name_to_id_t::iterator itr = iq_name_to_id_.find(iq_name);
	if(itr == iq_name_to_id_.end()) {
		std::ostringstream omsg;
		omsg	<< "AlgorithmState::find_and_assert(iq_name) : The iteration "
					"quantity with the name \"" << iq_name << "\" does not exist";
		throw DoesNotExist(omsg.str());
	}
	return itr;
}

AlgorithmState::iq_name_to_id_t::const_iterator AlgorithmState::find_and_assert(
	const std::string& iq_name) const
{
	iq_name_to_id_t::const_iterator itr = iq_name_to_id_.find(iq_name);
	if(itr == iq_name_to_id_.end()) {
		std::ostringstream omsg;
		omsg	<< "AlgorithmState::find_and_assert(iq_name) : The iteration "
					"quantity with the name \"" << iq_name << "\" does not exist";
		throw DoesNotExist(omsg.str());
	}
	return itr;
}

}	// end namespace GeneralIterationPack