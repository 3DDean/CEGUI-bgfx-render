#include "GuiBgfxRenderer.h"
#include "GuiBgfxGeometry.h"
#include "GuiBgfxTextureTarget.h"
#include "GuiBgfxViewportTarget.h"
#include <Util.h>
#include <bx/file.h>
#include <bgfx/bgfx.h>
class FileReader : public bx::FileReader
{
	typedef bx::FileReader super;

public:
	virtual bool open(const bx::FilePath& _filePath, bx::Error* _err) override
	{
		CEGUI::String filePath(_filePath.getCPtr());
		//filePath.append(_filePath.get());
		return super::open(filePath.c_str(), _err);
	}
};

namespace CEGUI
{
	static bx::DefaultAllocator defaultAlloc;

	static const bgfx::Memory* loadMem(bx::FileReaderI* _reader, const char* _filePath)
	{
		if (bx::open(_reader, _filePath))
		{
			uint32_t size = (uint32_t)bx::getSize(_reader);
			const bgfx::Memory* mem = bgfx::alloc(size + 1);
			bx::read(_reader, mem->data, size);
			bx::close(_reader);
			mem->data[mem->size - 1] = '\0';
			return mem;
		}

		//DBG("Failed to load %s.", _filePath);
		return NULL;
	}

	static bgfx::ShaderHandle loadShader(bx::FileReaderI* _reader, const char* _name)
	{
		char filePath[512];

		const char* shaderPath = "???";

		switch (bgfx::getRendererType())
		{
		case bgfx::RendererType::Noop:
		case bgfx::RendererType::Direct3D9:  shaderPath = "shaders/dx9/";   break;
		case bgfx::RendererType::Direct3D11:
		case bgfx::RendererType::Direct3D12: shaderPath = "shaders/dx11/";  break;
		case bgfx::RendererType::Gnm:        shaderPath = "shaders/pssl/";  break;
		case bgfx::RendererType::Metal:      shaderPath = "shaders/metal/"; break;
		case bgfx::RendererType::OpenGL:     shaderPath = "shaders/glsl/";  break;
		case bgfx::RendererType::OpenGLES:   shaderPath = "shaders/essl/";  break;
		case bgfx::RendererType::Vulkan:     shaderPath = "shaders/spirv/"; break;

		case bgfx::RendererType::Count:
			BX_CHECK(false, "You should not be here!");
			break;
		}

		bx::strCopy(filePath, BX_COUNTOF(filePath), shaderPath);
		bx::strCat(filePath, BX_COUNTOF(filePath), _name);
		bx::strCat(filePath, BX_COUNTOF(filePath), ".bin");
		bgfx::ShaderHandle handle = bgfx::createShader(loadMem(_reader, filePath));;


		bgfx::setName(handle, filePath);

		return handle;
	}


	GuiBgfxRenderer::GuiBgfxRenderer(const char* vsFileLocation, const char* fsFileLocation)
	{
		//d_renderBuffers.push_back(new GuiBgfxRenderTarget(*this));
		d_allocator = &defaultAlloc;
		const bgfx::Stats* temp = bgfx::getStats();

		//bx::FileReaderI* s_fileReader = 
		//	BX_NEW(d_allocator, FileReader);;

		//bgfx::ShaderHandle vsh = loadShader(s_fileReader, vsFileLocation);
		//bgfx::ShaderHandle fsh = BGFX_INVALID_HANDLE;
		//if (NULL != fsFileLocation)
		//{
		//	fsh = loadShader(s_fileReader, fsFileLocation);
		//}

		//d_program = bgfx::createProgram(vsh, fsh, true /* destroy shaders when program is destroyed */);
		d_program = loadProgram(vsFileLocation, fsFileLocation);
		d_textureUniform = bgfx::createUniform("s_texture0", bgfx::UniformType::Sampler);
		updateScreenSize(bgfx::getStats()->width, bgfx::getStats()->height);

		//BX_DELETE(d_allocator, s_fileReader);

		d_viewId = 1;
		

		//screenArea ;
		d_defaultTarget = new GuiBgfxViewportTarget(*this);
	}

	GuiBgfxRenderer & GuiBgfxRenderer::bootstrapSystem(const char* vsFileLocation, const char* fsFileLocation)
	{
		GuiBgfxRenderer& renderer(create(vsFileLocation, fsFileLocation));

		DefaultResourceProvider* rp = new DefaultResourceProvider();
		System::create(renderer, rp);
		return renderer;
	}

	GuiBgfxRenderer& GuiBgfxRenderer::create(const char* vsFileLocation, const char* fsFileLocation) {
		return *new GuiBgfxRenderer(vsFileLocation, fsFileLocation);
	}

	void GuiBgfxRenderer::destroy()
	{
		destroyAllGeometryBuffers();
		destroyAllTextures();
		destroyAllTextureTargets();
		bgfx::destroy(d_program);
		bgfx::destroy(d_textureUniform);
		delete this;
	}

	void GuiBgfxRenderer::updateScreenSize(int width, int height)
	{
		d_displaySize = Sizef(width, height);
		if(d_defaultTarget != nullptr){
					Rectf screenArea = d_defaultTarget->getArea();
		screenArea.d_max =(Vector2<float>(width, height));

		d_defaultTarget->setArea(screenArea);
		}
		if (System::getSingletonPtr()) {
			RenderTargetEventArgs args(&getDefaultRenderTarget());
			getDefaultRenderTarget().fireEvent(RenderTarget::EventAreaChanged, args);

		}
	}

	GuiBgfxRenderer::~GuiBgfxRenderer()
	{
	}

	RenderTarget & GuiBgfxRenderer::getDefaultRenderTarget()
	{
		return *d_defaultTarget;
	}

	GeometryBuffer & GuiBgfxRenderer::createGeometryBuffer()
	{
		GuiBgfxGeometry* ret = new GuiBgfxGeometry(*this);
		d_geometryBuffers.push_back(ret);
		ret->setProgramHandle(d_program, d_textureUniform);
		return *ret;
	}

	void GuiBgfxRenderer::destroyGeometryBuffer(const GeometryBuffer & buffer)
	{

		GeometryBufferList::iterator i = std::find(d_geometryBuffers.begin(),
			d_geometryBuffers.end(),
			&buffer);

		if (d_geometryBuffers.end() != i) {
			d_geometryBuffers.erase(i);
			delete &buffer;
		}
		else {
			CEGUI_THROW(NullObjectException("Geometry to destroy was not found"));
		}
	}

	void GuiBgfxRenderer::destroyAllGeometryBuffers()
	{
		while (!d_geometryBuffers.empty())
			destroyGeometryBuffer(**d_geometryBuffers.begin());
	}

	TextureTarget * GuiBgfxRenderer::createTextureTarget()
	{
		//TextureTarget* t = new GuiBgfxTextureTarget();

		return nullptr;
	}

	void GuiBgfxRenderer::destroyTextureTarget(TextureTarget * target)
	{
	}

	void GuiBgfxRenderer::destroyAllTextureTargets()
	{
	}

	Texture & GuiBgfxRenderer::createTexture(const String & name)
	{
		GuiBgfxTexture* ret = new GuiBgfxTexture(name);
		ret->setAllocator(d_allocator);
		d_textures[name.c_str()] = ret;
		return *ret;
	}

	Texture & GuiBgfxRenderer::createTexture(const String & name, const String & filename, const String & resourceGroup)
	{
		GuiBgfxTexture *ret = (GuiBgfxTexture*)&createTexture(name);
		ret->setAllocator(d_allocator);
		ret->loadFromFile(filename, resourceGroup);
		return *ret;
	}

	Texture & GuiBgfxRenderer::createTexture(const String & name, const Sizef & size)
	{
		return createTexture(name);
	}

	void GuiBgfxRenderer::destroyTexture(Texture & texture)
	{
		destroyTexture(texture.getName());
	}

	void GuiBgfxRenderer::destroyTexture(const String & ceguiName)
	{
		std::string name = ceguiName.c_str();
		if (d_textures.count(name) != 0) {
			d_textures[name]->destroy();
			delete d_textures[name];
			d_textures.erase(d_textures.find(name));
		}
	}

	void GuiBgfxRenderer::destroyAllTextures()
	{
		for (auto pair : d_textures) {
			pair.second->destroy();
			delete pair.second;
		}
		d_textures.clear();
	}

	Texture & GuiBgfxRenderer::getTexture(const String & name) const
	{
		return *d_textures.at(name.c_str());
	}

	bool GuiBgfxRenderer::isTextureDefined(const String & name) const
	{
		return d_textures.count(name.c_str()) > 0;
	}

	void GuiBgfxRenderer::beginRendering()
	{
		
		//unsigned char pass = 0;
		//d_default
		//for (auto& target : d_renderBuffers) {
		//	if (typeid(*target) == typeid(GuiBgfxTextureTarget)) {
		//		target->setPassId(256 - d_renderBuffers.size() + pass++);
		//	}
		//	else {
		//		target->setPassId(1);
		//	}
		//}
	}

	void GuiBgfxRenderer::endRendering()
	{
	}

	void GuiBgfxRenderer::setDisplaySize(const Sizef & size)
	{
		updateScreenSize(size.d_width, size.d_height);
	}

	const Vector2f & GuiBgfxRenderer::getDisplayDPI() const
	{
		static Vector2f tmp = Vector2f(72, 72);
		return tmp;
	}

	uint GuiBgfxRenderer::getMaxTextureSize() const
	{
		auto caps = bgfx::getCaps();
		return caps->limits.maxTextureSize;
	}

	const String & GuiBgfxRenderer::getIdentifierString() const
	{
		static String tmp = "Bgfx Renderer";
		return tmp;
	}

	//void GuiBgfxRenderer::activateTarget(GuiBgfxRenderTarget * target)
	//{
	//	currentPass = target->getPassId();
	//}

	//void GuiBgfxRenderer::activateRenderTarget()
	//{
	//	activateTarget(*d_renderBuffers.begin());
	//}

	void GuiBgfxRenderer::setAllocator(bx::AllocatorI * alloc)
	{
		d_allocator = alloc;
	}

	bgfx::ViewId GuiBgfxRenderer::getViewID() const
	{
		 return d_viewId; 
	}

	void GuiBgfxRenderer::throwIfNameExists(const String & name) const
	{
		throw "not yet implemented";
	}
}
//template<> GuiBgfxRenderer* Singleton<GuiBgfxRenderer>::ms_Singleton = 0;

