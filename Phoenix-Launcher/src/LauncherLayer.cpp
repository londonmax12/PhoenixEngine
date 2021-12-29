#include "LauncherLayer.h"

#include <GLFW/include/GLFW/glfw3.h>

#include <stdlib.h>
#include <Phoenix/UI/GuiWidgets.h>
#include <Phoenix/Utils/Environment.h>

#define MAX_PROJECT_NAME_LENGTH 255
#define MAX_PROJECT_FILEPATH_LENGTH 512

namespace phx
{
	static char* s_ProjectNameBuffer = new char[MAX_PROJECT_NAME_LENGTH];
	static char* s_ProjectFilePathBuffer = new char[MAX_PROJECT_FILEPATH_LENGTH];

	LauncherLayer::LauncherLayer()
		: Layer("LauncherLayer")
	{
	}

	void LauncherLayer::OnAttach()
	{
		PHX_PROFILE_FUNCTION();
		RenderCommand::Init();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Bold.ttf", 20.0f);
		io.FontDefault = io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Light.ttf", 16.0f);

		m_PhoenixLogoTexture = Texture2D::Create("resources/phoenix.png");

		if(Environment::HasEnvironmentalVariable("PHOENIX_DIR"))
			m_Properties.InstallPath = Environment::GetEnvironmentalVariable("PHOENIX_DIR");
	}

	void LauncherLayer::OnDetach()
	{
		PHX_PROFILE_FUNCTION();
	}

	void LauncherLayer::OnUpdate(DeltaTime dt)
	{
		PHX_PROFILE_FUNCTION();

		RenderCommand::Clear();
		RenderCommand::ClearColor({ 0.12,0.12,0.12,1 });
		auto& io = ImGui::GetIO();
		if (Input::IsMouseButtonPressed(MouseCode::Button0) && !io.WantCaptureMouse)
		{
			if (!m_Dragging)
			{
				m_WindowPrevX = Application::Get().GetWindow().GetWindowPos().first;
				m_WindowPrevY = Application::Get().GetWindow().GetWindowPos().second;
			}

			m_Dragging = true;

			int currentMouseX = Input::GetMousePosition().x;
			int currentMouseY = Input::GetMousePosition().y;
			int deltaX = m_MousePrevX - currentMouseX;
			int deltaY = m_MousePrevY - currentMouseY;

			int windowX = m_WindowPrevX -= deltaX;
			int windowY = m_WindowPrevY -= deltaY;
			
			Application::Get().GetWindow().SetWindowPos(windowX, windowY);
		}
		else
		{
			m_Dragging = false;
		}
		m_MousePrevX = Input::GetMousePosition().x;
		m_MousePrevY = Input::GetMousePosition().y;
	}

	void LauncherLayer::OnImGuiRender()
	{
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		ImGui::Begin("Launcher", 0, window_flags);

		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		ImGui::PopStyleVar(2);

		// Phoenix Install Folder Prompt
		{
			if (m_Properties.InstallPath.empty() && !ImGui::IsPopupOpen("Select Phoenix Install"))
			{
				ImGui::OpenPopup("Select Phoenix Install");
				m_Properties.InstallPath.reserve(MAX_PROJECT_FILEPATH_LENGTH);
			}

			ImVec2 center = ImGui::GetMainViewport()->GetCenter();
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			ImGui::SetNextWindowSize(ImVec2(700, 0));
			if (ImGui::BeginPopupModal("Select Phoenix Install", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
			{
				ImGui::PushFont(boldFont);
				ImGui::TextUnformatted("Failed to find Phoenix installation");
				ImGui::PopFont();

				ImGui::TextWrapped("Please select the folder for the Phoenix version you want to use.");

				//ImGui::Dummy(ImVec2(0, 8));

				ImVec2 label_size = ImGui::CalcTextSize("...", NULL, true);
				auto& style = ImGui::GetStyle();
				ImVec2 button_size = ImGui::CalcItemSize(ImVec2(0, 0), label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 10));
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 6));
				ImGui::SetNextItemWidth(700 - button_size.x - style.FramePadding.x * 2.0f - style.ItemInnerSpacing.x - 1);
				ImGui::InputTextWithHint("##phoenix_install_location", "C:/Users/user/Phoenix", m_Properties.InstallPath.data(), MAX_PROJECT_FILEPATH_LENGTH, ImGuiInputTextFlags_ReadOnly);
				ImGui::SameLine();
				if (ImGui::Button("..."))
				{
					std::string result = FileDialogs::BrowseFolder();
					m_Properties.InstallPath.assign(result);
				}

				if (ImGui::Button("Confirm"))
				{
					bool success = Environment::SetEnvironmentalVariable("PHOENIX_DIR", m_Properties.InstallPath);
					ImGui::CloseCurrentPopup();
				}

				ImGui::PopStyleVar(2);

				ImGui::EndPopup();
			}
		}

		ImGui::Columns(2);
		//ImGui::SetColumnWidth(0, viewport->WorkSize.x / 1.5f);

		static std::string s_ProjectToOpen = "";

		bool showNewProjectPopup = false;
		bool serializePreferences = false;

		// Info Area
		ImGui::BeginChild("info_area");
		float columnWidth = ImGui::GetColumnWidth();
		float columnCenterX = columnWidth / 2.0f;
		float imageSize = 140.0f;

		//ImGui::SetCursorPosY(-40.0f);
		UI::DrawImage(m_PhoenixLogoTexture, ImVec2(imageSize, imageSize));

		ImGui::Separator();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + imageSize / 3.0f);

		ImGui::BeginChild("RecentProjects");

		float projectButtonWidth = columnWidth - 60.0f;
		ImGui::SetCursorPosX(20.0f);
		ImGui::BeginGroup();

		bool anyFrameHovered = false;

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(20, 5));
		/*auto& recentProjects = m_Properties.UserPreferences->RecentProjects;
		for (auto it = recentProjects.begin(); it != recentProjects.end(); it++)
		{
			time_t lastOpened = it->first;
			auto& recentProject = it->second;

			// Custom button rendering to allow for multiple text elements inside a button
			std::string fullID = "Project_" + recentProject.FilePath;
			ImGuiID id = ImGui::GetID(fullID.c_str());

			bool changedColor = false;
			if (id == m_HoveredProjectID)
			{
				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetStyleColorVec4(ImGuiCol_FrameBgHovered));
				changedColor = true;
			}

			ImGui::BeginChildFrame(id, ImVec2(projectButtonWidth, 50));
			{
				float leftEdge = ImGui::GetCursorPosX();

				ImGui::PushFont(boldFont);
				ImGui::TextUnformatted(recentProject.Name.c_str());
				ImGui::PopFont();

				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.85f, 0.85f, 1.0f));
				ImGui::TextUnformatted(recentProject.FilePath.c_str());

				float prevX = ImGui::GetCursorPosX();
				ImGui::SameLine();
				std::string lastOpenedString = FormatDateAndTime(lastOpened);
				ImGui::SetCursorPosX(leftEdge + projectButtonWidth - ImGui::CalcTextSize(lastOpenedString.c_str()).x - ImGui::GetStyle().FramePadding.x * 1.5f);
				ImGui::TextUnformatted(lastOpenedString.c_str());
				ImGui::PopStyleColor();

				ImGui::SetCursorPosX(prevX);

				if (ImGui::IsWindowHovered())
				{
					anyFrameHovered = true;
					m_HoveredProjectID = id;

					if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					{
						s_ProjectToOpen = recentProject.FilePath;
						recentProject.LastOpened = time(NULL);
					}
				}

				if (ImGui::BeginPopupContextWindow("project_context_window"))
				{
					bool isStartupProject = m_Properties.UserPreferences->StartupProject == recentProject.FilePath;
					if (ImGui::MenuItem("Set Startup Project", nullptr, &isStartupProject))
					{
						m_Properties.UserPreferences->StartupProject = isStartupProject ? recentProject.FilePath : "";
						serializePreferences = true;
					}

					if (ImGui::MenuItem("Remove From List"))
					{
						if (isStartupProject)
							m_Properties.UserPreferences->StartupProject = "";
						it = recentProjects.erase(it);
					}

					ImGui::EndPopup();
				}
			}
			ImGui::EndChildFrame();

			if (changedColor)
				ImGui::PopStyleColor();
		}*/
		ImGui::PopStyleVar();

		if (!anyFrameHovered)
			m_HoveredProjectID = 0;

		ImGui::EndGroup();
		ImGui::EndChild();
	
		ImGui::EndChild();

		ImGui::NextColumn();

		ImGui::BeginChild("general_area");
		{
			float columnWidth = ImGui::GetColumnWidth();
			float buttonWidth = columnWidth / 1.5f;
			float columnCenterX = columnWidth / 2.0f;

			ImGui::SetCursorPosX(columnCenterX - buttonWidth / 2.0f);
			ImGui::BeginGroup();

			if (ImGui::Button("New Project", ImVec2(buttonWidth, 50)))
				showNewProjectPopup = true;

			if (ImGui::Button("Open Project...", ImVec2(buttonWidth, 50)))
			{
				std::string result = FileDialogs::OpenFile("Phoenix Project (*.phxproj)\0*.phxproj\0");
				std::replace(result.begin(), result.end(), '\\', '/');
				//AddProjectToRecents(result);
				s_ProjectToOpen = result;
			}

			ImGui::EndGroup();
		}
		ImGui::EndChild();

		if (showNewProjectPopup)
		{
			ImGui::OpenPopup("New Project");
			memset(s_ProjectNameBuffer, 0, MAX_PROJECT_NAME_LENGTH);
			memset(s_ProjectFilePathBuffer, 0, MAX_PROJECT_FILEPATH_LENGTH);
			showNewProjectPopup = false;
		}

		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(ImVec2{ 700, 325 });
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 10));
		if (ImGui::BeginPopupModal("New Project", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar))
		{
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 325 / 8);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 7));

			ImGui::PushFont(boldFont);
			std::string fullProjectPath = strlen(s_ProjectFilePathBuffer) > 0 ? std::string(s_ProjectFilePathBuffer) + "/" + std::string(s_ProjectNameBuffer) : "";
			ImGui::Text("Full Project Path: %s", fullProjectPath.c_str());
			ImGui::PopFont();

			ImGui::SetNextItemWidth(-1);
			ImGui::InputTextWithHint("##new_project_name", "Project Name", s_ProjectNameBuffer, MAX_PROJECT_NAME_LENGTH);

			ImVec2 label_size = ImGui::CalcTextSize("...", NULL, true);
			auto& style = ImGui::GetStyle();
			ImVec2 button_size = ImGui::CalcItemSize(ImVec2(0, 0), label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

			ImGui::SetNextItemWidth(600 - button_size.x - style.FramePadding.x * 2.0f - style.ItemInnerSpacing.x - 1);
			ImGui::InputTextWithHint("##new_project_location", "Project Location", s_ProjectFilePathBuffer, MAX_PROJECT_FILEPATH_LENGTH, ImGuiInputTextFlags_ReadOnly);
			ImGui::SameLine();
			if (ImGui::Button("..."))
			{
				std::string result = FileDialogs::BrowseFolder();
				std::replace(result.begin(), result.end(), '\\', '/');
				memcpy(s_ProjectFilePathBuffer, result.data(), result.length());
			}

			ImGui::Separator();

			ImGui::PushFont(boldFont);
			if (ImGui::Button("Create"))
			{
				CreateProject(s_ProjectNameBuffer, fullProjectPath);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
				ImGui::CloseCurrentPopup();
			ImGui::PopFont();

			ImGui::PopStyleVar();
			ImGui::EndPopup();
		}
		ImGui::PopStyleVar(2);

		ImGui::End();

		if (!s_ProjectToOpen.empty())
		{
			std::filesystem::path phxDir = Environment::GetEnvironmentalVariable("PHOENIX_DIR");
			std::string phoenixWorkingDirectory = (phxDir / "Phoenix-Editor").string();

			#ifdef PHX_DEBUG_MODE
						phxDir = phxDir / "bin" / "Debug-windows-x86_64" / "Phoenix-Editor";
			#else
						phxDir = phxDir / "bin" / "Release-windows-x86_64" / "Phoenix-Editor";
			#endif

			std::string phxExe = (phxDir / "Phoenix-Editor.exe").string();
			std::string commandLine = phxExe + " " + s_ProjectToOpen;

			PROCESS_INFORMATION processInfo;
			STARTUPINFOA startupInfo;
			ZeroMemory(&startupInfo, sizeof(STARTUPINFOA));
			startupInfo.cb = sizeof(startupInfo);

			bool result = CreateProcessA(phxExe.c_str(), commandLine.data(), NULL, NULL, FALSE, DETACHED_PROCESS, NULL, phoenixWorkingDirectory.c_str(), &startupInfo, &processInfo);
			if (result)
			{
				CloseHandle(processInfo.hThread);
				CloseHandle(processInfo.hProcess);
			}
			else
			{
				ImGui::OpenPopup("Enviroment Error");
			}
			s_ProjectToOpen = std::string();
		}
		if (ImGui::BeginPopupModal("Enviroment Error", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar))
		{
			ImGui::PushFont(boldFont);
			ImGui::TextUnformatted("Failed to open the Phoenix Launcher! Is the install directory correct?");
			ImGui::PopFont();

			ImVec2 label_size = ImGui::CalcTextSize("...", NULL, true);
			auto& style = ImGui::GetStyle();
			ImVec2 button_size = ImGui::CalcItemSize(ImVec2(0, 0), label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 10));
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 6));
			ImGui::SetNextItemWidth(700 - button_size.x - style.FramePadding.x * 2.0f - style.ItemInnerSpacing.x - 1);
			ImGui::InputTextWithHint("##phoenix_install_location", "C:/Users/user/Phoenix", m_Properties.InstallPath.data(), MAX_PROJECT_FILEPATH_LENGTH, ImGuiInputTextFlags_ReadOnly);
			ImGui::SameLine();
			if (ImGui::Button("..."))
			{
				std::string result = FileDialogs::BrowseFolder();
				m_Properties.InstallPath.assign(result);
			}

			if (ImGui::Button("Confirm"))
			{
				bool success = Environment::SetEnvironmentalVariable("PHOENIX_DIR", m_Properties.InstallPath);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Close"))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::PopStyleVar(2);
			ImGui::EndPopup();
		}
	}

	void LauncherLayer::OnEvent(Event& e)
	{
	}
	void LauncherLayer::CreateProject(std::string name, std::string projectPath)
	{
		if (!std::filesystem::exists(projectPath))
			std::filesystem::create_directories(projectPath);

		Ref<Project> newProj = Project::Create(name, projectPath);
		ProjectSerializer serializer(newProj);
		serializer.Serialize(projectPath + "\\" + name + ".phxproj");
	}
}