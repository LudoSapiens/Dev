# Is the following useful?
bl_info = {
   "name": "Ludo Sapiens I/O Utilities",
   "author": "Jocelyn Houle",
   "version": (0,2),
   "blender": (2, 5, 7),
   "api": 35622,
   "location": "File > Import-Export > Ludo Sapiens",
   "description": "Import/Export Ludo Sapiens formats (.mesh format)",
   "warning": "",
   "wiki_url": "",
   "tracker_url": "",
   #"support": "COMMUNITY"
   "category": "Import-Export"
}

# To support reload properly, try to access a package var,
# if it's there, reload everything
if "bpy" in locals():
    import imp
    if "import_trimesh" in locals():
        imp.reload(export_ply)
    if "export_trimesh" in locals():
        imp.reload(import_ply)

import os
import bpy
from bpy.props import BoolProperty, CollectionProperty, EnumProperty, StringProperty
from bpy_extras.io_utils import ImportHelper, ExportHelper

#-----------------------------------------------------------------------------
class ImportLudoTrimesh( bpy.types.Operator, ImportHelper ):
    '''Import Ludo Sapiens trimesh format'''
    bl_idname    = "import_mesh.trimesh"
    bl_label     = "Import Ludo Sapiens trimesh"
    filename_ext = ".mesh"

    @classmethod
    def poll( cls, context ):
        ob = context.active_object
        return (ob and ob.type == 'MESH')

    def execute( self, context ):
        import io_ludo.import_trimesh
        return io_ludo.import_trimesh.load( self, context, **self.properties )

#-----------------------------------------------------------------------------
class ExportLudoTrimesh( bpy.types.Operator, ExportHelper ):
    '''Export Ludo Sapiens trimesh format'''
    bl_idname = "export_mesh.trimesh"
    bl_label = "Export Ludo Sapiens trimesh"

    filename_ext = ".mesh"

    spaceList = [
       ('LUDO_REF_LOCAL' ,'World space',"Create data relative to the world"),
       ('LUDO_REF_GLOBAL','Local space',"Create data relative to the object (its transform will be meaningless)"),
    ]
    space = EnumProperty( name        = 'Origin',
                          description = 'Set the origin of the exported data',
                          items       = spaceList,
                          default     = 'LUDO_REF_GLOBAL' )

    fixYUp = BoolProperty( name="Y axis up", description="Force Y axis to point up", default=False )

    @classmethod
    def poll(cls, context):
        obj = context.active_object
        return (obj and (obj.type == 'MESH' or obj.type == 'TEXT' or obj.type == 'CURVE'))

    def execute(self, context):
        import io_ludo.export_trimesh
        return io_ludo.export_trimesh.save( self, context, **self.properties )

#-----------------------------------------------------------------------------
def menu_func_import(self, context):
    self.layout.operator( ImportLudoTrimesh.bl_idname, text="Ludo Sapiens trimesh (.mesh)" )


#-----------------------------------------------------------------------------
def menu_func_export(self, context):
    self.layout.operator( ExportLudoTrimesh.bl_idname, text="Ludo Sapiens trimesh (.mesh)" )


#-----------------------------------------------------------------------------
def register():
    bpy.utils.register_module(__name__)
    #bpy.types.INFO_MT_file_import.append( menu_func_import )
    bpy.types.INFO_MT_file_export.append( menu_func_export )


#-----------------------------------------------------------------------------
def unregister():
    bpy.utils.unregister_module(__name__)
    #bpy.types.INFO_MT_file_import.remove( menu_func_import )
    bpy.types.INFO_MT_file_export.remove( menu_func_export )

#-----------------------------------------------------------------------------
if __name__ == "__main__":
    register()
