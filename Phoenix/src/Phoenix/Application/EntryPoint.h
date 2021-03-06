#pragma once

#include "Phoenix/Application/Application.h"

#ifdef PHX_PLATFORM_WINDOWS

extern phx::Application* phx::CreateApplication(ApplicationCommandLineArgs args);

int main(int argc, char** argv)
{
	phx::Log::Init();

	PHX_PROFILE_BEGIN_SESSION("Startup", "PhoenixSesson-Startup.json");
	auto app = phx::CreateApplication({ argc, argv });
	app->OnInit();
	PHX_PROFILE_END_SESSION();

	PHX_PROFILE_BEGIN_SESSION("Runtime", "PhoenixSesson-Runtime.json");
	app->Run();
	PHX_PROFILE_END_SESSION();

	PHX_PROFILE_BEGIN_SESSION("Shutdown", "PhoenixSesson-Shutdown.json");
	delete app;
	PHX_PROFILE_END_SESSION();
}
#endif

