/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_GRID_H
#define FUSION_GRID_H

#include <Fusion/StdDefs.h>
#include <Fusion/Widget/WidgetContainer.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS GRID
==============================================================================*/

//!
//!
//! Attributes list for VM:
//!
//!   read/write:
//!     hGap:     Horizontal space between to consecutive widget (default: 0)
//!     vGap:     Vertical space between consecutive widget (default: 0)
//!     rowMajor: Is children list row major?
//!               true  = Row major (default)
//!               false = Column major.
//!     dimMajor: Number of grid boxes along the major axis
//!               The other grid dimension of matrix is determined automatically
//!               based on the number of children.

class Grid
   : public WidgetContainer
{

public:
  
   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/

   FUSION_DLL_API Grid();

   // VM.
   const char* meta() const;
   FUSION_DLL_API void init( VMState* vm );
   FUSION_DLL_API bool performGet( VMState* );
   FUSION_DLL_API bool performSet( VMState* );
   
protected:

   /*----- methods -----*/

   virtual ~Grid();

   virtual void render( const RCP<Gfx::RenderNode>& );
   virtual bool isAttribute( const char* ) const;

   virtual Vec2f performComputeBaseSize();
   virtual void performSetGeometry();
   virtual void performSetPosition();

   inline int rowCount() const;
   inline int colCount() const;
   inline int getDimMajor() const;
   inline int getDimMinor() const;
   
   /*----- data members -----*/

   // Members are not privates to allow override of the values
   // in a derived constructor.

   Vec2f _gap;
   bool  _rowMajor;
   int   _dimMajor;
   
private:

   /*----- methods -----*/

   Vec2f computeBaseRowColSize( float* majorSize, float* minorSize ) const;

   
   /*----- data members -----*/
   
   bool  _scissored;
   Vec2f _scPos;
   Vec2f _scSize;
};



//------------------------------------------------------------------------------
//!
inline int
Grid::rowCount() const
{
   return (_rowMajor)?getDimMajor():getDimMinor();
}

//------------------------------------------------------------------------------
//!
inline int
Grid::colCount() const
{
   return (_rowMajor)?getDimMinor():getDimMajor();
}

//------------------------------------------------------------------------------
//!
inline int
Grid::getDimMajor() const
{
   return _dimMajor;
}

//------------------------------------------------------------------------------
//!
inline int
Grid::getDimMinor() const
{
   return (int(_widgets.size())+(_dimMajor-1)) / _dimMajor;
}

NAMESPACE_END

#endif

