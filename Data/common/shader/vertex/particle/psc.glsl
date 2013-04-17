varying vec4 color;

void main(void)
{
   gl_Position   = gfxWorldViewProjectionMatrix * gl_Vertex;
   gl_PointSize  = gl_MultiTexCoord1.x;
   color         = gl_Color;
}
