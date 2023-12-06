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
uniform sampler2D _TextureNight;

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
    vec4 texColorN = texture(_TextureNight, fs_in.UV);

    vec3 mainColor = texColor.rgb * (ambient + diffuse);
    mainColor += texColorN.rgb * (0.9f - (ambient + diffuse));
    mainColor += texColor.rgb * specular * (1.0f - texColor.b * specular);

    float fresnel = 3.0f * pow(1.0f - dot(normal, viewDir), 4.0f);
    vec3 baseColor = vec3(0.0, 0.1f, 0.5f);
    vec3 envColor = vec3(0.75f);

    vec3 color = mix(baseColor, envColor, fresnel);

    vec3 finalColor = mainColor + color * (ambient + diffuse);

    FragColor = vec4(finalColor, texColor.a);

}
