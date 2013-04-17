//
// colorTex GLSL fragment shader.
//

varying vec2 mapping;
uniform vec4 color;
uniform sampler2D colorTex;

void main( void )
{
   gl_FragColor = color * texture2D( colorTex, mapping );
}
