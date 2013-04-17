varying in vec2 mapping[3];
varying in vec3 normal[3];
varying in vec3 position[3];

varying out vec2 mappingGS;
varying out vec3 normalGS;
varying out vec3 positionGS;

varying out vec3 dist;

void main(void)
{
   //vec2 scale = vec2( 512.0, 720.0 );
   vec2 scale = vec2( 1.0, 1.0 );
   vec2 p0 = scale * gl_PositionIn[0].xy/gl_PositionIn[0].w;
   vec2 p1 = scale * gl_PositionIn[1].xy/gl_PositionIn[1].w;
   vec2 p2 = scale * gl_PositionIn[2].xy/gl_PositionIn[2].w;
   vec2 v0 = p2-p1;
   vec2 v1 = p2-p0;
   vec2 v2 = p1-p0;
   float area = abs(v1.x*v2.y - v1.y * v2.x);

   mappingGS   = mapping[0];
   normalGS    = normal[0];
   positionGS  = position[0];
   dist        = vec3(area/length(v0),0,0);;
   gl_Position = gl_PositionIn[0];
   EmitVertex();

   mappingGS   = mapping[1];
   normalGS    = normal[1];
   positionGS  = position[1];
   dist        = vec3(0,area/length(v1),0);
   gl_Position = gl_PositionIn[1];
   EmitVertex();

   mappingGS   = mapping[2];
   normalGS    = normal[2];
   positionGS  = position[2];
   dist        = vec3(0,0,area/length(v2));
   gl_Position = gl_PositionIn[2];
   EmitVertex();

   EndPrimitive();
}
