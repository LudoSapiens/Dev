//
// Texture GLSL fragment shader.
//

varying vec2 mappingGS;
varying vec3 normalGS;
varying vec3 positionGS;
varying vec3 dist;

uniform vec4 color;
uniform sampler2D colorTex0;

void main( void )
{
   // Output eye vector.
   vec3  o = normalize( gfxCameraPosition - positionGS ); //aka k2

   // Normal computation.
   vec3 n        = normalize(normalGS);
   float o_dot_n = dot( o, n );

   // Lighting.
   vec3 lightColor = vec3( 1.0 );
   vec3 lightPos   = gfxCameraPosition + vec3(0.0,1.0,0.0);
   //vec3 lightPos   = vec3( 100.0, 250.0, 200.0 );
   vec3 l          = lightPos - positionGS;
   vec3 i          = normalize( l ); //aka k1
   vec3 h          = normalize( i + o );
   float n_dot_h   = max( 0.0, dot(n, h) );
   float i_dot_n   = max( 0.0, dot(i, n) );
   float ii_dot_n  = max( 0.0, -dot(i, n) );
   //float ii_dot_n  = 1.0-max( 0.0, -dot(i, n) );

   float ph       = 1.0;
   //float ph       = texture2D( dbrdfTex, vec2( n_dot_h, 0.0/128.0 ) ).x;
   //float ph       = velvet(acos(n_dot_h));
   //float ph       = pow(n_dot_h, 100.0) + 0.3;

   float att      = i_dot_n*0.7 + 0.1;
   //float att      = i_dot_n;
   //float att      = i_dot_n*0.7 + pow(n_dot_h,40.0) + 0.1;
   //vec4 lighting  = vec4( lightColor*(ph*att+0.2*ii_dot_n), 1.0 );
   vec4 lighting  = vec4( lightColor*(ph*att), 1.0 );

   // Return final color.
   float d = min( dist.x, min( dist.y, dist.z ) );
   //float w = exp2(-1.0*d*d);
   float w = 1.0-smoothstep( 0.0, 1.0*fwidth(dist.y), d );
   gl_FragColor = mix( lighting*color*texture2D( colorTex0, mappingGS ), vec4(0.0,0.0,0.0,1.0), w );
}
