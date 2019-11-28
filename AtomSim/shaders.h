#pragma once

const char *renderShader = R"glsl(

#ifdef VERTEX

layout(location = 0) out vec3 out_Pos;
layout(location = 1) out vec3 out_Normal;
layout(location = 2) out float out_Velocity;

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;

layout(location = 2) in vec4 ParticlePosition;
layout(location = 3) in vec4 ParticleVelocity;

layout(location = 0) uniform mat4 VPMatrix;
layout(location = 3) uniform float Radius;

void main()
{
	out_Pos = ParticlePosition.xyz + Position * Radius;
	out_Normal = Normal;
	out_Velocity = length(ParticleVelocity.xyz);
	gl_Position = VPMatrix * vec4(out_Pos, 1);
}

#endif

#ifdef FRAGMENT

layout(location = 0) out vec4 fragment;

layout(location = 0) in vec3 in_Pos;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in float in_Velocity;

layout(location = 1) uniform vec3 Color;
layout(location = 2) uniform vec3 CamPos;

void main()
{
	vec3 lightDir = normalize(vec3(-1, -1, 0.5));
	vec3 ambient = vec3(0.2);

	vec3 viewDir = normalize(CamPos - in_Pos);
	vec3 reflectDir = reflect(lightDir, in_Normal);  

	float diffuse = max(dot(in_Normal, -lightDir), 0.0);
	float specular = pow(max(dot(viewDir, reflectDir), 0.0), 16) * 0;

	fragment = vec4(diffuse * mix(Color, vec3(1.0), in_Velocity / 300.0) + specular * vec3(1.0) + ambient * Color, 1);
}

#endif

)glsl";

const char* simulationShader = R"glsl(

#ifdef COMPUTE
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

struct Particle
{
	vec4 Position;
	vec4 Velocity;
	vec4 Acceleration;
	vec4 Padding;
};

layout(std430, binding = 0) buffer ParticleBuffer
{
	Particle Particles[];
};

layout(location = 0) uniform float deltaTime;

void main()
{
	Particle p = Particles[gl_GlobalInvocationID.x];

	p.Velocity.xyz += p.Acceleration.xyz * deltaTime;
	p.Position.xyz += p.Velocity.xyz * deltaTime;
	p.Acceleration.xyz = vec3(0.0);

	Particles[gl_GlobalInvocationID.x] = p;
}
#endif

)glsl";

const char* interactionShader = R"glsl(

#ifdef COMPUTE
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

struct Particle
{
	vec4 PositionCharge;
	vec4 VelocityMass;
	vec4 Acceleration;
	vec4 Padding;
};

layout(std430, binding = 0) buffer Particle1Buffer
{
	Particle Particles1[];
};

layout(std430, binding = 1) buffer Particle2Buffer
{
	Particle Particles2[];
};

layout(location = 0) uniform float electrostaticConst;
layout(location = 1) uniform float strongConst;
layout(location = 2) uniform float strongDistConst;

//Proton-Proton, Proton-Neutron, Proton-Electron, Electron-Electron, Neutron-Neutron

float emf(float q1, float q2, float oneOverR)
{
	return oneOverR * oneOverR * q1 * q2 * electrostaticConst;
}

float sf(float r)
{
	return strongConst * pow(r, 1.0/3.0) / exp(strongDistConst * r);
	//return strongConst * (r - 3) / (r * (pow(r, strongDistConst) + 4));
}

void main()
{
#if defined(PROTON_PROTON) || defined(ELECTRON_ELECTRON) || defined(NEUTRON_NEUTRON)
	Particle p1 = Particles1[gl_GlobalInvocationID.x];
	Particle p2 = Particles1[gl_GlobalInvocationID.y];
#else
	Particle p1 = Particles1[gl_GlobalInvocationID.x];
	Particle p2 = Particles2[gl_GlobalInvocationID.y];
#endif
	vec3 vecTo = p2.PositionCharge.xyz - p1.PositionCharge.xyz;
	float norm = length(vecTo) * 0.00001;
	float oneOverNorm = 1.0 / norm;
	vecTo = normalize(vecTo);

#ifdef PROTON_PROTON
	vec3 emforce = -vecTo * emf(p1.PositionCharge.w, p2.PositionCharge.w, oneOverNorm);
	vec3 sforce = vecTo * sf(norm);
	p1.Acceleration.xyz += mix((emforce + sforce) / p1.VelocityMass.w, vec3(0.0), isnan(emforce) || isnan(sforce)); 
#endif

#ifdef PROTON_NEUTRON
	vec3 sforce = vecTo * sf(norm);
	p1.Acceleration.xyz += mix(sforce / p1.VelocityMass.w, vec3(0.0), isnan(sforce)); 
	p2.Acceleration.xyz -= mix(sforce / p2.VelocityMass.w, vec3(0.0), isnan(sforce)); 
#endif

#ifdef PROTON_ELECTRON
	vec3 emforce = -vecTo * emf(p1.PositionCharge.w, p2.PositionCharge.w, oneOverNorm);
	p1.Acceleration.xyz += mix(emforce / p1.VelocityMass.w, vec3(0.0), isnan(emforce));
	p2.Acceleration.xyz -= mix(emforce / p2.VelocityMass.w, vec3(0.0), isnan(emforce));
#endif

#ifdef NEUTRON_NEUTRON
	vec3 sforce = vecTo * sf(norm);
	p1.Acceleration.xyz += mix(sforce / p1.VelocityMass.w, vec3(0.0), isnan(sforce)); 
#endif

#ifdef ELECTRON_ELECTRON
	vec3 emforce = -vecTo * emf(p1.PositionCharge.w, p2.PositionCharge.w, oneOverNorm);
	p1.Acceleration.xyz += mix(emforce / p1.VelocityMass.w, vec3(0.0), isnan(emforce)) * 0;
#endif

	Particles1[gl_GlobalInvocationID.x] = p1;
#if defined(PROTON_PROTON) || defined(ELECTRON_ELECTRON) || defined(NEUTRON_NEUTRON)
	Particles1[gl_GlobalInvocationID.y] = p2;
#else
	Particles2[gl_GlobalInvocationID.y] = p2;
#endif
}
#endif

)glsl";