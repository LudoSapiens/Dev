//
// Particle fragment shader
//

uniform sampler2D tex0;
uniform sampler2D depthTex;
varying vec2 mapping;

void main(void)
{
   vec4  particleColor = gl_Color * texture2D( tex0, mapping );
   gl_FragColor = particleColor;
}
