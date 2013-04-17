//
// Particle fragment shader
//

uniform sampler2D tex0;
uniform sampler2D depthTex;
varying vec2 mapping;
varying vec3 screenPosition;

void main(void)
{
   vec4  particleColor = gl_Color * texture2D( tex0, mapping );
   float sceneDepth    = texture2D( depthTex, screenPosition.xy ).x;
   float depthDiff     = sceneDepth + screenPosition.z;
   depthDiff           = min( 1.0, depthDiff*32.0 );
   
   gl_FragColor = particleColor * depthDiff;
}
