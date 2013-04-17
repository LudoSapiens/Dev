//
// GLSL fragment shader for testing.
//
varying vec2 mapping;
varying vec3 position;
varying vec3 normal;
varying vec4 tangent;
uniform vec4 color;
uniform vec4 clipPlane;
uniform sampler2D colorTex;

void main( void )
{
   if( (dot(clipPlane.xyz, position) + clipPlane.w) < 0.0 )
   {
      discard;
   }
   float ao = length( normal );
   gl_FragColor = color*vec4(ao,ao,ao,1.0)*texture2D( colorTex, (mapping*0.125).yx );
}
