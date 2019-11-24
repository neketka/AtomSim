#include "World.h"
#include "Framebuffer.h"
#include "CommandBuffer.h"
#include "shaders.h"
#include <glm/gtx/polar_coordinates.hpp>

void World::Initialize()
{
	GenerateRenderBuffers();
	GenerateShaders();
}

void World::Simulate(float deltaTime)
{
	ShaderConstants dt;
	dt.AddConstant(0, deltaTime);

	ShaderConstants cons;
	cons.AddConstant(0, m_eStaticConst);
	cons.AddConstant(1, m_strongConst);
	cons.AddConstant(2, m_strongDistConst);

	ShaderDescriptor pnDesc;
	pnDesc.AddShaderStorageBuffer(m_protonBuffer, 0);
	pnDesc.AddShaderStorageBuffer(m_neutronBuffer, 1);

	ShaderDescriptor peDesc;
	peDesc.AddShaderStorageBuffer(m_protonBuffer, 0);
	peDesc.AddShaderStorageBuffer(m_electronBuffer, 1);

	ShaderDescriptor ppDesc;
	ppDesc.AddShaderStorageBuffer(m_protonBuffer, 0);

	ShaderDescriptor nnDesc;
	nnDesc.AddShaderStorageBuffer(m_neutronBuffer, 0);

	ShaderDescriptor eeDesc;
	eeDesc.AddShaderStorageBuffer(m_electronBuffer, 0);

	CommandBuffer cmd;
	
	cmd.BindComputeShaderCommand(m_protonNeutronShader);
	cmd.BindConstantsCommand(cons);
	cmd.BindDescriptorCommand(pnDesc);
	cmd.DispatchCommand(m_protonCount, m_neutronCount, 1);

	cmd.BindComputeShaderCommand(m_protonElectronShader);
	cmd.BindConstantsCommand(cons);
	cmd.BindDescriptorCommand(peDesc);
	cmd.DispatchCommand(m_protonCount, m_electronCount, 1);

	cmd.BindComputeShaderCommand(m_protonProtonShader);
	cmd.BindConstantsCommand(cons);
	cmd.BindDescriptorCommand(ppDesc);
	cmd.DispatchCommand(m_protonCount, m_protonCount, 1);

	cmd.BindComputeShaderCommand(m_neutronNeutronShader);
	cmd.BindConstantsCommand(cons);
	cmd.BindDescriptorCommand(nnDesc);
	cmd.DispatchCommand(m_neutronCount, m_neutronCount, 1);

	cmd.BindComputeShaderCommand(m_electronElectronShader);
	cmd.BindConstantsCommand(cons);
	cmd.BindDescriptorCommand(eeDesc);
	cmd.DispatchCommand(m_electronCount, m_electronCount, 1);

	cmd.MemoryBarrier(MemoryBarrierType::ShaderStorageBuffer);

	cmd.BindComputeShaderCommand(m_simulationShader);
	cmd.BindConstantsCommand(dt);

	cmd.BindDescriptorCommand(ppDesc);
	cmd.DispatchCommand(m_protonCount, 1, 1);

	cmd.BindDescriptorCommand(nnDesc);
	cmd.DispatchCommand(m_neutronCount, 1, 1);

	cmd.BindDescriptorCommand(eeDesc);
	cmd.DispatchCommand(m_electronCount, 1, 1);

	cmd.ExecuteCommands();
}

void World::Render()
{
	Pipeline pipe;
	ShaderConstants consts;

	CommandBuffer cmd;

	pipe.FragmentStage.DepthTest = true;
	pipe.FragmentStage.Viewport = IQuad2D(0, 0, m_width, m_height);
	pipe.FragmentStage.Framebuffer = Framebuffer::GetDefault();
	pipe.Shader = m_renderingShader;

	consts.AddConstant(0, m_projection * m_view);
	consts.AddConstant(2, m_camPos);

	cmd.ClearAllBuffers();
	cmd.MemoryBarrier(MemoryBarrierType::VertexAttribArray);

	pipe.VertexStage.VertexArray = m_protonVao;
	consts.AddConstant(1, m_protonColor);
	consts.AddConstant(3, m_protonRadius);

	cmd.BindPipelineCommand(pipe);
	cmd.BindConstantsCommand(consts);

	cmd.DrawCommand(PrimitiveType::Triangles, 0, m_sphereVertices.GetSize(), m_protonCount, 0);

	pipe.VertexStage.VertexArray = m_electronVao;
	consts.AddConstant(1, m_electronColor);
	consts.AddConstant(3, m_electronRadius);

	cmd.BindPipelineCommand(pipe);
	cmd.BindConstantsCommand(consts);

	cmd.DrawCommand(PrimitiveType::Triangles, 0, m_sphereVertices.GetSize(), m_electronCount, 0);

	pipe.VertexStage.VertexArray = m_neutronVao;
	consts.AddConstant(1, m_neutronColor);
	consts.AddConstant(3, m_neutronRadius);

	cmd.BindPipelineCommand(pipe);
	cmd.BindConstantsCommand(consts);

	cmd.DrawCommand(PrimitiveType::Triangles, 0, m_sphereVertices.GetSize(), m_neutronCount, 0);

	cmd.ExecuteCommands();
}

void World::Dispose()
{
	m_protonVao.Destroy();
	m_electronVao.Destroy();
	m_neutronVao.Destroy();

	m_sphereVertices.Destroy();

	m_protonBuffer.Destroy();
	m_electronBuffer.Destroy();
	m_neutronBuffer.Destroy();

	m_simulationShader.Destroy();

	m_protonProtonShader.Destroy();
	m_protonNeutronShader.Destroy();
	m_protonElectronShader.Destroy();
	m_electronElectronShader.Destroy();
	m_neutronNeutronShader.Destroy();

	m_renderingShader.Destroy();
}

void GenerateRandomParticle(Particle& p, float minRad, float maxRad, float minVel, float maxVel, float charge, float mass)
{
	float radiusLen = (maxRad - minRad) * (((float)std::rand() / (RAND_MAX)) + 1.f) + minRad;
	float radiusTheta = 2.f * 3.14159265f * (((float)std::rand() / (RAND_MAX)) + 1.f);
	float radiusPhi = 2.f * 3.14159265f * (((float)std::rand() / (RAND_MAX)) + 1.f);

	float velLen = (maxVel - minVel) * (((float)std::rand() / (RAND_MAX)) + 1.f) + minVel;
	float velTheta = 2.f * 3.14159265f * (((float)std::rand() / (RAND_MAX)) + 1.f);
	float velPhi = 2.f * 3.14159265f * (((float)std::rand() / (RAND_MAX)) + 1.f);

	p.Position = glm::vec4(glm::euclidean(glm::vec2(radiusTheta, radiusPhi)) * radiusLen, charge);
	p.Velocity = glm::vec4(glm::euclidean(glm::vec2(velTheta, velPhi)) * velLen, mass);
	p.Acceleration = glm::vec4(0);
}

void World::ClearAndGenerate(int protons, int electrons, int neutrons)
{
	m_protonCount = protons;
	m_electronCount = electrons;
	m_neutronCount = neutrons;

	m_protonBuffer.Destroy();
	m_electronBuffer.Destroy();
	m_neutronBuffer.Destroy();

	std::vector<Particle> protonVec;
	std::vector<Particle> neutronVec;
	std::vector<Particle> electronVec;

	for (int i = 0; i < protons; ++i)
	{
		Particle p;
		GenerateRandomParticle(p, 1, m_generationDistance, m_minNucleonVel, m_maxNucleonVel, 1, m_protonMass);
		protonVec.push_back(p);
	}

	for (int i = 0; i < electrons; ++i)
	{
		Particle p;
		GenerateRandomParticle(p, 1, m_generationDistance, m_minElectronVel, m_maxElectronVel, -1, m_electronMass);
		electronVec.push_back(p);
	}

	for (int i = 0; i < neutrons; ++i)
	{
		Particle p;
		GenerateRandomParticle(p, 1, m_generationDistance, m_minNucleonVel, m_maxNucleonVel, 0, m_neutronMass);
		neutronVec.push_back(p);
	}

	m_protonBuffer = Buffer<Particle>(protonVec, BufferOptions(false, false, false, false, false, false));
	m_neutronBuffer = Buffer<Particle>(neutronVec, BufferOptions(false, false, false, false, false, false));
	m_electronBuffer = Buffer<Particle>(electronVec, BufferOptions(false, false, false, false, false, false));

	m_protonVao.SetBuffer(1, m_protonBuffer);
	m_electronVao.SetBuffer(1, m_electronBuffer);
	m_neutronVao.SetBuffer(1, m_neutronBuffer);
}

void World::GenerateRenderBuffers()
{
	std::vector<Vertex> m_vertices;
	Vertex v1, v2, v3, v4;

	int parallels = 50, meridians = 50;
	for (int i = 0; i < parallels; ++i)
	{
		float parallel1 = 3.14159265f * i / static_cast<float>(parallels);
		float parallel2 = 3.14159265f * (i + 1) / static_cast<float>(parallels);
		for (int j = 0; j < meridians; ++j)
		{
			float meridian1 = 2.f * 3.14159265f * j / static_cast<float>(meridians);
			float meridian2 = 2.f * 3.14159265f * (j + 1) / static_cast<float>(meridians);

			v1.Position = glm::euclidean(glm::vec2(meridian1, parallel1));
			v1.Normal = glm::normalize(v1.Position);

			v2.Position = glm::euclidean(glm::vec2(meridian1, parallel2));
			v2.Normal = glm::normalize(v2.Position);

			v3.Position = glm::euclidean(glm::vec2(meridian2, parallel2));
			v3.Normal = glm::normalize(v3.Position);

			v4.Position = glm::euclidean(glm::vec2(meridian2, parallel1));
			v4.Normal = glm::normalize(v4.Position);

			if (i == 0)
			{
				m_vertices.push_back(v4);
				m_vertices.push_back(v3);
				m_vertices.push_back(v1);
			}
			else if ((i + 1) == parallels)
			{
				m_vertices.push_back(v2);
				m_vertices.push_back(v1);
				m_vertices.push_back(v3);
			}
			else
			{
				m_vertices.push_back(v4);
				m_vertices.push_back(v2);
				m_vertices.push_back(v1);

				m_vertices.push_back(v4);
				m_vertices.push_back(v3);
				m_vertices.push_back(v2);
			}
		}
	}

	m_sphereVertices = Buffer<Vertex>(m_vertices, BufferOptions(false, false, false, false, false, false));

	std::vector<BufferStructure> bufStruct = {
		BufferStructure({
			Attribute(0, 0, 3, 12, true),
			Attribute(1, 12, 3, 12, true)
		}),
		BufferStructure({
			Attribute(2, 0, 4, 16, true, 1),
			Attribute(3, 16, 4, 16, true, 1),
			Attribute(4, 32, 4, 16, true, 1),
			Attribute(5, 48, 4, 16, true, 1)
		})
	};

	m_protonVao = VertexArray(bufStruct);
	m_neutronVao = VertexArray(bufStruct);
	m_electronVao = VertexArray(bufStruct);

	m_protonVao.SetBuffer(0, m_sphereVertices);
	m_neutronVao.SetBuffer(0, m_sphereVertices);
	m_electronVao.SetBuffer(0, m_sphereVertices);
}

void World::GenerateShaders()
{
	m_renderingShader = Shader(renderShader, {});
	m_simulationShader = Shader(simulationShader, {});
	m_protonProtonShader = Shader(interactionShader, { "PROTON_PROTON" });
	m_protonElectronShader = Shader(interactionShader, { "PROTON_ELECTRON" });
	m_protonNeutronShader = Shader(interactionShader, { "PROTON_NEUTRON" });
	m_neutronNeutronShader = Shader(interactionShader, { "NEUTRON_NEUTRON" });
	m_electronElectronShader = Shader(interactionShader, { "ELECTRON_ELECTRON" });
}
