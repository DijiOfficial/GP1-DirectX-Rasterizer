//-------------------------------------------------
// Global Variables
//-------------------------------------------------
float4x4  gWorldViewProj  : WorldViewProjection;

Texture2D gDiffuseMap     : DiffuseMap;
Texture2D gNormalMap      : NormalMap;
Texture2D gSpecularMap    : SpecularMap;
Texture2D gGlossMap       : GlossMap;

float     gTime           : Time;
float3    gCameraPos      : CameraPos;
bool      gUseNormalMap   : UseNormalMap;

float gPI = 3.14159265358979311599796346854;
float gKD = 7.0f;
float gShininess = 25.0f;
float3 gAmbient = float3(0.03f, 0.03f, 0.03f);
float3 gLightDirection = float3(0.577f, -0.577f, 0.577f);
float gRotationSpeed = 0.785398f; //45degrees

//-------------------------------------------------
// Input/Output Structs
//-------------------------------------------------
struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Color    : COLOR;
    float2 Uv       : TEXCOORD;
    float3 Normal   : NORMAL;
    float3 Tangent  : TANGENT;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float3 Color    : COLOR;
    float2 Uv       : TEXCOORD;
    float3 Normal   : NORMAL;
    float3 Tangent  : TANGENT;
    float3 ViewDirection : VERTEX_TO_CAMERA;
};

//-------------------------------------------------
// Sampler States
//-------------------------------------------------
SamplerState samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap;
    AddressV = Wrap;
};

SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

SamplerState samAnisotropic
{
    Filter = ANISOTROPIC;
    AddressU = Wrap;
    AddressV = Wrap;
};

//-------------------------------------------------
// Partial Coverage
//-------------------------------------------------
DepthStencilState gDepthStencilState
{
    DepthEnable = true;
    DepthWriteMask = zero;
    DepthFunc = less;
    StencilEnable = false;

    StencilReadMask = 0xFF;
    StencilWriteMask = 0xFF;

    FrontFaceStencilFunc = always;
    BackFaceStencilFunc = always;

    FrontFaceStencilDepthFail = keep;
    BackFaceStencilDepthFail = keep;

    FrontFaceStencilPass = keep;
    BackFaceStencilPass = keep;

    FrontFaceStencilFail = keep;
    BackFaceStencilFail = keep;
};

DepthStencilState gNoDepthStencilState
{
    DepthEnable = true;
    DepthWriteMask = all;
    DepthFunc = less;
    StencilEnable = false;

    StencilReadMask = 0xFF;
    StencilWriteMask = 0xFF;

    FrontFaceStencilFunc = always;
    BackFaceStencilFunc = always;

    FrontFaceStencilDepthFail = keep;
    BackFaceStencilDepthFail = keep;

    FrontFaceStencilPass = keep;
    BackFaceStencilPass = keep;

    FrontFaceStencilFail = keep;
    BackFaceStencilFail = keep;
};

BlendState gBlendState
{
    BlendEnable[0] = true;
    SrcBlend = src_alpha;
    DestBlend = inv_src_alpha;
    BlendOp = add;
    SrcBlendAlpha = zero;
    DestBlendAlpha = zero;
    BlendOpAlpha = add;
    RenderTargetWriteMask[0] = 0x0F;
};

RasterizerState gRasterizerState
{
    CullMode = none;
    FrontCounterClockwise = false;
};

//-------------------------------------------------
// Vertex Shader
//-------------------------------------------------
float4x4 RotationMatrix(float yaw)
{
    float4x4 rotation = (float4x4)0;
    
    rotation[0][0] = cos(yaw);
    rotation[0][2] = -sin(yaw);
    rotation[1][1] = 1.0f;
    rotation[2][0] = sin(yaw);
    rotation[2][2] = cos(yaw);
    rotation[3][3] = 1.0f;
    
    return rotation;
}

VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    
    input.Position  = mul(input.Position, RotationMatrix(gRotationSpeed * gTime));
    output.Position = mul(float4(input.Position, 1.0f), gWorldViewProj);
    
    input.Normal    = mul(input.Normal, RotationMatrix(gRotationSpeed * gTime));
    output.Normal   = input.Normal;
    
    input.Tangent   = mul(input.Tangent, RotationMatrix(gRotationSpeed * gTime));
    output.Tangent  = input.Tangent;
    
    output.Color    = input.Color;
    output.Uv       = input.Uv;
    output.ViewDirection = normalize(gCameraPos - input.Position.xyz);

    return output;
}

VS_OUTPUT VS_FireFX(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    
    input.Position  = mul(input.Position, RotationMatrix(gRotationSpeed * gTime));
    output.Position = mul(float4(input.Position, 1.0f), gWorldViewProj);
    
    output.Uv       = input.Uv;

    return output;
}

//-------------------------------------------------
// Pixel Shader
//-------------------------------------------------
float4 PixelShading(VS_OUTPUT input, SamplerState sampleState)
{
    float3 color = (float3)0;

    float3 diffuseColor = gDiffuseMap.Sample(sampleState, input.Uv).rgb;
    float3 normalColor = gNormalMap.Sample(sampleState, input.Uv).rgb;
    float3 specularColor = gSpecularMap.Sample(sampleState, input.Uv).rgb;
    float  gloss = gGlossMap.Sample(sampleState, input.Uv).r;

    float3 normal = input.Normal;
    float3 tangent = input.Tangent;
    float3 viewDir = input.ViewDirection;

    //normal
    float3 binormal = cross(normal, tangent);
    float3x3 tangentSpace = float3x3(tangent, binormal, normal);
    normalColor = 2 * normalColor - 1.0f;
    normal = gUseNormalMap ? mul(normalColor, tangentSpace) : normal;

    float observedArea = dot(normal, -gLightDirection);
    if (observedArea < 0)
    {
        return float4(color, 1.0f);
    }

    // Phong
    float3 reflectedLight = reflect(-gLightDirection, normal);
    float cosAlpha = saturate(dot(reflectedLight, -viewDir));
    float3 lambert = diffuseColor * gKD / gPI;
    float3 phong =  specularColor * pow(cosAlpha, gloss * gShininess);
   
    color = (lambert + phong + gAmbient) * observedArea;
    return float4(color, 1.0f);
}

float4 PS_Point(VS_OUTPUT input) : SV_TARGET
{    
    return PixelShading(input, samPoint);
}

float4 PS_Linear(VS_OUTPUT input) : SV_TARGET
{    
    return PixelShading(input, samLinear);
}

float4 PS_Anisotropic(VS_OUTPUT input) : SV_TARGET
{    
    return PixelShading(input, samAnisotropic);
}

float4 PS_FireFX(VS_OUTPUT input) : SV_TARGET
{
    float4 color = gDiffuseMap.Sample(samPoint, input.Uv);
    return color;
}

//-------------------------------------------------
// Techniques
//-------------------------------------------------
technique11 DefaultTechnique
{
    pass P0 // Point sampling
    {
        SetDepthStencilState( gNoDepthStencilState, 0 );
        
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS_Point() ) );
    }
    
    pass P1 // Linear sampling
    {
        SetDepthStencilState( gNoDepthStencilState, 0 );

        SetVertexShader( CompileShader( vs_5_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS_Linear() ) );
    }
    
    pass P2 // Anisotropic sampling
    {
        SetDepthStencilState( gNoDepthStencilState, 0 );

        SetVertexShader( CompileShader( vs_5_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS_Anisotropic() ) );
    }
    
    pass P3 // FireFX
    {
        SetRasterizerState( gRasterizerState );
        SetDepthStencilState( gDepthStencilState, 0 );
        SetBlendState( gBlendState, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF ); 
        
        SetVertexShader( CompileShader( vs_5_0, VS_FireFX() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS_FireFX() ) );
    }
}