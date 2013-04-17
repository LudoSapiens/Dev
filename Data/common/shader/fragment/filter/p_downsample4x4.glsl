uniform sampler2D tex;
uniform vec2 texelSize;

void main( void )
{
   vec4 color;

   color  = texture2D(tex, gl_TexCoord[0].xy + vec2(-texelSize.x, -texelSize.y));
   color += texture2D(tex, gl_TexCoord[0].xy + vec2( texelSize.x, -texelSize.y));
   color += texture2D(tex, gl_TexCoord[0].xy + vec2(-texelSize.x,  texelSize.y));
   color += texture2D(tex, gl_TexCoord[0].xy + vec2( texelSize.x,  texelSize.y));
   color *= (1.0 / 4.0);

   gl_FragColor = color;
}
