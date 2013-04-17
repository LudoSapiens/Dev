uniform sampler2D tex;
uniform sampler2D dev;

uniform vec2 texDim;
uniform vec2 texelOffset;

void main( void )
{
   vec2 deviation = texture2D(dev, gl_TexCoord[0].xy).xy;
   //deviation *= 1.0/16.0;
   deviation *= 0.25;
   //deviation *= 0.5;
   //deviation *= 0.0;

   vec2 coord = gl_TexCoord[0].xy + deviation;
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

   gl_FragColor = color;
}
