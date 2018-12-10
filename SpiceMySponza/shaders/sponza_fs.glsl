#version 330

struct Light
{
	vec3 intensity;
	vec3 position;
	float range;
};

uniform Light lights[22];
uniform int numLights;
uniform vec3 ambient_intensity;
uniform vec3 ambient_colour;
uniform vec3 diffuse_colour;
uniform vec3 camPos;
uniform vec3 spec_colour;
uniform float shininess;

in vec3 vpos;
in vec3 vnormal;

out vec4 fragment_colour;

void main(void)
{	
	vec3 diffuse_sum = vec3(0);
	vec3 spec_sum = vec3(0);

	// surface position and normal from vertex attributes
	vec3 P = vpos;
	vec3 N = normalize(vnormal);

	for (int i = 0; i < 12; i++)
	{
		// light vector (direction towards incoming ray of light)
		vec3 L = normalize(lights[i].position - P);

		vec3 distance = lights[i].position - P;
		float d = length(distance);
		float attenuation = smoothstep(lights[i].range, lights[i].range / 2, d);

		vec3 viewDir = normalize(camPos - P);
		vec3 reflectDir = reflect(-L, N);

		if (shininess > 0)
		{
			float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

			vec3 specular = spec * spec_colour;
			spec_sum += attenuation * specular * lights[i].intensity;
		}

		//reflected diffuse light intensity
		float diffuse_intensity = attenuation * max(0, dot(L, N));

		diffuse_sum += diffuse_intensity * lights[i].intensity;
	}

	vec3 reflected_light;
	// pass to fragment shader as a varying
	reflected_light = ambient_intensity * ambient_colour + (diffuse_colour * diffuse_sum) + spec_sum * spec_colour;

	fragment_colour = vec4(reflected_light, 1.0);
}
