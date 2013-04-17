/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_VMSUBJECT_H
#define FUSION_VMSUBJECT_H

#include <Fusion/StdDefs.h>

#include <Fusion/VM/VMObject.h>

#include <Base/Util/RCObject.h>
#include <Base/Msg/Subject.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS VMSubject
==============================================================================*/
class VMSubject:
   public RCObject
{
public:

   /*----- methods -----*/

   VMSubject();
   virtual ~VMSubject();

         Subject&  subject()       { return _subject; }
   const Subject&  subject() const { return _subject; }

   void  subject( const Subject& subject ) { _subject = subject; }

   // VM.
   FUSION_DLL_API void init( VMState* );
   FUSION_DLL_API virtual bool performGet( VMState* );
   FUSION_DLL_API virtual bool performSet( VMState* );

protected:

   /*----- data members -----*/

   Subject  _subject;  //!< The pointer to the subject class.

private:
}; //class VMSubject


/*==============================================================================
  VM Section
==============================================================================*/

VMOBJECT_TRAITS( VMSubject, subject )
typedef VMObject< VMSubject > VMSubjectVM;


NAMESPACE_END

#endif //FUSION_VMSUBJECT_H
