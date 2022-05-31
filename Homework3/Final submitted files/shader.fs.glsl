#version 330

in vec2 texCoord;
in vec3 vertex_view;
in vec3 vertex_normal;
in vec4 vertex_color;

out vec4 fragColor;

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

uniform mat4 view_matrix;
uniform Material material;
uniform int light_type;
uniform Light light[3];
uniform int vertex_perpixel;

// [TODO] passing texture from main.cpp
// Hint: sampler2D
uniform sampler2D diffuseTexture;

vec4 calculateLight(vec3 N, vec3 V, int ligh_type){
	vec4 lightInView = view_matrix * vec4(light[ligh_type].position, 1.0f);
	vec3 S = normalize(lightInView.xyz + V);
	vec3 H = normalize(S + V);
	float dc = dot(S, N);
	float sc = pow(max(dot(H, N), 0), material.shininess);
	vec3 color;

	if(light_type == 0){
		color = light[0].Ambient * material.Ka + dc * light[0].Diffuse * material.Kd + sc * light[0].Specular * material.Ks;
	}
	if(light_type == 1){
		float dist = length(lightInView.xyz + V);
		float attenuation = 1.0 / (light[1].constantAttenuation + light[1].linearAttenuation * dist + pow(dist, 2) * light[1].quadraticAttenuation);
		color = attenuation * (light[1].Ambient * material.Ka + dc * light[1].Diffuse * material.Kd + sc * light[1].Specular * material.Ks);
	}
	if(light_type == 2){
		float spot = dot(-S, normalize(light[2].spotDirection.xyz));
		float dist = length(lightInView.xyz + V);
		float attenuation = 1.0 / (light[2].constantAttenuation + light[2].linearAttenuation * dist + pow(dist, 2) * light[2].quadraticAttenuation);

		color;
		if(spot < light[2].spotCutoff)
			color = light[2].Ambient * material.Ka;
		else
			color = light[2].Ambient * material.Ka + attenuation * pow(max(spot, 0), light[2].spotExponent) * (dc * light[2].Diffuse * material.Kd + sc * light[2].Specular * material.Ks);
	}

	vec4 output_color = vec4(color, 1.0f);
	return output_color;
}

void main() {
	// fragColor = vec4(texCoord.xy, 0, 1);

	// [TODO] sampleing from texture
	// Hint: texture

	vec3 N = normalize(vertex_normal);
	vec3 V = - vertex_view;
	vec4 color = vec4(0, 0, 0, 0);
	color += calculateLight(N, V, light_type);

	if(vertex_perpixel == 0)
		fragColor = texture(diffuseTexture, texCoord) * vertex_color;
	else
		fragColor = texture(diffuseTexture, texCoord) * color;

}
