#version 450
out vec4 FragColor;
in vec2 UV;

uniform float iTime;

uniform sampler2D _BrickTexture;
uniform sampler2D _SmokeTexture;

void main()
{
	vec2 ut = vec2(UV.x + iTime, UV.y);

    vec4 brick = texture(_BrickTexture, ut);
    vec4 smoke = texture(_SmokeTexture, UV);
	
	vec3 ResultColor = smoke.rgb * smoke.a + brick.rgb * (1.0 - smoke.a);

	FragColor = vec4(ResultColor,1.0);
}
