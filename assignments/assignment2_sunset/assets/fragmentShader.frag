#version 450

out vec4 FragColor;
in vec2 UV;

uniform float iTime;
uniform vec2 iResolution;

uniform vec3 _skyTop;
uniform vec3 _skyBottom;
uniform vec3 _sunBottom;
uniform vec3 _sunTop;
uniform vec3 _hillColor;
uniform float _sunRadius;
uniform float _speed;

float circleSDF(vec2 p, float r) {
    return length(p) - r;
}

void main(){
    float edgeCenter = sin(iTime * _speed) * 0.5 + 0.5;
    float t = smoothstep(edgeCenter-0.5, edgeCenter+0.5, UV.y);
    vec3 finalColor = mix(_skyBottom, _skyTop, t);

    vec2 circlePos = vec2(0.5, sin(iTime * _speed) * 0.5 + 0.5);
    circlePos.y -= 0.5;
    float d = circleSDF(UV - circlePos, _sunRadius);

    float edgeSmoothness = 0.08;
    float circleFactor = smoothstep(edgeSmoothness, -edgeSmoothness, d);
    
    float circleColorFactor = circlePos.y * 0.5 + 0.5;
    vec3 circleColor = mix(_sunBottom, _sunTop, circleColorFactor);

    finalColor = mix(finalColor, circleColor, circleFactor); 

    // Hills
    float hill1 = sin(UV.x * 4.0) * 0.1 + 0.2;
    float hill2 = sin(UV.x * 2.5 + 2.0) * 0.15 + 0.25;
    float hill3 = sin(UV.x * 3.0 - 1.5) * 0.08 + 0.3;
    float combinedHill = min(hill1, hill2);
    combinedHill = min(combinedHill, hill3);

    if(UV.y < combinedHill) {
        finalColor = _hillColor;
    }

    FragColor = vec4(finalColor, 1.0);
}
