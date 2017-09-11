#version 120

struct PointLight{
	vec3 position;
	vec3 color;
    float intensity;

 	float constant;
    float linear;
    float quadratic;
};

float shininess = 12; 

uniform PointLight light;
uniform vec3 viewPos;

varying vec2 texCoordVarying;
varying vec3 positionVarying;
varying vec3 normalVarying;

void main(){
  	// diffuse 
    vec3 norm = normalize(normalVarying);
    vec3 lightDir = normalize(light.position - positionVarying);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.color * diff;  
    

    // specular
    vec3 viewDir = normalize(viewPos - positionVarying);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = light.color * spec;  


    // attenuation
    float distance    = length(light.position - positionVarying);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    

    diffuse   *= attenuation;
    specular *= attenuation;   
        
    vec3 result = (diffuse + specular) * light.intensity;
    gl_FragColor = vec4(result, 1.0);
}