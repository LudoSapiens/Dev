/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_DFMESSENGER_H
#define PLASMA_DFMESSENGER_H

#include <Plasma/StdDefs.h>

#include <Plasma/DataFlow/DFNode.h>

#include <Fusion/VM/DelegateGroup.h>
#include <Fusion/VM/VM.h>

#include <Base/ADT/Map.h>
#include <Base/Dbg/Defs.h>

NAMESPACE_BEGIN

class DFGraph;

/*==============================================================================
  CLASS DFMessenger
==============================================================================*/
class DFMessenger
{
public:

   /*----- types -----*/

   typedef Delegate1Group<DFGraph*>  GraphUpdateGroup;
   typedef GraphUpdateGroup::Func    GraphUpdateFunc;
   typedef Delegate1Group<DFNode*>   NodeUpdateGroup;
   typedef NodeUpdateGroup::Func     NodeUpdateFunc;
   typedef Delegate3Group<DFNode*, const DFNodeAttrList*, const DFNodeAttrStates*>  NodeModifyGroup;
   typedef NodeModifyGroup::Func     NodeModifyFunc;
   typedef Delegate1Group<DFNode*>   NodeRemoveGroup;
   typedef NodeRemoveGroup::Func     NodeRemoveFunc;

   /*----- methods -----*/

   PLASMA_DLL_API       DFGraph&  graph();
   PLASMA_DLL_API const DFGraph&  graph() const;

   PLASMA_DLL_API void  update();

   inline void  addOnUpdate   ( const GraphUpdateFunc& d ) { _graphUpdate.add(d);    }
   inline void  addOnUpdate   ( const VMRef& r )           { _graphUpdate.add(r);    }
   inline void  removeOnUpdate( const GraphUpdateFunc& d ) { _graphUpdate.remove(d); }
   inline void  removeOnUpdate( const VMRef& r )           { _graphUpdate.remove(r); }
   inline void  removeAllOnUpdate()                        { _graphUpdate.clear();   }

   PLASMA_DLL_API void  update( DFNode* node );

   inline void  addOnUpdate   ( DFNode* n, const NodeUpdateFunc& d ) { CHECK(n->graph() == &graph()); _nodeUpdate[n].add(d);    }
   inline void  addOnUpdate   ( DFNode* n, const VMRef& r )          { CHECK(n->graph() == &graph()); _nodeUpdate[n].add(r);    }
   inline void  removeOnUpdate( DFNode* n, const NodeUpdateFunc& d ) { CHECK(n->graph() == &graph()); _nodeUpdate[n].remove(d); }
   inline void  removeOnUpdate( DFNode* n, const VMRef& r )          { CHECK(n->graph() == &graph()); _nodeUpdate[n].remove(r); }
   inline void  removeAllOnUpdate( DFNode* n )                       { CHECK(n->graph() == &graph()); _nodeUpdate.erase(n);     }

   inline         void  modify( DFNode* n, const DFNodeAttrList* a )   { modify( n, a, nullptr ); }
   inline         void  modify( DFNode* n, const DFNodeAttrStates* t ) { modify( n, nullptr, t ); }
   PLASMA_DLL_API void  modify( DFNode* n, const DFNodeAttrList* a, const DFNodeAttrStates* t );

   inline void  addOnModify   ( DFNode* n, const NodeModifyFunc& d ) { CHECK(n->graph() == &graph()); _nodeModify[n].add(d);    }
   inline void  addOnModify   ( DFNode* n, const VMRef& r )          { CHECK(n->graph() == &graph()); _nodeModify[n].add(r);    }
   inline void  removeOnModify( DFNode* n, const NodeModifyFunc& d ) { CHECK(n->graph() == &graph()); _nodeModify[n].remove(d); }
   inline void  removeOnModify( DFNode* n, const VMRef& r )          { CHECK(n->graph() == &graph()); _nodeModify[n].remove(r); }
   inline void  removeAllOnModify( DFNode* n )                       { CHECK(n->graph() == &graph()); _nodeModify.erase(n);     }

   PLASMA_DLL_API void  remove( DFNode* node );

   inline void  addOnRemove   ( DFNode* n, const NodeRemoveFunc& d ) { CHECK(n->graph() == &graph()); _nodeRemove[n].add(d);    }
   inline void  addOnRemove   ( DFNode* n, const VMRef& r )          { CHECK(n->graph() == &graph()); _nodeRemove[n].add(r);    }
   inline void  removeOnRemove( DFNode* n, const NodeRemoveFunc& d ) { CHECK(n->graph() == &graph()); _nodeRemove[n].remove(d); }
   inline void  removeOnRemove( DFNode* n, const VMRef& r )          { CHECK(n->graph() == &graph()); _nodeRemove[n].remove(r); }
   inline void  removeAllOnRemove( DFNode* n )                       { CHECK(n->graph() == &graph()); _nodeRemove.erase(n);     }

   inline void  removeAll( DFNode* n ) { removeAllOnUpdate(n); removeAllOnModify(n); removeAllOnRemove(n); }

protected:

	friend class DFGraph;

   /*----- data members -----*/

   GraphUpdateGroup               _graphUpdate;
   Map<DFNode*, NodeUpdateGroup>  _nodeUpdate;
   Map<DFNode*, NodeModifyGroup>  _nodeModify;
   Map<DFNode*, NodeRemoveGroup>  _nodeRemove;

   /*----- methods -----*/

   DFMessenger();
   ~DFMessenger();

private:
}; //class DFMessenger

NAMESPACE_END

#endif //PLASMA_DFMESSENGER_H
