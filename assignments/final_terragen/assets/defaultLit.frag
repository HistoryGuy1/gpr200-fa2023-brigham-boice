#version 450
out vec4 FragColor;

in Surface {
    vec2 UV; 
    vec3 WorldPosition; 
    vec3 WorldNormal;
} fs_in;

struct Light {
    vec3 position;
    vec3 color;
};
#define MAX_LIGHTS 4
uniform Light _Lights[MAX_LIGHTS];
uniform vec3 _ViewPosition;
uniform int numLights;
uniform int useBlinnPhong;

uniform float ambientK;
uniform float diffuseK;
uniform float specularK;
uniform float shininess;

uniform sampler2D _Texture;

void main() {
    vec3 normal = normalize(fs_in.WorldNormal);
    vec3 viewDir = normalize(_ViewPosition - fs_in.WorldPosition);

    vec3 ambient = vec3(0.0);
    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);

    for (int i = 0; i < numLights; i++) {
        vec3 lightDir = normalize(_Lights[i].position - fs_in.WorldPosition);
        diffuse += diffuseK * _Lights[i].color * max(dot(normal, lightDir), 0.0);

        if(useBlinnPhong == 0)
        {
            //Blinn-Phong
            vec3 halfDir = normalize(lightDir + viewDir);
            float spec = pow(max(dot(halfDir, normal), 0.0), shininess);
            specular += specularK * spec * _Lights[i].color;
        }
        else
        {
            //Phong
            vec3 reflectDir = reflect(-lightDir, normal);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
            specular += specularK * spec * _Lights[i].color;
        }

        ambient += ambientK * _Lights[i].color;
    }

    vec4 texColor = texture(_Texture, fs_in.UV);
    vec3 resultColor = texColor.rgb * (ambient + diffuse + specular);
    FragColor = vec4(resultColor, texColor.a);
}
