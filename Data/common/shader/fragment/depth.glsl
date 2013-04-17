//
// Depth passthrough GLSL pixel shader.
//

varying vec3 position;

void main( void )
{
   gl_FragColor = position.zzzz;
}
