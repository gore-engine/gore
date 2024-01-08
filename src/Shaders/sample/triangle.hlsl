struct Attributes
{
    float3 positionOS : POSITION;
    half4 color : COLOR;
};

struct Varyings
{
    float4 pos : SV_Position;
    float3 color : COLOR;
};

Varyings vs(Attributes IN)
{
    Varyings OUT = (Varyings)0;
    OUT.pos = float4(IN.positionOS, 1.0f);
    OUT.color = IN.color.rgb;
    return OUT;
}

float4 ps(Varyings IN) : SV_Target0
{
    return float4(IN.color, 1.0f);
}