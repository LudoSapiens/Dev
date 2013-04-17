varying vec2 mapping;

void main( void )
{
   gl_Position = gfxWorldViewProjectionMatrix * gl_Vertex;
   mapping     = gl_Vertex.xy * 0.5 + 0.5;
}
