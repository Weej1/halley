#pragma once
#include "entity_editor.h"
#include "entity_list.h"
#include "halley/ui/ui_widget.h"
#include "scene_editor_canvas.h"

namespace Halley {
	class HalleyAPI;
	class Project;
	class UIFactory;

	class SceneEditorWindow final : public UIWidget {
	public:
		SceneEditorWindow(UIFactory& factory, Project& project, const HalleyAPI& api);
		~SceneEditorWindow();

		void loadScene(const String& sceneName);
		void loadPrefab(const String& name);
		void loadScene(const Prefab& prefab);
		void unloadScene();
		void markModified();

		void onEntityAdded(const String& id, const String& parentId);
		void onEntityRemoved(const String& id, const String& parentId);
		void onEntityModified(const String& id);
		void onFieldChangedByGizmo(const String& componentName, const String& fieldName);

		std::shared_ptr<const Prefab> getGamePrefab(const String& id) const;

	protected:
		void update(Time t, bool moved) override;
		
	private:
		UIFactory& uiFactory;
		Project& project;

		std::shared_ptr<SceneEditorCanvas> canvas;
		std::shared_ptr<EntityList> entityList;
		std::shared_ptr<EntityEditor> entityEditor;

		Path assetPath;
		std::shared_ptr<ISceneData> sceneData;
		std::unique_ptr<Prefab> prefab;
		std::shared_ptr<EntityFactory> entityFactory;

		String currentEntityId;

		void makeUI();
		void load();
		void selectEntity(const String& id);
		void panCameraToEntity(const String& id);
		void saveEntity();
		
		void addNewEntity();
		void addNewPrefab();
		void addNewPrefab(const String& prefabName);
		void addEntity(ConfigNode data);
		void addEntity(const String& parentId, ConfigNode data);
		void removeEntity();
		void removeEntity(const String& entityId);
		
		String findParent(const String& entityId) const;
		const String* findParent(const String& entityId, const EntityTree& tree, const String& prev) const;

		void preparePrefab(Prefab& prefab);
		void preparePrefabEntity(ConfigNode& node);
	};
}