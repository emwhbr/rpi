// ************************************************************************
// *                                                                      *
// * Copyright (C) 2013 Bonden i Nol (hakanbrolin@hotmail.com)            *
// *                                                                      *
// * This program is free software; you can redistribute it and/or modify *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation; either version 2 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// ************************************************************************

#ifndef __CFG_FILE_H__
#define __CFG_FILE_H__

#include <string>
#include <vector>
#include <typeinfo>
#include <sstream>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

// Return codes
#define CFG_FILE_SUCCESS           0
#define CFG_FILE_FILE_NOT_FOUND   -1
#define CFG_FILE_FILE_IO_ERROR    -2
#define CFG_FILE_BAD_FILE_FORMAT  -3
#define CFG_FILE_BAD_VALUE_FORMAT -4
#define CFG_FILE_ITEM_NOT_DEFINED -5

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////

class item {

 public:
 item(const string &name,
      ios_base& (*f)(ios_base&)) : m_name(name), m_f(f) {}
  virtual ~item(void) {}
  
  string m_name;
  ios_base& (*m_f)(ios_base&);
};

template< typename T >
class typed_item : public item {

 public:
  typed_item(const string &name,
	     const T &value,
	     ios_base& (*f)(ios_base&))
    : item(name, f), m_value(value) {}
   ~typed_item(void) {}
  
  T m_value;
};

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class cfg_file {

 public:
  cfg_file(string file_name);
  ~cfg_file(void);

  long parse(void);

 protected:

  ////////////////////////////////////////////////////////////////

  template <typename T>
    void set_default_item_value(const char *item,
				const T value,
				ios_base& (*f)(ios_base&))
    {
      m_item_list.push_back(new typed_item<T>(item, value, f));
    }

  ////////////////////////////////////////////////////////////////

  template <typename T>
    long get_item_value(const char *item,
			T &value)
    {
      bool defined = false;
      value = 0.0;
      
      for (unsigned i=0; i < m_item_list.size(); i++) {
	if (m_item_list[i]->m_name == item) {
	  typed_item<T> *item_T;
	  item_T = dynamic_cast<typed_item<T>*>(m_item_list[i]);
	  if (item_T) {
	    value = item_T->m_value;
	    defined = true;
	    break;
	  }
	}
      }
      
      if (!defined) {
	return CFG_FILE_ITEM_NOT_DEFINED;
      }
      
      return CFG_FILE_SUCCESS;
    }

 private:
  string m_file_name;

  // List of populated items from config file
  vector<item*> m_item_list;

  string trim_all(const string &row);

  bool row_format_ok(const string &row);

  long populate_item(const char *item,
		     const char *value);

  ////////////////////////////////////////////////////////////////

  template <typename T>
    bool is_string_type_t(const string &value,
			  ios_base& (*f)(ios_base&))
    {
      istringstream iss(value);
      T num;
      return !(iss >> f >> num).fail();
    }

  ////////////////////////////////////////////////////////////////

  template <typename T>
    void from_string(T& num,
		     const string &value,
		     ios_base& (*f)(ios_base&))
    {
      istringstream iss(value);
      (iss >> f >> num);
    }
};

#endif // __CFG_FILE_H__
