/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_VALIDATOR_H
#define BASE_VALIDATOR_H

#include <Base/StdDefs.h>

#include <Base/ADT/String.h>

#include <cfloat>
#include <climits>

NAMESPACE_BEGIN


/*==============================================================================
  CLASS Validator
==============================================================================*/
class Validator
{
public:

   /*----- methods -----*/

   Validator();
   virtual ~Validator();

   virtual bool    validate( const String& input ) = 0;
   virtual bool    validateIntermediate( const String& input );
   virtual String  makeValid( const String& input );
   virtual String  makeValidIntermediate( const String& input );

protected:

private:
}; //class Validator


/*==============================================================================
  CLASS IntValidator
==============================================================================*/
class IntValidator:
   public Validator
{
public:

   /*----- methods -----*/

   IntValidator( const int minimum = (-INT_MAX-1), const int maximum = INT_MAX );
   virtual ~IntValidator();

   virtual bool    validate( const String& input );
   virtual String  makeValid( const String& input );

   int  minimum() { return _min; }
   int  maximum() { return _max; }

   void  minimum( const int minimum ) { _min = minimum; }
   void  maximum( const int maximum ) { _max = maximum; }
   void  setRange( const int minimum, const int maximum ) { _min = minimum; _max = maximum; }

protected:

   /*----- data members -----*/

   int  _value;
   int  _min;
   int  _max;

private:
}; //class IntValidator


/*==============================================================================
  CLASS FltValidator
==============================================================================*/
class FltValidator:
   public Validator
{
public:

   /*----- methods -----*/

   FltValidator( const double minimum = 0.0, const double maximum = DBL_MAX, const uint frac = 2 );
   virtual ~FltValidator();

   virtual bool    validate( const String& input );
   virtual String  makeValid( const String& input );

   double  minimum() { return _min; }
   double  maximum() { return _max; }
   uint    frac() { return _frac; }

   void  minimum( const double minimum ) { _min = minimum; }
   void  maximum( const double maximum ) { _max = maximum; }
   void  setRange( const double minimum, const double maximum ) { _min = minimum; _max = maximum; }
   void  frac( const uint frac ) { _frac = frac; }

protected:

   /*----- data members -----*/

   double  _value;
   double  _min;
   double  _max;
   uint    _frac;   //!< The number of decimals

private:
}; //class FltValidator


NAMESPACE_END


#endif /* BASE_VALIDATOR_H */
