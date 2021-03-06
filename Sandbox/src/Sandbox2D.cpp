#include "Sandbox2D.h"
#include "imgui.h"
#include <glm/gtc/type_ptr.hpp>
#include "Platform/OpenGL/OpenGLShader.h"

#include <chrono>

float highfps = 0;

Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f)
{
}

void Sandbox2D::OnAttach()
{
	PHX_PROFILE_FUNCTION();

	ImGuiIO& io = ImGui::GetIO();
	io.FontDefault = io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Light.ttf", 14.0f);

	m_Texture = phx::Texture2D::Create("assets/textures/placeholder.png");
}

void Sandbox2D::OnDetach()
{
	PHX_PROFILE_FUNCTION();
}

void Sandbox2D::OnUpdate(phx::DeltaTime dt)
{
	PHX_PROFILE_FUNCTION();

	phx::Renderer2D::ResetStats();

	m_CameraController.OnUpdate(dt);

	{
		PHX_PROFILE_SCOPE("Renderer Prep");
		phx::RenderCommand::ClearColor({ 0.1, 0.1, 0.1, 1 });
		phx::RenderCommand::Clear();
	}

	{
		PHX_PROFILE_SCOPE("Renderer Draw");

		deltatimems = dt.GetMilliseconds();

		static float rotation = 0.0f;
		rotation += 30 * dt;

		phx::Renderer2D::BeginScene(m_CameraController.GetCamera());
		phx::Renderer2D::DrawQuadFilled({ -0.23f, -0.23f, -0.01 }, { 10.5f, 10.5f }, m_Texture, tiling);
		//phx::Renderer2D::DrawRotatedQuadFilled({ 0.0f, 0.0f, -0.01f }, { 10.0f, 10.0f }, rotation, m_Texture, tiling, m_SquareColor);
		//phx::Renderer2D::DrawRotatedQuadFilled({ 0.0f, 0.0f, -0.005f }, { 5.0f, 5.0f }, rotation, m_SquareColor);

		phx::Renderer2D::EndScene();

		phx::Renderer2D::BeginScene(m_CameraController.GetCamera());
		for (float y = -5.0f; y < 5.0f; y += 0.5f)
		{
			for (float x = -5.0f; x < 5.0f; x += 0.5f)
			{
				glm::vec4 color = { (x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.7f };
				phx::Renderer2D::DrawQuadFilled({ x, y }, { 0.45f, 0.45f }, color);
			}
		}
		phx::Renderer2D::EndScene();
	}
	
}

void Sandbox2D::OnImGuiRender()
{
	PHX_PROFILE_FUNCTION();

	ImGui::Begin("Metrics");
	float fps = 1000.0f / deltatimems;

	if (fps > highfps)
		highfps = fps;

	auto stats = phx::Renderer2D::GetStats();
	ImGui::Text("FPS: %.0f", fps);
	ImGui::SameLine();
	ImGui::Text("High: %.0f", highfps);
	ImGui::SameLine();
	if (ImGui::Button("Reset"))
		highfps = 0;
	ImGui::Separator();
	ImGui::Text("Renderer2D Stats");
	ImGui::Text("Draw Calls: %d", stats.DrawCalls);
	ImGui::Text("Quads: %d", stats.QuadCount);
	ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
	ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
	ImGui::End();
}

void Sandbox2D::OnEvent(phx::Event& e)
{
	m_CameraController.OnEvent(e);
}
