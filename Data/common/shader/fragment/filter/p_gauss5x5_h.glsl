uniform sampler2D tex;
uniform float disp;

void main( void )
{
   vec4 color;
   color  = (5.0/16.0) * texture2D(tex, gl_TexCoord[0].xy + vec2(-disp, 0.0));
   color += (6.0/16.0) * texture2D(tex, gl_TexCoord[0].xy);
   color += (5.0/16.0) * texture2D(tex, gl_TexCoord[0].xy + vec2( disp, 0.0));
   gl_FragColor = color;
}
