#version 450
out vec4 FragColor;
in vec2 UV;

uniform sampler2D _GuyTexture;
uniform sampler2D _VineTexture;

void main(){
	vec4 Vine = texture(_VineTexture, UV);
    vec4 Guy = texture(_GuyTexture, UV);

	float alpha = 1.0 - (1.0 - Vine.a) * (1.0 - Guy.a);
    vec3 color = (Vine.rgb * Vine.a + Guy.rgb * Guy.a * (1.0 - Vine.a)) / alpha;
    
    FragColor = vec4(color, alpha);
}