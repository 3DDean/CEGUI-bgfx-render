#pragma once
//#define CEGUIBASE_EXPORTS
#include "CEGUI/Base.h"
#include "CEGUI/Renderer.h"
#include "CEGUI/Size.h"
#include "CEGUI/Vector.h"
#include <map>
#include <set>
#include <vector>
#include <string>
#include <bgfx/bgfx.h>
//#include <GuiBgfxRenderTarget.h>
//using namespace std;
//TODO look into outputing 


namespace CEGUI 
{
	class GuiBgfxTexture;
	class GuiBgfxTextureTarget;
	class GuiBgfxGeometry;


	class GuiBgfxRenderer : public Renderer//, public Singleton<GuiBgfxRenderer>
	{
	public:
		static GuiBgfxRenderer& bootstrapSystem(const char* vsFileLocation, const char* fsFileLocation);
		static GuiBgfxRenderer& create(const char* vsFileLocation, const char* fsFileLocation);

		void destroy();
		void updateScreenSize(int width, int height);
		~GuiBgfxRenderer();

		// Inherited via Renderer
		virtual RenderTarget & getDefaultRenderTarget() override;
		virtual GeometryBuffer & createGeometryBuffer() override;
		virtual void destroyGeometryBuffer(const GeometryBuffer & buffer) override;
		virtual void destroyAllGeometryBuffers() override;
		virtual TextureTarget * createTextureTarget() override;
		virtual void destroyTextureTarget(TextureTarget * target) override;
		virtual void destroyAllTextureTargets() override;
		virtual Texture & createTexture(const String & name) override;
		virtual Texture & createTexture(const String & name, const String & filename, const String & resourceGroup) override;
		virtual Texture & createTexture(const String & name, const Sizef & size) override;
		virtual void destroyTexture(Texture & texture) override;
		virtual void destroyTexture(const String & name) override;
		virtual void destroyAllTextures() override;
		virtual Texture & getTexture(const String & name) const override;
		virtual bool isTextureDefined(const String & name) const override;
		virtual void beginRendering() override;
		virtual void endRendering() override;
		virtual void setDisplaySize(const Sizef & size) override;
		virtual const Sizef & getDisplaySize() const override {
			return d_displaySize;
		};
		virtual const Vector2f & getDisplayDPI() const override;
		virtual uint getMaxTextureSize() const override;
		virtual const String & getIdentifierString() const override;

		//void activateTarget(GuiBgfxRenderTarget* target);
		//void activateRenderTarget();
		 
		void setAllocator(bx::AllocatorI* alloc);
		bgfx::ViewId getViewID() const;
		void setViewID(bgfx::ViewId value) { d_viewId = value; };

	private:
		//! helper to throw exception if name is already used.
		void throwIfNameExists(const String& name) const;
		//! helper to safely log the creation of a named texture
		static void logTextureCreation(const String& name);
		//! helper to safely log the destruction of a named texture
		static void logTextureDestruction(const String& name);

		//Rectf screenArea;
		//Sizef screenSize;
		//! What the renderer considers to be the current display size.
		Sizef d_displaySize;
		//! What the renderer considers to be the current display DPI resolution.
		Vector2f d_displayDPI;
		//! Container type to hold texture Targets
		typedef std::vector<GuiBgfxTexture*> TextureTargetList;
		//! Container used to track Texture Targets
		TextureTargetList d_textureTargets;
		//! Container type to hold Geomitry Buffers
		typedef std::vector<GuiBgfxGeometry*> GeometryBufferList;
		//! Container used to track geomitryBuffers
		GeometryBufferList d_geometryBuffers;
		//! The Default RenderTareget
		RenderTarget* d_defaultTarget = nullptr;

		//! Container type to hold  render Targets
		//typedef std::vector<GuiBgfxRenderTarget*> RenderTargetList;
		////! Container used to track render targets
		//RenderTargetList d_renderBuffers;
		//! Container type to hold textures
		typedef std::map<String, GuiBgfxTexture*, StringFastLessCompare
					CEGUI_MAP_ALLOC(String, GuiBgfxTexture*)> TextureMap;
		
		TextureMap d_textures;

		bgfx::ViewId d_viewId;

		bgfx::ProgramHandle d_program;
		bgfx::UniformHandle d_textureUniform;
		bx::AllocatorI* d_allocator;

		GuiBgfxRenderer(const char* vsFileLocation, const char* fsFileLocation);

		
	};
}
#if defined(_MSC_VER)
#   pragma warning(pop)
#endif

