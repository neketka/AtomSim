#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "Buffer.h"
#include "VertexArray.h"
#include "Shader.h"

class Particle
{
public:
	glm::vec4 Position; //+Charge
	glm::vec4 Velocity; //+Mass
	glm::vec4 Acceleration;
	glm::vec4 Padding;
};

class Vertex
{
public:
	glm::vec3 Position;
	glm::vec3 Normal;
};

class World
{
public:
	void Initialize();

	void Simulate(float deltaTime);
	void Render();
	void Dispose();

	void ClearAndGenerate(int protons, int electrons, int neutrons);

	void SetConstants(float eStatic, float sForce, float sForceDist) 
	{ 
		m_eStaticConst = eStatic; 
		m_strongConst = sForce;
		m_strongDistConst = sForceDist;
	}

	void SetMasses(float electron, float proton, float neutron)
	{
		m_protonMass = proton;
		m_electronMass = electron;
		m_neutronMass = neutron;
	}

	void SetElectronVelocity(float min, float max)
	{
		m_minElectronVel = min;
		m_maxNucleonVel = max;
	}

	void SetNucleonVelocity(float min, float max)
	{
		m_minNucleonVel = min;
		m_maxNucleonVel = max;
	}

	void SetGenerationDistance(float distance) 
	{
		this->m_generationDistance = distance;
	}

	void SetRenderingParameters(glm::vec3 protonColor, glm::vec3 electronColor, glm::vec3 neutronColor, float protonRadius, float electronRadius, float neutronRadius)
	{
		m_protonColor = protonColor;
		m_electronColor = electronColor;
		m_neutronColor = neutronColor;

		m_electronRadius = electronRadius;
		m_protonRadius = protonRadius;
		m_neutronRadius = neutronRadius;
	}

	void SetupCameraProjection(float fovY, float width, float height)
	{
		m_width = width;
		m_height = height;
		m_projection = glm::perspectiveFov(fovY * 3.141592f / 180.f, width, height, 0.3f, 10000.f);
	}

	void SetupCameraLook(glm::vec3 pos, glm::vec3 lookat)
	{
		m_camPos = pos;
		m_camLookat = lookat;
		m_view = glm::lookAt(pos, lookat, glm::vec3(0, 1, 0));
	}

	void RotateAround(float dX, float dY)
	{
		m_camPos = glm::vec3(glm::translate<float>(m_camLookat) * glm::rotate<float>(dY, glm::vec3(0, 1, 0)) * glm::rotate<float>(dX, glm::vec3(1, 0, 0)) * glm::translate<float>(-m_camLookat) * glm::vec4(m_camPos, 1));
		SetupCameraLook(m_camPos, m_camLookat);
	}

	void ScaleDown(float scale)
	{
		glm::vec3 vec = m_camPos - m_camLookat;
		float length = glm::length(vec);
		float newLen = length - scale;
		vec = vec / length * ((newLen == 0.f) ? scale * 10 : newLen);
		m_camPos = vec + m_camLookat;
		SetupCameraLook(m_camPos, m_camLookat);
	}

	void Translate(float x, float y)
	{
		glm::vec3 forward = glm::normalize(m_camLookat - m_camPos);
		glm::vec3 right = glm::cross(forward, glm::vec3(0, 1, 0));
		glm::vec3 down = glm::cross(forward, right);
		glm::vec3 delta = down * y + right * x;

		m_camPos += delta;
		m_camLookat += delta;
		SetupCameraLook(m_camPos, m_camLookat);
	}

private:
	void GenerateRenderBuffers();
	void GenerateShaders();

	float m_eStaticConst, m_strongConst, m_strongDistConst;
	float m_protonMass, m_electronMass, m_neutronMass;
	float m_minElectronVel, m_maxElectronVel;
	float m_minNucleonVel, m_maxNucleonVel;

	float m_generationDistance;

	float m_protonRadius, m_electronRadius, m_neutronRadius;
	glm::vec3 m_protonColor, m_electronColor, m_neutronColor;

	VertexArray m_protonVao;
	VertexArray m_neutronVao;
	VertexArray m_electronVao;

	Buffer<Vertex> m_sphereVertices;

	Buffer<Particle> m_protonBuffer;
	Buffer<Particle> m_electronBuffer;
	Buffer<Particle> m_neutronBuffer;

	float m_width, m_height;
	glm::vec3 m_camPos;
	glm::vec3 m_camLookat;
	glm::mat4 m_projection;
	glm::mat4 m_view;

	Shader m_simulationShader;

	Shader m_protonProtonShader;
	Shader m_protonNeutronShader;
	Shader m_protonElectronShader;
	Shader m_neutronNeutronShader;
	Shader m_electronElectronShader;

	Shader m_renderingShader;

	int m_protonCount, m_neutronCount, m_electronCount;
};

