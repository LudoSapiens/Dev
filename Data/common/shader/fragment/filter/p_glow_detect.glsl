uniform sampler2D tex;

const float glowV = 0.9;
const float glowF = 1.3;
//const float glowV = 1.0;
//const float glowF = 1.0;

vec4 remap( vec4 color )
{
   return max( vec4(0.0, 0.0, 0.0, 0.0), color-vec4(glowV, glowV, glowV, 0.0) )*glowF;
}

void main( void )
{
   gl_FragColor = remap( texture2D(tex, gl_TexCoord[0].xy) );
}
