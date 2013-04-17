void main( void )
{
   gl_Position = gfxWorldViewProjectionMatrix * gl_Vertex;
   gl_TexCoord[0] = gl_Vertex * 0.5 + 0.5;
}
