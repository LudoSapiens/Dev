uniform sampler2D baseTex;
uniform sampler2D bloomTex1;
uniform sampler2D bloomTex2;
float bloomFactor1 = 2.0;
float bloomFactor2 = 2.0;

void main( void )
{
   vec4 color = vec4(0.0);
   color += texture2D(baseTex, gl_TexCoord[0].xy);
   color += bloomFactor1 * texture2D(bloomTex1, gl_TexCoord[0].xy);
   color += bloomFactor2 * texture2D(bloomTex2, gl_TexCoord[0].xy);
   // Tone mapping?
   gl_FragColor = color;
}
