uniform sampler2D tex;

const vec3 grayScaleWeights = vec3(0.30, 0.59, 0.11);

void main( void )
{
   vec4 color = texture2D(tex, gl_TexCoord[0].xy);
   float luminance = dot(color.xyz, grayScaleWeights);
   gl_FragColor = vec4(luminance, luminance, luminance, 1.0);
}
