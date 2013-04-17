varying vec4 vcolor;

void main(void)
{
   gl_Position  = gfxWorldViewProjectionMatrix * gl_Vertex;
   gl_PointSize = gl_MultiTexCoord1.x;
   vcolor       = gl_Color;
}
