#version 450
layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;

uniform mat4 Model;

out vec3 Normal;

void main(){
	Normal = vNormal;
	gl_Position = Model * vec4(vPos,1.0);
	gl_Position.z*=-1.0;
}