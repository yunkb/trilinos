/*
 * TwoKeyMap.hpp
 *
 *  Created on: Aug 29, 2011
 *      Author: wiesner
 */
#ifndef MUELU_TWOKEYMAP_HPP
#define MUELU_TWOKEYMAP_HPP

#include <Teuchos_TabularOutputter.hpp>
#include <Teuchos_ParameterEntry.hpp>
#include <Teuchos_map.hpp>

#include "MueLu_Exceptions.hpp"
#include "MueLu_BaseClass.hpp"
#include "MueLu_FactoryBase.hpp"

// JG: Some notes from code review 2011-10-12:
//
// TODO: 
// - Use factory ID instead of pointer to distinghuish factories
//
// QUESTIONS:
// - Can we use an std::map<Tuple<const std::string, const MueLu::FactoryBase*>, ... > instead?
// - Teuchos::any vs. Teuchos::ParameterEntry?
// - Teuchos::ConstNonconstObjectContainer? Avoid const and non-cosnt version of same method
// - Can we use an std::map<... , Tuple<counter, factory*> >  instead?
// - Can be more generic (template type for key1, key2)

namespace MueLu {
  namespace UTILS {

    using std::string;
    
    class TwoKeyMap : public BaseClass {

    private:
     
      //! map container typedef (factory ptr -> value)
      typedef Teuchos::map<const MueLu::FactoryBase*, Teuchos::ParameterEntry> dataMapType;

      //! hashtable container typedef (map of a map)
      typedef Teuchos::map<const string, dataMapType > dataTableType;

      //! hashtable container const iterator typedef
      typedef dataTableType::const_iterator ConstIterator;

      //! map container const iterator typedef
      typedef dataMapType::const_iterator ConstMapIterator;

      //! data storage object for extended hashtable
      dataTableType map_;

    public:

      TwoKeyMap() { };

      template<class Value> 
      void Set(const string & ename, const Value & evalue, const FactoryBase* factory) {
        // if ename does not exist at all
        if (!map_.count(ename) > 0) {
          Teuchos::map<const MueLu::FactoryBase*, Teuchos::ParameterEntry> newMapData;
          map_[ename] = newMapData; // empty map
        }

        dataMapType & mapData = map_[ename];
        Teuchos::ParameterEntry & foundEntry = mapData[factory]; // operator [] will add the entry for key 'factory' if not exists
        foundEntry.setValue(evalue);
      }

      template<class Value>
      Value & Get(const string & ename, const FactoryBase* factory) {
        if (!map_.count(ename) > 0) {
          std::stringstream str; str << "key" << ename << " does not exist in Hashtable.";
          throw(Exceptions::RuntimeError(str.str()));
        }

        ConstIterator k = map_.find(ename);
        const dataMapType & mapData = k->second;

        if (!mapData.count(factory) > 0) {
          std::stringstream str; str << "key " << ename << " generated by " << factory << " does not exist in Hashtable.";
          throw(Exceptions::RuntimeError(str.str()));
        }

        ConstMapIterator i = mapData.find(factory);
        return Teuchos::getValue<Value>(entry(i));
      }

      template<class Value>
      const Value & Get(const string & ename, const FactoryBase* factory) const {
        if (!map_.count(ename) > 0) {
          std::stringstream str; str << "key" << ename << " does not exist in Hashtable.";
          throw(Exceptions::RuntimeError(str.str()));
        }

        ConstIterator k = map_.find(ename);
        const dataMapType & mapData = k->second;

        if (!mapData.count(factory) > 0) {
          std::stringstream str; str << "key " << ename << " generated by " << factory << " does not exist in Hashtable.";
          throw(Exceptions::RuntimeError(str.str()));
        }

        ConstMapIterator i = mapData.find(factory);
        return Teuchos::getValue<Value>(entry(i));
      }

      void Remove(const string & ename, const FactoryBase* factory) {
        if (!map_.count(ename) > 0) {
          std::stringstream str; str << "key" << ename << " does not exist in Hashtable.";
          throw(Exceptions::RuntimeError(str.str()));
        }

        if (!map_[ename].count(factory) > 0) {
          std::stringstream str; str << "key " << ename << " generated by " << factory << " does not exist in Hashtable.";
          throw(Exceptions::RuntimeError(str.str()));
        }

        if (map_[ename].erase(factory)!=1) {
          std::stringstream str; str << "error: could not erase " << ename << "generated gy " << factory;
          throw(Exceptions::RuntimeError(str.str()));
        }

        // check if there exist other instances of 'ename' (generated by other factories than 'factory')
        if (map_.count(ename) == 0)
          map_.erase(ename); // last instance of 'ename' can be removed
      }

      std::string GetType(const string & ename, const FactoryBase* factory) const {
        if (!map_.count(ename) > 0) {
          std::stringstream str; str << "key" << ename << " does not exist in Hashtable.";
          return std::string("requested, but not existing?");
          //throw(Exceptions::RuntimeError(str.str()));
        }

        ConstIterator k = map_.find(ename);
        const dataMapType & mapData = k->second;

        if (!mapData.count(factory) > 0) {
          std::stringstream str; str << "key " << ename << " generated by " << factory << " does not exist in Hashtable.";
          return std::string("requested, but not existing?");
          //throw(Exceptions::RuntimeError(str.str()));
        }

        ConstMapIterator i = mapData.find(factory);
        return entry(i).getAny(true).typeName();
      }

      bool isKey(const string & ename, const FactoryBase* factory) const {
        // check if ename exists
        if (!map_.count(ename) > 0) return false;

        // resolve factory to a valid factory ptr
        if (map_.find(ename)->second.count(factory) > 0) // == if (map_[ename].count(factory) > 0) but operator [] can't be used with 'const'
          return true;

        return false;
      }

      //TODO: GetKeyList and GetFactoryList looks expensive. Also return by value a vector...

      std::vector<string> GetKeyList() const {
        std::vector<string> v;
        for(ConstIterator it = map_.begin(); it != map_.end(); ++it) {
          v.push_back(it->first);
        }
        return v;
      }

      std::vector<const MueLu::FactoryBase*> GetFactoryList(const string & ename) const {
        if (!map_.count(ename) > 0) {
          std::stringstream str; str << "key" << ename << " does not exist in Hashtable.";
          throw(Exceptions::RuntimeError(str.str()));
        }

        std::vector<const MueLu::FactoryBase*> v;

        const dataMapType & mapData = map_.find(ename)->second;

        for(ConstMapIterator it = mapData.begin(); it != mapData.end(); ++it) {
          v.push_back(it->first);
        }

        return v;
      }

      void print(Teuchos::FancyOStream &out, const VerbLevel verbLevel = Default) const {
        Teuchos::TabularOutputter outputter(out);
        outputter.pushFieldSpec("name", Teuchos::TabularOutputter::STRING, Teuchos::TabularOutputter::LEFT, Teuchos::TabularOutputter::GENERAL, 12);
        outputter.pushFieldSpec("gen. factory addr.", Teuchos::TabularOutputter::STRING, Teuchos::TabularOutputter::LEFT, Teuchos::TabularOutputter::GENERAL, 18);
        outputter.pushFieldSpec("type", Teuchos::TabularOutputter::STRING, Teuchos::TabularOutputter::LEFT, Teuchos::TabularOutputter::GENERAL, 18);
        outputter.outputHeader();

        std::vector<std::string> ekeys = GetKeyList();
        for (std::vector<std::string>::iterator it = ekeys.begin(); it != ekeys.end(); it++) {
          std::vector<const MueLu::FactoryBase*> ehandles = GetFactoryList(*it);
          for (std::vector<const MueLu::FactoryBase*>::iterator kt = ehandles.begin(); kt != ehandles.end(); kt++) {
            outputter.outputField(*it);
            outputter.outputField(*kt);
            outputter.outputField(GetType(*it, *kt));
            outputter.nextRow();
          }
        }
      }

    private:

      //! \name Access to ParameterEntry in dataMap
      //@{

      const Teuchos::ParameterEntry & entry(ConstMapIterator i) const {
        return (i->second);
      }

      //@}

    };

  } // namespace UTILS

} // namespace MueLu

#endif /* MUELU_TWOKEYMAP_HPP */
