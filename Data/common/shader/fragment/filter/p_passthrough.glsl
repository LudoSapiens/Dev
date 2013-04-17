uniform sampler2D tex;

void main( void )
{
#if 1
   vec4 color = texture2D(tex, gl_TexCoord[0].xy);
#else
   vec2 texDim      = vec2( 256.0, 256.0 );
   vec2 texelOffset = vec2( 0.5/texDim.x, 0.5/texDim.y );

   vec2 coord = gl_TexCoord[0].xy;

   // Perform bilinear by hand.
   vec4 bl = texture2D(tex, coord + vec2(-texelOffset.x, -texelOffset.y));
   vec4 br = texture2D(tex, coord + vec2( texelOffset.x, -texelOffset.y));
   vec4 tl = texture2D(tex, coord + vec2(-texelOffset.x,  texelOffset.y));
   vec4 tr = texture2D(tex, coord + vec2( texelOffset.x,  texelOffset.y));
   vec2 w  = fract( (coord * texDim) - vec2(0.5) );
   //w = vec2( 1.0 - w.x, 1.0 - w.y );
   vec4 top = tl * (1.0 - w.x) + tr * (w.x);
   vec4 bot = bl * (1.0 - w.x) + br * (w.x);
   vec4 color = bot * (1.0 - w.y) + top * (w.y);
   //vec4 color = (tl + tr + bl + br) * 0.25;
   //vec4 color = texture2D(tex, coord);
#endif
   gl_FragColor = color;
}
