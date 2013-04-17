//
// Distance field GLSL fragment shader.
//

varying vec2 mapping;

uniform sampler2D colorTex;

uniform vec4  color;

void main( void )
{
   //const float scaleFactor = 64.0; // Normal  (1024/16.0)==(texSize/(2.0*maxDistance)
   const float scaleFactor = 48.0; // Sharp
   //const float scaleFactor = 32.0; // Extra sharp
   float w = fwidth(mapping.x)*scaleFactor;

   // Main
   vec4  t = texture2D( colorTex, mapping );
   float d = t.a;

   //float f = smoothstep( 0.5-w, 0.5+w, d );
   float f = clamp((d - (0.5-w))/(2.0*w), 0.0, 1.0);
   vec4 c  = color*f;

   // Border
   //float b = clamp((d - (0.3-w))/(2.0*w), 0.0, 1.0);
   //c       = c + vec4(0.0,0.0,0.0,1.0)*(b*(1.0-c.a));

   // Shadow
   //vec4  t2 = texture2D( colorTex, mapping+vec2(-0.004,0.004) );
   //float d2 = t2.a;
   //float s  = clamp((d2 - (0.5-0.3))/(2.0*0.3), 0.0, 1.0);
   //c        = c + vec4(0.0,0.0,0.0,0.6)*(s*(1.0-c.a));

   gl_FragColor = c;
}
