//
// Fragment shader for simple forward rendering color.
//
uniform sampler2D colorTex;
uniform sampler2D normalTex;
uniform sampler2D dbrdfTex;

varying vec2 mapping;
varying vec3 position;

void main(void)
{
   // Output eye vector.
   vec3  o = normalize( gfxCameraPosition - position ); //aka k2

   // Normal computation.
   vec3 n        = texture2D( normalTex, mapping ).rgb * 2.0 - 1.0;
   n             = normalize((gfxWorldMatrix * vec4( n, 0.0 )).xyz);
   float o_dot_n = dot( o, n );

   // Lighting.
   vec3 lightColor = vec3( 1.0 );
   vec3 lightPos   = gfxCameraPosition + vec3(0.0,1.0,0.0);
   //vec3 lightPos   = vec3( 100.0, 250.0, 200.0 );
   vec3 l          = lightPos - position;
   vec3 i          = normalize( l ); //aka k1
   vec3 h          = normalize( i + o );
   float n_dot_h   = max( 0.0, dot(n, h) );
   float i_dot_n   = max( 0.0, dot(i, n) );
   float ii_dot_n  = max( 0.0, -dot(i, n) );
   //float ii_dot_n  = 1.0-max( 0.0, -dot(i, n) );

   //float ph       = texture2D( dbrdfTex, vec2( n_dot_h, 0.0/128.0 ) ).x;
   float att      = i_dot_n;
   float ph       = 1.0;
   //float att      = i_dot_n*0.7 + pow(n_dot_h,40.0) + 0.1;
   vec4 lighting  = vec4( lightColor*(ph*att+0.2*ii_dot_n), 1.0 );

   // Return final color.
   gl_FragColor = lighting*texture2D( colorTex, mapping );
}
