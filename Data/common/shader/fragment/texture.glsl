//
// Texture GLSL fragment shader.
//

varying vec2 mapping;

uniform sampler2D colorTex;

void main( void )
{
   gl_FragColor = texture2D( colorTex, mapping );
}
