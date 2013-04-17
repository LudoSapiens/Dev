/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/Util/Validator.h>

#include <cstdio>

USING_NAMESPACE


/*==============================================================================
  CLASS Validator
==============================================================================*/

//------------------------------------------------------------------------------
//!
Validator::Validator
( void )
{
   
}

//------------------------------------------------------------------------------
//!
Validator::~Validator
( void )
{
   
}

//------------------------------------------------------------------------------
//!
bool
Validator::validateIntermediate
( const String& /*input*/ )
{
   return true;
}

//------------------------------------------------------------------------------
//!
String
Validator::makeValid
( const String& input )
{
   return input;
}

//------------------------------------------------------------------------------
//!
String
Validator::makeValidIntermediate
( const String& input )
{
   return input;
}


/*==============================================================================
  CLASS IntValidator
==============================================================================*/

//------------------------------------------------------------------------------
//!
IntValidator::IntValidator
( const int minimum, const int maximum ):
   _value(0),
   _min(minimum),
   _max(maximum)
{
   
}

//------------------------------------------------------------------------------
//!
IntValidator::~IntValidator
( void )
{
   
}

//------------------------------------------------------------------------------
//!
bool
IntValidator::validate
( const String& input )
{
   uint len;
   String input_nws = input.eatWhites();
   if( sscanf(input_nws.cstr(), "%d%n", &_value, &len) == 1 )
   {
      if( len != input_nws.size() )
      {
         //Extra character(s)
         return false;
      }
      else
      if( _min <= _value && _value <= _max )
      {
         return true;
      }
      else
      {
         return false;
      }
   }
   else
   {
      return false;
   }
}

//------------------------------------------------------------------------------
//!
String
IntValidator::makeValid
( const String& input )
{
   String tmp;
   if( sscanf(input.cstr(), "%d", &_value) == 1 )
   {
      if( _max < _value )  _value = _max;
      if( _value < _min )  _value = _min;
      tmp.format("%d", _value);
   }
   else
   {
      //tmp.format("%d", _min);
   }
   return tmp;
}


/*==============================================================================
  CLASS FltValidator
==============================================================================*/

//------------------------------------------------------------------------------
//!
FltValidator::FltValidator
( const double minimum, const double maximum, const uint frac ):
   _value(0.0),
   _min(minimum),
   _max(maximum),
   _frac(frac)
{
   
}

//------------------------------------------------------------------------------
//!
FltValidator::~FltValidator
( void )
{
   
}

//------------------------------------------------------------------------------
//!
bool
FltValidator::validate
( const String& input )
{
   uint len;
   String input_nws = input.eatWhites();
   if( sscanf(input_nws.cstr(), "%lf%n", &_value, &len) == 1 )
   {
      if( len != input_nws.size() )
      {
         //Extra character(s)
         return false;
      }
      else
      if( _min <= _value && _value <= _max )
      {
         //Need to check format (reformat, and check that the value hasn't changed)
         double old_value = _value;
         String fmt;
         fmt.format("%%.%df", _frac);
         String tmp;
         tmp.format(fmt.cstr(), _value);
         sscanf(tmp.cstr(), "%lf", &_value);
         return _value == old_value;
      }
      else
      {
         return false;
      }
   }
   else
   {
      return false;
   }
}

//------------------------------------------------------------------------------
//!
String
FltValidator::makeValid
( const String& input )
{
   String fmt;
   fmt.format("%%.%df", _frac);
   String tmp;
   if( sscanf(input.cstr(), "%lf", &_value) == 1 )
   {
      if( _max < _value )  _value = _max;
      if( _value < _min )  _value = _min;
      tmp.format(fmt.cstr(), _value);
   }
   else
   {
      //tmp.format(fmt.cstr(), _min);
   }
   return tmp;
}
