/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_FILE_DIALOG_H
#define FUSION_FILE_DIALOG_H

#include <Fusion/StdDefs.h>

#include <Fusion/VM/VM.h>

#include <Base/ADT/String.h>
#include <Base/ADT/Vector.h>
#include <Base/IO/Path.h>
#include <Base/Msg/DelegateList.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS FileDialog
==============================================================================*/
class FileDialog:
   public RCObject,
   public VMProxy
{
public:

   /*----- types -----*/
   enum Type
   {
      OPEN,
      SAVE,
   };

   typedef Delegate1< FileDialog& >  DialogDelegate;

   /*----- static methods -----*/

   static void  initialize();


   /*----- methods -----*/

   FUSION_DLL_API FileDialog();
   FUSION_DLL_API virtual ~FileDialog();

   Type  type() const { return _type; }
   void  type( Type type ) { _type = type; }

   void  title( const String& str ) { _title = str; }
   const String&  title() const { return _title; }

   void  prompt( const String& str ) { _prompt = str; }
   const String&  prompt() const { return _prompt; }

   void  message( const String& str ) { _message = str; }
   const String&  message() const { return _message; }

   void  startingLocation( const Path& path ) { _start = path; }
   const Path&  startingLocation() const { return _start; }

   void  allowType( const String& type ) { _types.pushBack(type); }
   void  allowAll() { _types.clear(); }
         Vector<String>&  allowedTypes()       { return _types; }
   const Vector<String>&  allowedTypes() const { return _types; }

   void  canSelectDirectories( bool v ) { _canSelectDirs = v; }
   bool  canSelectDirectories() const { return _canSelectDirs; }

   void  canSelectFiles( bool v ) { _canSelectFiles = v; }
   bool  canSelectFiles() const { return _canSelectFiles; }

   void  showHidden( bool v ) { _showHidden = v; }
   bool  showHidden() const { return _showHidden; }

   void  canShowHidden( bool v ) { _canShowHidden = v; }
   bool  canShowHidden() const { return _canShowHidden; }

   void  canCreateDirectories( bool v ) { _canCreateDirs = v; }
   bool  canCreateDirectories() const { return _canCreateDirs; }

   void  resolveAliases( bool v ) { _resolveAliases = v; }
   bool  resolveAliases() const { return _resolveAliases; }

   void  multipleSelect( bool v ) { _multiSelect = v; }
   bool  multipleSelect() const { return _multiSelect; }

   FUSION_DLL_API void  ask();

   void  addOnCancel( const DialogDelegate& d ) { _onCancelDel.addDelegate(d); }
   void  removeOnCancel( const DialogDelegate& d ) { _onCancelDel.removeDelegate(d); }
   FUSION_DLL_API virtual void onCancel();

   void  addOnConfirm( const DialogDelegate& d ) { _onConfirmDel.addDelegate(d); }
   void  removeOnConfirm( const DialogDelegate& d ) { _onConfirmDel.removeDelegate(d); }
   FUSION_DLL_API virtual void onConfirm();

   uint  numPaths() const { return uint(_paths.size()); }
   const Path&  path( uint idx ) const { return _paths[idx]; }
   const Path&  path()           const { return _paths[0];   }
   void  add( const Path& path ) { _paths.pushBack(path); }

   // VM.
   FUSION_DLL_API virtual const char*  meta() const;
   FUSION_DLL_API virtual void  init( VMState* vm );
   FUSION_DLL_API virtual bool  performGet( VMState* vm );
   FUSION_DLL_API virtual bool  performSet( VMState* vm );

protected:

   /*----- types -----*/

   typedef Delegate1List< FileDialog& >  DialogDelegateList;

   /*----- data members -----*/

   Type    _type;    //!< The type of file dialog (open or save).

   String  _title;   //!< The title of the dialog window.
   String  _prompt;  //!< A very short text displayed on the button.
   String  _message; //!< A short message that can be displayed in the dialog window.

   Path    _start;   //!< The starting location; if it's a file, will set the appropriate dialog fields at startup.

   Vector<String>  _types;  //!< The allowed types.

   bool    _canSelectDirs;  //!< Indicates that we can select directories.
   bool    _canSelectFiles; //!< Indicates that we can select files.
   bool    _showHidden;     //!< Indicates whether or not the dialog displays normally hidden files/dirs.
   bool    _canShowHidden;  //!< Indicates if the user can display hidden files/dirs if desired.
   bool    _canCreateDirs;  //!< Indicates if the user can create directories or not.
   bool    _resolveAliases; //!< Indicates if files exposes aliases as their destination or not.
   bool    _multiSelect;    //!< Indicates if we the user can select multiple files or not.

   DialogDelegateList  _onCancelDel; //!< The C++ delegates called when the user cancels the dialog.
   VMRef               _onCancelRef; //!< The VM delegates called when the user cancels the dialog.

   DialogDelegateList  _onConfirmDel; //!< The C++ delegates called when the user confirms the dialog.
   VMRef               _onConfirmRef; //!< The VM delegates called when the user confirms the dialog.

   Vector<Path>        _paths;        //!< The paths for the user specified file(s).

}; //class FileDialog

NAMESPACE_END

#endif //FUSION_FILE_DIALOG_H
