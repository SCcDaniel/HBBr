
#ifndef _BASE_PASS_VERTEX_SHADER_HLSL
#define _BASE_PASS_VERTEX_SHADER_HLSL

#ifndef CustomVSMain
VSToPS VSMain(VSInput IN)
{
    VSToPS OUT;
    InitVSToPS(OUT);

    //Transform MVP
    OUT.LocalPosition = IN.Position;
    float4 WorldPosition = mul(World, float4(IN.Position,1.0));
    OUT.SVPosition = mul(VP, WorldPosition);
    OUT.WorldPosition = WorldPosition.xyz;

    //Transform TangentSpace
    OUT.WorldNormal =  normalize(DirectionLocalToWorld(IN.Normal));
    OUT.WorldTangent =  normalize(DirectionLocalToWorld(IN.Tangent));
    OUT.WorldBitangent = normalize(cross(IN.Normal,IN.Tangent));

    //Texcoord 0123
    OUT.Texcoord01 = IN.Texcoord01;
    OUT.Texcoord23 = IN.Texcoord23;

    //Vertex Color
    OUT.Color = IN.Color;

    //Others
    OUT.CameraVector = (OUT.WorldPosition.xyz - CameraPos.xyz);
    
#ifdef DefineVert
    vert(IN,OUT);
#endif

    return OUT;
}
#endif

#endif