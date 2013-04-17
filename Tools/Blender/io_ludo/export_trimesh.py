"""
This script exports the selected Mesh to Ludo Sapiens trimesh format file.
"""

import bpy

#-----------------------------------------------------------------------------
# Converts a Blender position (right-handed, with Y pointing up) to our
# standard (right-handed, but with Z pointing up).
# This is simply equivalent to a rotation of -90' around the X axis.
def YUpToXUp( pos ):
   return [ pos[0], pos[2], -pos[1] ]

#-----------------------------------------------------------------------------
def passThrough( pos ):
   return pos

#-----------------------------------------------------------------------------
def key2d(v):
   return round(v[0], 6), round(v[1], 6)

#-----------------------------------------------------------------------------
def dumpIndexArray( f, indent, mesh ):
   #print("DumpIndexArray", len(mesh.faces) )
   for face in mesh.faces:
      verts = face.vertices_raw
      if len(verts) == 4:
         a = verts[0] + 1
         b = verts[1] + 1
         c = verts[2] + 1
         d = verts[3] + 1
         f.write( indent+"%d, %d, %d,  %d, %d, %d,\n" % (a, b, c, a, c, d) )
      else:
         a = verts[0] + 1
         b = verts[1] + 1
         c = verts[2] + 1
         f.write( indent+"%d, %d, %d,\n" % (a, b, c) )

#-----------------------------------------------------------------------------
def dumpVertexArray( f, indent, mesh ):
   #print("DumpVertexArray", len(mesh.faces) )
   f.write( indent+"format = { Attribute.POSITION, Attribute.NORMAL },\n" )
   for vertex in mesh.vertices:
      p = YUpToXUp( vertex.co )
      n = YUpToXUp( vertex.normal )
      f.write( indent+"%f,%f,%f,  %f,%f,%f,\n" % (p[0], p[1], p[2], n[0], n[1], n[2]) )

#-----------------------------------------------------------------------------
# This routine dumps proper vertices+normals data in the order they are
# encountered in the faces, but also creates a remapping table to reorder
# the indices for a later pass.
# In Blender, smooth vertices use their vertex normals, but the others
# use their face normals.
# When the latter occurs, we need to create a new vertex that is different
# than the former.
# The encoding we use for the dictionary key is a tuple of the form:
#  (vertexID, faceID_for_normal)
# The vertexID is just the original vertex data we dumped.
# The faceID_for_normal is the face which normal we must use, or -1 if
# we should reuse the vertex' normal (i.e. a smooth vertex).
# The value store at that key is simply the actual vertex index we stored that data at.
def dumpVertices( f, indent, mesh, fixYUp=False ):
   if fixYUp:
      fix = YUpToXUp
   else:
      fix = passThrough
   f.write( indent+"format = { Attribute.POSITION, Attribute.NORMAL },\n" )
   remap = {}
   for face in mesh.faces:
      if face.use_smooth:
         ni    = -1
         verts = face.vertices
         for vi in verts:
            key   = ( vi, ni )
            s     = len(remap)
            value = remap.setdefault( key, s )
            if value == s:
               # The key,value was added to remap, so dump the vertex+normal.
               vertex = mesh.vertices[vi]
               p      = fix(vertex.co)
               n      = fix(vertex.normal)
               f.write( indent+"%f,%f,%f,  %f,%f,%f,\n" % (p[0], p[1], p[2], n[0], n[1], n[2]) )
      else:
         ni    = face.index
         n     = fix(face.normal)
         verts = face.vertices
         for vi in verts:
            key   = ( vi, ni )
            s     = len(remap)
            value = remap.setdefault( key, s )
            if value == s:
               # The key,value was added to remap, so dump the vertex+normal.
               vertex = mesh.vertices[vi]
               p      = fix(vertex.co)
               f.write( indent+"%f,%f,%f,  %f,%f,%f,\n" % (p[0], p[1], p[2], n[0], n[1], n[2]) )
   return remap
#-----------------------------------------------------------------------------
def dumpVerticesUV( f, indent, mesh, fixYUp=False ):
   if len(mesh.uv_textures) == 0:
      return dumpVertices( f, indent, mesh, fixYUp )

   uv_layer = mesh.uv_textures.active.data

   if fixYUp:
      fix = YUpToXUp
   else:
      fix = passThrough
   f.write( indent+"format = { Attribute.POSITION, Attribute.NORMAL, Attribute.MAPPING },\n" )
   remap = {}
   for face in mesh.faces:
      if face.use_smooth:
         ni    = -1
         verts = face.vertices
         uvs   = uv_layer[face.index].uv
         for i, vi in enumerate(verts):
            uv    = uvs[i]
            uvk   = key2d(uv)
            key   = ( vi, ni, uvk )
            s     = len(remap)
            value = remap.setdefault( key, s )
            if value == s:
               # The key,value was added to remap, so dump the vertex+normal.
               vertex = mesh.vertices[vi]
               p      = fix(vertex.co)
               n      = fix(vertex.normal)
               f.write( indent+"%f,%f,%f,  %f,%f,%f, %f,%f,\n" % (p[0], p[1], p[2], n[0], n[1], n[2], uv[0], uv[1]) )
      else:
         ni    = face.index
         n     = fix(face.normal)
         verts = face.vertices
         uvs   = uv_layer[ni].uv
         for i, vi in enumerate(verts):
            uv    = uvs[i]
            uvk   = key2d(uv)
            key   = ( vi, ni, uvk )
            s     = len(remap)
            value = remap.setdefault( key, s )
            if value == s:
               # The key,value was added to remap, so dump the vertex+normal.
               vertex = mesh.vertices[vi]
               p      = fix(vertex.co)
               f.write( indent+"%f,%f,%f,  %f,%f,%f, %f,%f,\n" % (p[0], p[1], p[2], n[0], n[1], n[2], uv[0], uv[1]) )
   return remap


#-----------------------------------------------------------------------------
# This routines dumps the triangles' indices using the remapping table
# dumped by dumpVertices().
def dumpIndices( f, indent, mesh, remap ):
   #print("DumpIndices", len(mesh.faces) )
   for face in mesh.faces:
      if face.use_smooth:
         ni = -1
      else:
         ni = face.index
      verts = face.vertices
      if len(verts) == 4:
         a = remap[(verts[0], ni)] + 1
         b = remap[(verts[1], ni)] + 1
         c = remap[(verts[2], ni)] + 1
         d = remap[(verts[3], ni)] + 1
         f.write( indent+"%d, %d, %d,  %d, %d, %d,\n" % (a, b, c, a, c, d) )
      else:
         a = remap[(verts[0], ni)] + 1
         b = remap[(verts[1], ni)] + 1
         c = remap[(verts[2], ni)] + 1
         f.write( indent+"%d, %d, %d,\n" % (a, b, c) )

#-----------------------------------------------------------------------------
def dumpIndicesUV( f, indent, mesh, remap ):
   if len(mesh.uv_textures) == 0:
      return dumpIndices( f, indent, mesh, remap )

   uv_layer = mesh.uv_textures.active.data

   for face in mesh.faces:
      if face.use_smooth:
         ni = -1
      else:
         ni = face.index
      verts = face.vertices
      uvs   = uv_layer[face.index].uv
      if len(verts) == 4:
         a = remap[(verts[0], ni, key2d(uvs[0]))] + 1
         b = remap[(verts[1], ni, key2d(uvs[1]))] + 1
         c = remap[(verts[2], ni, key2d(uvs[2]))] + 1
         d = remap[(verts[3], ni, key2d(uvs[3]))] + 1
         f.write( indent+"%d, %d, %d,  %d, %d, %d,\n" % (a, b, c, a, c, d) )
      else:
         a = remap[(verts[0], ni, key2d(uvs[0]))] + 1
         b = remap[(verts[1], ni, key2d(uvs[1]))] + 1
         c = remap[(verts[2], ni, key2d(uvs[2]))] + 1
         f.write( indent+"%d, %d, %d,\n" % (a, b, c) )


#-----------------------------------------------------------------------------
def getMesh():
   if bpy.ops.object.mode_set.poll():
      bpy.ops.object.mode_set( mode='OBJECT' )
   bpy.ops.object.join()
   return bpy.context.active_object.to_mesh( bpy.context.scene, True, "PREVIEW" ), 2
   #for obj in bpy.context.selected_objects:
   #   if obj.type == 'MESH' or obj.type == 'TEXT' or obj.type == 'CURVE':
   #      #matrix = obj.matrix_world
   #      return obj.to_mesh( bpy.context.scene, True, "PREVIEW" )
   #      #obj.join()
   #return None

#-----------------------------------------------------------------------------
def save( operator, context, filepath="", **ignore ):
   mesh, nUndo = getMesh()
   #print("Export trimesh", mesh)
   if not mesh:
      print("ERROR - No mesh selected.")
      return

   # Dump the mesh into a file.
   indent = "   "
   f = open( filepath, "w" )

   if True:
      #print("Dumping vertex array")
      f.write( "local v = {\n" )
      remap = dumpVerticesUV( f, indent, mesh, operator.fixYUp )
      f.write( "}\n" )

      f.write( "\n" )

      #print("Dumping index array")
      f.write( "local i = {\n" )
      dumpIndicesUV( f, indent, mesh, remap )
      f.write( "}\n" )

      f.write( "\n" )
   else:
      """Old way; deprecated"""
      #print("Dumping vertex array")
      f.write( "local v = {\n" )
      dumpVertexArray( f, indent, mesh )
      f.write( "}\n" )

      f.write( "\n" )

      #print("Dumping index array")
      f.write( "local i = {\n" )
      dumpIndexArray( f, indent, mesh )
      f.write( "}\n" )

      f.write( "\n" )

   #print("Dumping instantiation")
   f.write( "trimesh{ indices=i, vertices=v }\n" )

   f.close()

   #print("Undoing "+str(nUndo)+" operations")
   while nUndo > 0:
      bpy.ops.ed.undo()
      nUndo -= 1

   return {'FINISHED'}
