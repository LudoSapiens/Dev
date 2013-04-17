uniform sampler2D tex;
uniform vec2 disp;

void main( void )
{
   vec4 color;
   color  = 0.074876 * texture2D(tex, gl_TexCoord[0].xy + vec2(0.0, -disp.y));
   color += 0.313037 * texture2D(tex, gl_TexCoord[0].xy + vec2(0.0, -disp.x));
   color += 0.224174 * texture2D(tex, gl_TexCoord[0].xy);
   color += 0.313037 * texture2D(tex, gl_TexCoord[0].xy + vec2(0.0, disp.x));
   color += 0.074876 * texture2D(tex, gl_TexCoord[0].xy + vec2(0.0, disp.y));
   gl_FragColor = color;
}
