uniform sampler2D colorTex0;

varying vec4 color;

void main( void )
{
   gl_FragColor = color * texture2D( colorTex0, gl_PointCoord );
   //gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
