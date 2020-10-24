#pragma once

#include "halley/core/graphics/texture.h"
#include "halley/core/graphics/sprite/animation.h"
#include "halley/core/resources/resources.h"
#include "halley/ui/ui_factory.h"
#include "halley/ui/ui_widget.h"
#include "halley/ui/widgets/ui_animation.h"
#include "assets_editor_window.h"
#include "src/ui/scroll_background.h"

namespace Halley {
	class Project;
	class AnimationEditorDisplay;

	class AnimationEditor : public AssetEditor {
    public:
        AnimationEditor(UIFactory& factory, Resources& resources, AssetType type, Project& project);

        void reload() override;

    protected:
        void update(Time t, bool moved) override;
		
	private:
		void setupWindow();
		void loadAssetData();

        std::shared_ptr<AnimationEditorDisplay> animationDisplay;
        std::shared_ptr<UILabel> info;
        std::shared_ptr<ScrollBackground> scrollBg;
	};

	class AnimationEditorDisplay : public UIWidget {
	public:
		AnimationEditorDisplay(String id, Resources& resources);

		void setZoom(float zoom);
		void setAnimation(std::shared_ptr<const Animation> animation);
		void setSprite(std::shared_ptr<const SpriteResource> sprite);
		void setTexture(std::shared_ptr<const Texture> texture);
		void setSequence(const String& sequence);
		void setDirection(const String& direction);

		const Rect4f& getBounds() const;
		Vector2f getMousePos() const;
		void onMouseOver(Vector2f mousePos) override;

	protected:
		void update(Time t, bool moved) override;
		void draw(UIPainter& painter) const override;
		
	private:
		Resources& resources;
		std::shared_ptr<const Animation> animation;
		AnimationPlayer animationPlayer;

		Sprite origSprite;
		Sprite drawSprite;
		Sprite boundsSprite;
		Sprite nineSliceVSprite;
		Sprite nineSliceHSprite;
		Sprite pivotSprite;
		Rect4f bounds;
		float zoom = 1.0f;
		Vector2f mousePos;

		void updateBounds();
		Vector2f imageToScreenSpace(Vector2f pos) const;
		Vector2f screenToImageSpace(Vector2f pos) const;
	};
}

