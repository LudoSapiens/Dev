//
// Generic 4-spotlight shader.
//
// Notes:
//  * All single-character vectors are normalized
//

#version 120

uniform float     dbrdfId;
uniform vec4      color;
uniform vec3      gfxCameraPosition;
uniform sampler2D dbrdfTex;
//uniform sampler2D colorTex;

// Light information.
uniform float numLights;
uniform sampler2D light0_depth;
uniform vec3      light0_color;
uniform vec3      light0_position;
uniform mat4      light0_matrix;
uniform sampler2D light1_depth;
uniform vec3      light1_color;
uniform vec3      light1_position;
uniform mat4      light1_matrix;
uniform sampler2D light2_depth;
uniform vec3      light2_color;
uniform vec3      light2_position;
uniform mat4      light2_matrix;
uniform sampler2D light3_depth;
uniform vec3      light3_color;
uniform vec3      light3_position;
uniform mat4      light3_matrix;

const float SHADOW_SIZE     = 1024.0/2.0;
const float SHADOW_SIZE_INV = 1.0/SHADOW_SIZE;

varying vec2 mapping;
varying vec3 position;
varying vec3 normal;
varying vec4 tangent;

float vsm( vec2 moments, float z )
{
   float variance = max( moments.y - (moments.x*moments.x), 0.00004 );
   float dl       = max( 0.0, z - moments.x );
   float sh       = variance / (variance + dl*dl);
   return smoothstep( 0.5, 1.0, sh );
}

#if 0
float esm( vec2 moments, float t )
{
   float z  = dot( m.xy, vec2( 255.0*16.0/4095.0, 255.0/4095.0) );
   float zz = dot( m.zw, vec2( 255.0*16.0/4095.0, 255.0/4095.0) );
   
   float ez = m.x;
   float sh = min( 1.0, pow( exp2(-10.0*t)*ez, 40.0 ) );
   float sh = min( 1.0, exp2(-80.0*t)*(ez) );
   float sh = min( 1.0, exp2(200.0*(m.x-t)) );
}
#endif

float vsm_shadow( vec3 position, vec3 posToLight, mat4 light_matrix, sampler2D light_depth )
{
   const float dsf  = 0.01; // Some scale factor used for the 't' term.
   float t    = length( posToLight ) * dsf;
   vec4  lpos = light_matrix * vec4( position, 1.0 );
   vec2  t1   = fract( SHADOW_SIZE * (lpos.xy/lpos.w) );
   vec2  t0   = vec2( 1.0, 1.0 ) - t1;
   float d    = SHADOW_SIZE_INV * lpos.w;

   vec4 m = vec4( 0.0, 0.0, 0.0, 0.0 );
   m += texture2DProj( light_depth, lpos + vec4( 0.0, 0.0, 0.0, 0.0 ) ) * (t0.x * t0.y);
   m += texture2DProj( light_depth, lpos + vec4(   d, 0.0, 0.0, 0.0 ) ) * (t1.x * t0.y);
   m += texture2DProj( light_depth, lpos + vec4( 0.0,   d, 0.0, 0.0 ) ) * (t0.x * t1.y);
   m += texture2DProj( light_depth, lpos + vec4(   d,   d, 0.0, 0.0 ) ) * (t1.x * t1.y);
   //m.xy += m.zw/32.0;

   return vsm( m.xy, t );
}

float spot( vec3 i, mat4 light_matrix )
{
   vec3 spotDir  = transpose( light_matrix )[2].xyz;
   float spotAtt = dot( normalize(spotDir), -i );
   return smoothstep( 0.5, 0.6, spotAtt );
}

void main(void)
{
   // Common terms.
   vec3  o       = normalize( gfxCameraPosition - position ); //aka k2
   vec3  n       = normalize( normal );
   float o_dot_n = dot( o, n );
   
   vec4 allLights; // Accumulating all of the lights energy.

   // CameraLight.
   {
      float ph   = texture2D( dbrdfTex, vec2(o_dot_n, dbrdfId) ).x;
      float dist = distance( gfxCameraPosition, position );
      float att  = (1.0 - smoothstep( 3.5, 6.0, dist ))/(2.0-o_dot_n);
      allLights  = vec4( vec3(0.5)*ph*att, 1.0 );
   }

   //if( numLights >= 1.0 )
   {
      vec3  l       = light0_position - position;
      vec3  i       = normalize( l ); //aka k1
      vec3  h       = normalize( i + o );
      float n_dot_h = dot(n, h);
      float i_dot_n = max( 0.0, dot(i, n) );

      float sh      = vsm_shadow( position, l, light0_matrix, light0_depth );
      sh           *= spot( i, light0_matrix );
   
      float ph      = texture2D( dbrdfTex, vec2(n_dot_h, dbrdfId) ).x;      
      float att     = i_dot_n/(i_dot_n + o_dot_n - (i_dot_n*o_dot_n));
      att           = min( 1.0, att );
      allLights    += vec4( light0_color*ph*att*sh, 1.0 );
   }
#if 0   
   if( numLights >= 2.0 )
   {
      vec3  l       = light1_position - position;
      vec3  i       = normalize( l ); //aka k1
      vec3  h       = normalize( i + o );
      float n_dot_h = dot(n, h);
      float i_dot_n = max( 0.0, dot(i, n) );

      float sh      = vsm_shadow( position, l, light1_matrix, light1_depth );
      sh           *= spot( i, light1_matrix );

      float ph      = texture2D( dbrdfTex, vec2(n_dot_h, dbrdfId) ).x;
      float att     = i_dot_n/(i_dot_n + o_dot_n - (i_dot_n*o_dot_n));
      att           = min( 1.0, att );
      allLights    += vec4( light1_color*ph*att*sh, 1.0 );
   }
   if( numLights >= 3.0 )
   {
      vec3  l       = light2_position - position;
      vec3  i       = normalize( l ); //aka k1
      vec3  h       = normalize( i + o );
      float n_dot_h = dot(n, h);
      float i_dot_n = max( 0.0, dot(i, n) );

      float sh      = vsm_shadow( position, l, light2_matrix, light2_depth );
      sh           *= spot( i, light2_matrix );

      float ph      = texture2D( dbrdfTex, vec2(n_dot_h, dbrdfId) ).x;
      float att     = i_dot_n/(i_dot_n + o_dot_n - (i_dot_n*o_dot_n));
      att           = min( 1.0, att );
      allLights    += vec4( light2_color*ph*att*sh, 1.0 );
   }
   if( numLights >= 4.0 )
   {
      vec3  l       = light3_position - position;
      vec3  i       = normalize( l ); //aka k1
      vec3  h       = normalize( i + o );
      float n_dot_h = dot(n, h);
      float i_dot_n = max( 0.0, dot(i, n) );

      float sh      = vsm_shadow( position, l, light3_matrix, light3_depth );
      sh           *= spot( i, light3_matrix );
      
      float ph      = texture2D( dbrdfTex, vec2(n_dot_h, dbrdfId) ).x;
      float att     = i_dot_n/(i_dot_n + o_dot_n - (i_dot_n*o_dot_n));
      att           = min( 1.0, att );
      allLights    += vec4( light3_color*ph*att*sh, 1.0 );
   }
#endif

   allLights += vec4( 0.1, 0.1, 0.1, 0.0 ) * o_dot_n;

   gl_FragColor = allLights * color + color*color.a; //*texture2D( colorTex, mapping );
}
