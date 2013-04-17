local params = ... or {}
local color    = params.color    or 2
local dotColor = params.dotColor or 1
local dotSize  = params.dotSize  or 0.1
local function dot( x, y, z )
   local pos = vec3( x, y, z )
   block{
      pos - dotSize,
      pos + dotSize,
      c=0x000, g=1, id=dotColor,
   }
end

differenceBegin()
   blocksBegin()
      block{
         {-1,-1,-1},
         { 1,-1,-1},
         {-1, 1,-1},
         { 1, 1,-1},
         {-1,-1, 1},
         { 1,-1, 1},
         {-1, 1, 1},
         { 1, 1, 1},
         m=0, g=0, c=0x000, s=0xffffff, id=color,
      }
   blocksEnd()
   blocksBegin()
      -- One at -X
      dot(-1.0, 0.0, 0.0 )
      -- Six at +X
      dot( 1.0,-0.5,-0.5 )
      dot( 1.0,-0.5, 0.0 )
      dot( 1.0,-0.5, 0.5 )
      dot( 1.0, 0.5,-0.5 )
      dot( 1.0, 0.5, 0.0 )
      dot( 1.0, 0.5, 0.5 )
      -- Two at -Y
      dot(-0.5,-1.0, 0.5 )
      dot( 0.5,-1.0,-0.5 )
      -- Five at +Y
      dot(-0.5, 1.0,-0.5 )
      dot(-0.5, 1.0, 0.5 )
      dot( 0.0, 1.0, 0.0 )
      dot( 0.5, 1.0,-0.5 )
      dot( 0.5, 1.0, 0.5 )
      -- Three at -Z
      dot(-0.5, 0.5,-1.0 )
      dot( 0.0, 0.0,-1.0 )
      dot( 0.5,-0.5,-1.0 )
      -- Four at +Z
      dot(-0.5,-0.5, 1.0 )
      dot(-0.5, 0.5, 1.0 )
      dot( 0.5,-0.5, 1.0 )
      dot( 0.5, 0.5, 1.0 )
   blocksEnd()
differenceEnd()
