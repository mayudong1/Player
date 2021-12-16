#version 330 core
precision mediump float;
in vec2 textureCoord;
uniform sampler2D sampler;
out vec4 fragColor;
uniform bool isVertical;
void main()
{
   vec2 tex_offset =vec2(1.0/100.0,1.0/100.0);
   vec4 orColor=texture(sampler,textureCoord);
   float orAlpha=orColor.a;
   float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
   vec3 color=orColor.rgb*weight[0];
   if(!isVertical)
   {
     for(int i=1;i<5;i++)
     {
       color+=texture(sampler,textureCoord+vec2(tex_offset.x * float(i), 0.0)).rgb*weight[i];
       color+=texture(sampler,textureCoord-vec2(tex_offset.x * float(i), 0.0)).rgb*weight[i];

     }
   }
   else
   {
      for(int i=1;i<5;i++)
      {
        color+=texture(sampler,textureCoord+vec2(0.0,tex_offset.y * float(i))).rgb*weight[i];
        color+=texture(sampler,textureCoord-vec2(0.0,tex_offset.y * float(i))).rgb*weight[i];
      }
   }
   fragColor=vec4(color,orAlpha);
}
