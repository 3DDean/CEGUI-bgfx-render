#pragma once
#include "GuiBgfxRenderer.h"
#include "CEGUI/TextureTarget.h"
#include "GuiBgfxRenderTarget.h"
#include "CEGUI/Rect.h"
#include <bgfx/bgfx.h>

#include "GuiBgfxTexture.h"
namespace CEGUI
{
	class GuiBgfxTextureTarget : /*public TextureTarget,*/ public GuiBgfxRenderTarget<TextureTarget>
	{
		bgfx::FrameBufferHandle handle;
		const bgfx::Memory* textureMemory;
	public:
		GuiBgfxTextureTarget(GuiBgfxRenderer& owner);
		~GuiBgfxTextureTarget();

		// Inherited via TextureTarget
		virtual bool isImageryCache() const override;
		virtual void activate() override;
		virtual void deactivate() override;
		virtual void clear() override;
		virtual Texture & getTexture() const override;
		virtual void declareRenderSize(const Sizef & sz) override;
		virtual bool isRenderingInverted() const override;

		GuiBgfxTexture* texture;
		virtual void destroy() override;
	};

}