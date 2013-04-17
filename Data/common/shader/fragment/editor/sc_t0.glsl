uniform sampler2D colorTex0;
uniform vec4 color;
varying vec4 vcolor;

void main( void )
{
   gl_FragColor = vcolor * color * texture2D( colorTex0, gl_PointCoord );
}
