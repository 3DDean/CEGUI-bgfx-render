#pragma once
#include <CEGUI/CEGUI.h>
#include <bgfx/bgfx.h>
#include <bx/allocator.h>
#include "GuiBgfxRenderer.h"


namespace CEGUI 
{

	class GuiBgfxTexture : public Texture
	{
	public:
		GuiBgfxTexture(String name);
		~GuiBgfxTexture();

		// Inherited via Texture
		virtual const String & getName() const override;
		virtual const Sizef & getSize() const override;
		virtual const Sizef & getOriginalDataSize() const override;
		virtual const Vector2f & getTexelScaling() const override;
		virtual void loadFromFile(const String & filename, const String & resourceGroup) override;
		virtual void loadFromMemory(const void * buffer, const Sizef & buffer_size, Texture::PixelFormat pixel_format) override;
		virtual void loadFromMemory(const bgfx::Memory *mem, const Sizef & buffer_size, bgfx::TextureFormat::Enum format);
		virtual void blitFromMemory(const void * sourceData, const Rectf & area) override;
		virtual void blitToMemory(void * targetData) override;
		virtual bool isPixelFormatSupported(const Texture::PixelFormat fmt) const override;
		void destroy();

		bgfx::TextureHandle getHandle() const;
		void setAllocator(bx::AllocatorI * alloc);
	protected:
		String name;
		Sizef size;
		Vector2f texel;
		mutable bgfx::TextureHandle handle;
		unsigned char* data;
		void setSize(const Sizef & value);
		bx::AllocatorI* allocator;

	
	};

}
