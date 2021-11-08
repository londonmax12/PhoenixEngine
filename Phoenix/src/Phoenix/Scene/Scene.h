#pragma once

#include "Phoenix/Time/DeltaTime.h"
#include "Phoenix/Renderer/EditorCamera.h"

#include "entt.hpp"

class b2World;

namespace phx {
	class Entity;

	class Scene
	{
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name = std::string());
		void DestroyEntity(Entity entity);

		entt::registry& Reg() { return m_Registry; }

		uint32_t GetRegistrySize() { return m_Registry.size(); };

		std::string GetFilePath() { return m_Filepath; }
		void SetFilePath(std::string& filepath) { m_Filepath = filepath; }

		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnUpdateRuntime(DeltaTime dt);
		void OnUpdateEditor(DeltaTime dt, EditorCamera& camera);
		void OnViewportResize(uint32_t width, uint32_t height);

		Entity GetPrimaryCameraEntity();
	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

		std::string m_Filepath;

		entt::registry m_Registry;

		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		b2World* m_PhysicsWorld = nullptr;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};
}