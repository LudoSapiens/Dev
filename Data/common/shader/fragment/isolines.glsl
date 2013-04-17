//
// Isolines fragment shader with Gooch shading and texture mapping.
//
uniform sampler2D colorTex;
uniform vec4 frontColor;
uniform vec4 backColor;
varying vec3 normal;
varying vec2 mapping;

void main(void)
{
   // Compute lighting.
   vec3  lightPos = vec3( 1.0, 5.0, 20.0 );
   float nl       = max( dot( normalize( normal ), normalize( lightPos ) ), 0.0 );  
   vec4 lighting  = mix( backColor, frontColor, nl );

   // Default values.
   vec2 pos;
   vec2 uv      = mapping;
   vec4 color   = vec4( 1.0 );
   vec4 linecol = vec4( 0.0, 0.0, 0.0, 1.0 );
   
   // Derivative.
   vec2 d = max( abs( dFdx( mapping ) ), abs( dFdy( mapping ) ) ) * 0.5;

   // Lines color.
   for( int i = 0; i < 1; i++ )
   {
      pos   = vec2( 0.501 ) - abs( vec2( 0.5 ) - fract( uv ) );
      pos   = smoothstep( d, 4.0*d, pos );
      color = min( color, mix( linecol, vec4( 1.0 ), pos.x*pos.y ) );
      
      d  = d * 1.8;
      uv = uv * 2.0;
      linecol = mix( linecol, vec4( 1.0 ), 0.8 );
   }
   
   // Return final color.
   gl_FragColor = color*lighting*texture2D( colorTex, mapping );
}
