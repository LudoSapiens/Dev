uniform sampler2D tex;
uniform vec2 texelSize;

void main( void )
{
#if 1
   vec4 color1;
   vec4 color2;
   color1  = texture2D(tex, gl_TexCoord[0].xy + vec2( -texelSize.x, -texelSize.y )*1.5);
   color1 -= texture2D(tex, gl_TexCoord[0].xy + vec2(  texelSize.x,  texelSize.y )*1.5);
   color2  = texture2D(tex, gl_TexCoord[0].xy + vec2(  texelSize.x, -texelSize.y )*1.5);
   color2 -= texture2D(tex, gl_TexCoord[0].xy + vec2( -texelSize.x,  texelSize.y )*1.5);
   
   //color1  = texture2D(tex, gl_TexCoord[0].xy + vec2( -texelSize.x*0.5, -texelSize.y*0.5 ));
   //color1 -= texture2D(tex, gl_TexCoord[0].xy + vec2(  texelSize.x*1.5,  texelSize.y*1.5 ));
   //color2  = texture2D(tex, gl_TexCoord[0].xy + vec2(  texelSize.x*1.5, -texelSize.y*0.5 ));
   //color2 -= texture2D(tex, gl_TexCoord[0].xy + vec2( -texelSize.x*0.5,  texelSize.y*1.5 ));

   vec4 color = (abs(color1)+abs(color2))*0.5;
   color.a = color.r;
#endif

#if 0
   vec4 p1 = texture2D(tex, gl_TexCoord[0].xy + vec2( -texelSize.x, -texelSize.y ));
   vec4 p2 = texture2D(tex, gl_TexCoord[0].xy + vec2( 0.0, -texelSize.y ));
   vec4 p3 = texture2D(tex, gl_TexCoord[0].xy + vec2( texelSize.x, -texelSize.y ));
   
   vec4 p4 = texture2D(tex, gl_TexCoord[0].xy + vec2( -texelSize.x, 0.0 ));
   vec4 p6 = texture2D(tex, gl_TexCoord[0].xy + vec2( texelSize.x, 0.0 ));
   
   vec4 p7 = texture2D(tex, gl_TexCoord[0].xy + vec2( -texelSize.x, texelSize.y ));
   vec4 p8 = texture2D(tex, gl_TexCoord[0].xy + vec2( 0.0, texelSize.y ));
   vec4 p9 = texture2D(tex, gl_TexCoord[0].xy + vec2( texelSize.x, texelSize.y ));

   vec4 color = abs( (p1+2.0*p2+p3)-(p7+2.0*p8+p9) ) + abs( (p3+2.0*p6+p9)-(p1+2.0*p4+p7));
#endif

   if( color.r < 0.01 )
      discard;

   //gl_FragColor = color;
   //gl_FragColor = color*vec4(0.2, 0.2, 2.2, 1.0);
   gl_FragColor = color*vec4(0.2*4.0, 0.2*4.0, 2.2*4.0, 1.0);
}
