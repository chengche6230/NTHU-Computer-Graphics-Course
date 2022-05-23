#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;

struct Material{
	vec3 Ka;
	vec3 Kd;
	vec3 Ks;
	float shininess;
};

struct Light{
	vec3 position;
	vec3 direction;
	vec3 spotDirection;
	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;
	float spotExponent;
	float spotCutoff;
	float constantAttenuation;
	float linearAttenuation;
	float quadraticAttenuation;
};

out vec3 vertex_view;
out vec3 vertex_normal;
out vec4 vertex_color;

uniform mat4 view_matrix;
uniform mat4 model_matrix;
uniform int light_type;
uniform mat4 mvp;
uniform Material material;
uniform Light light[3];

vec4 directionalLight(vec3 N, vec3 V){
	vec4 lightInView = view_matrix * vec4(light[0].position, 1.0f);
	vec3 S = normalize(lightInView.xyz + V);
	vec3 H = normalize(S + V);

	float dc = dot(S, N);
	float sc = pow(max(dot(H, N), 0), material.shininess);
	vec3 color = light[0].Ambient * material.Ka + dc * light[0].Diffuse * material.Kd + sc * light[0].Specular * material.Ks;
	vec4 output_color = vec4(color, 1.0f);

	return output_color;
}

vec4 pointLight(vec3 N, vec3 V){
	vec4 lightInView = view_matrix * vec4(light[1].position, 1.0f);
	vec3 S = normalize(lightInView.xyz + V);
	vec3 H = normalize(S + V);

	float dc = dot(S, N);
	float sc = pow(max(dot(H, N), 0), material.shininess);

	float dist = length(lightInView.xyz + V);
	float attenuation = 1.0 / (light[1].constantAttenuation + light[1].linearAttenuation * dist + pow(dist, 2) * light[1].quadraticAttenuation);
	vec3 color = attenuation * (light[1].Ambient * material.Ka + dc * light[1].Diffuse * material.Kd + sc * light[1].Specular * material.Ks);
	vec4 output_color = vec4(color, 1.0f);

	return output_color;
}

vec4 spotLight(vec3 N, vec3 V){
	vec4 lightInView = view_matrix * vec4(light[2].position, 1.0f);
	vec3 S = normalize(lightInView.xyz + V);
	vec3 H = normalize(S + V);

	float dc = dot(S, N);
	float sc = pow(max(dot(H, N), 0), material.shininess);

	float spot = dot(-S, normalize(light[2].spotDirection.xyz));
	float dist = length(lightInView.xyz + V);
	float attenuation = 1.0 / (light[2].constantAttenuation + light[2].linearAttenuation * dist + pow(dist, 2) * light[2].quadraticAttenuation);

	vec3 color;
	if(spot < light[2].spotCutoff)
		color = light[2].Ambient * material.Ka;
	else
		color = light[2].Ambient * material.Ka + attenuation * pow(max(spot, 0), light[2].spotExponent) * (dc * light[2].Diffuse * material.Kd + sc * light[2].Specular * material.Ks);
	vec4 output_color = vec4(color, 1.0f);

	return output_color;
}

void main()
{
	// V [TODO]
	// gl_Position = mvp * vec4(aPos.x, aPos.y, aPos.z, 1.0);
	// vertex_color = aColor;
	// vertex_normal = aNormal;

	vertex_color = vec4(0, 0, 0, 0);

	vec4 vertex = view_matrix * model_matrix * vec4(aPos.x, aPos.y, aPos.z, 1.0);
	vec4 normal = transpose(inverse(view_matrix * model_matrix)) * vec4(aNormal, 0.0);

	vertex_view = vertex.xyz;
	vertex_normal = normal.xyz;

	vec3 N = normalize(vertex_normal);
	vec3 V = -vertex_view;

	if(light_type == 0)
		vertex_color += directionalLight(N, V);
	else if(light_type == 1)
		vertex_color += pointLight(N, V);
	else if(light_type == 2)
		vertex_color += spotLight(N, V);

	gl_Position = mvp * vec4(aPos.x, aPos.y, aPos.z, 1.0);
}