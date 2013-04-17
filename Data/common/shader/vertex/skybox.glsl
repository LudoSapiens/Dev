varying vec3 position;

void main(void)
{
   position    = gl_Vertex.xyz;
   vec4 wpos   = ( gfxWorldMatrix * gl_Vertex ) + vec4( gfxCameraPosition, 0.0 );
   gl_Position = gfxProjectionMatrix * gfxViewMatrix * wpos;
}
