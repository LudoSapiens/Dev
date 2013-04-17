varying vec3 position;
varying vec3 normal;
varying vec3 screenPosition;

uniform vec3      gfxCameraPosition;
uniform sampler2D tex;
uniform vec2      texelSize;
uniform float     eta;

void main( void )
{
   vec3 i = normalize( gfxCameraPosition - position );
   vec3 n = normalize( normal );
   vec3 r = refract( i, n, eta );

   vec2 d = r.xy * 0.01;

   vec2 coord = (screenPosition.xy/screenPosition.z) + d;

#if 0
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

   //gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
   //gl_FragColor = color;
   //gl_FragColor.xy = distortion.xy;
   gl_FragColor = texture2D(tex, coord);
   //gl_FragColor.xy = gl_FragCoord.xy / 1024.0;
}
