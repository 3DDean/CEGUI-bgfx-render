
#include "GuiBgfxGeometry.h"
#include "GuiBgfxRenderer.h"

#include <bx/math.h>
#include <iostream>


namespace CEGUI
{

	void GuiBgfxGeometry::syncHardwareBuffer() const
	{
		size_t sizeToInsert = 0;
		size_t locationOfIsert = 0;
		const size_t vertex_count = d_vertices.size();

		bgfx::update(vertexHandle, 0, bgfx::makeRef(d_vertices.data(), sizeof(d_vertices[0]) * d_vertices.size()));

		d_bufferSynched = true;
	}

	GuiBgfxGeometry::GuiBgfxGeometry(GuiBgfxRenderer & renderer) : owner(renderer)
	{
		d_pivot = Vector3f(0, 0, 0);
		d_translation = Vector3f(0, 0, 0);
		d_effect = NULL;
		d_activeTexture = NULL;

		decl.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true, true)
			.end();

		vertexHandle = bgfx::createDynamicVertexBuffer(10, decl, BGFX_BUFFER_ALLOW_RESIZE);

	}

	GuiBgfxGeometry::~GuiBgfxGeometry()
	{
		bgfx::destroy(vertexHandle);
	}
	float kPiHalf = 1.5707963267948966192313216916398f;
	void GuiBgfxGeometry::updateMatrix() const
	{
		bx::mtxIdentity(matrix);
		Vector3f pivoted = d_pivot + d_translation;
		bx::mtxTranslate(matrix, pivoted.d_x, pivoted.d_y, pivoted.d_z);
		bx::Quaternion quat1 = bx::rotateX(kPiHalf);
		bx::Quaternion quat{ d_rotation.d_x , d_rotation.d_y, d_rotation.d_z, d_rotation.d_w };
		float tmp1[16], tmp2[16];
		bx::mtxQuat(tmp1, quat);
		bx::mtxMul(tmp2, matrix, tmp1);
		memcpy(matrix, tmp2, sizeof(float) * 16);

		bx::mtxIdentity(tmp1);
		bx::mtxTranslate(tmp1, -d_pivot.d_x, -d_pivot.d_y, -d_pivot.d_z);
		bx::mtxMul(tmp2, matrix, tmp1);
		memcpy(matrix, tmp2, sizeof(float) * 16);
		//bx::mtxInverse(invMatrix, matrix);
	}

	void GuiBgfxGeometry::draw() const
	{
		if (!d_bufferSynched)
			syncHardwareBuffer();

		if (!d_matrixValid)
			updateMatrix();

		uint32_t transformCache = bgfx::setTransform(matrix);

		const int pass_count = d_effect ? d_effect->getPassCount() : 1;
		for (int pass = 0; pass < pass_count; ++pass) {

			if (d_effect)
				d_effect->performPreRenderFunctions(pass);
			//Run the batches
			size_t pos = 0;
			BatchList::const_iterator i = d_batches.begin();
			for (; i != d_batches.end(); ++i)
			{
				bgfx::setTransform(transformCache);
				bgfx::setVertexBuffer(0, vertexHandle, pos, i->vertexCount);

				const GuiBgfxTexture* currentBatchTexture = i->texture;
				if (currentBatchTexture || currentBatchTexture->getHandle().idx != bgfx::kInvalidHandle) {
					bgfx::setTexture(0, uniform, currentBatchTexture->getHandle());
				}
				else {
					bgfx::setTexture(0, uniform, BGFX_INVALID_HANDLE);
				}
				bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);
				bgfx::submit(owner.getViewID(), program);
				pos += i->vertexCount;
			}
		}
	}

	void GuiBgfxGeometry::setTranslation(const Vector3f & v)
	{
		d_translation = v;
		d_matrixValid = false;
	}

	void GuiBgfxGeometry::setRotation(const Quaternion & r)
	{
		d_rotation = r;
		d_matrixValid = false;
	}

	void GuiBgfxGeometry::setPivot(const Vector3f & p)
	{
		d_pivot = p;
		d_matrixValid = false;
	}

	void GuiBgfxGeometry::setClippingRegion(const Rectf & region)
	{
		d_clipRect.top(ceguimax(0.0f, region.top()));
		d_clipRect.bottom(ceguimax(0.0f, region.bottom()));
		d_clipRect.left(ceguimax(0.0f, region.left()));
		d_clipRect.right(ceguimax(0.0f, region.right()));
	}

	void GuiBgfxGeometry::appendVertex(const Vertex & vertex)
	{
		appendGeometry(&vertex, 1);
	}

	void GuiBgfxGeometry::appendGeometry(const Vertex* const vbuff, uint vertex_count)
	{
		GuiBgfxTexture* srv = d_activeTexture ? d_activeTexture : 0;

		if (d_batches.empty() ||
			srv != d_batches.back().texture ||
			d_clippingActive != d_batches.back().clip)
		{
			BatchInfo batch = { srv, 0, d_clippingActive };
			d_batches.push_back(batch);
		}
		GuiBgfxGeometry::GuiBgfxVertex *temp = d_vertices.data();

		//Keep track of batch indicies so that one can feed them into 
		uint16_t vc = d_batches.back().vertexCount;
		d_batches.back().vertexCount += vertex_count;

		GuiBgfxVertex vd;
		const Vertex* vs = vbuff;
		//Texture Verices Size	
		String vertices;
		for (uint i = 0; i < vertex_count; ++i, ++vs)
		{
			//Convert from vertex to bgfx format
			vd.x = vs->position.d_x;
			vd.y = vs->position.d_y;
			vd.z = vs->position.d_z;
			vd.u = vs->tex_coords.d_x;
			vd.v = vs->tex_coords.d_y;
			vd.a = vs->colour_val.getAlpha() * 255.0f;
			vd.b = vs->colour_val.getBlue() * 255.0f;
			vd.g = vs->colour_val.getGreen() * 255.0f;
			vd.r = vs->colour_val.getRed() * 255.0f;
			d_vertices.push_back(vd);

		}

		d_bufferSynched = false;
	}


	void GuiBgfxGeometry::setActiveTexture(Texture * texture)
	{
		this->d_activeTexture = static_cast<GuiBgfxTexture*>(texture);
	}

	void GuiBgfxGeometry::reset()
	{
		d_batches.clear();
		d_vertices.clear();
		d_activeTexture = 0;
	}

	Texture * GuiBgfxGeometry::getActiveTexture() const
	{
		return d_activeTexture;
	}

	uint GuiBgfxGeometry::getVertexCount() const
	{
		return d_vertices.size();
	}

	uint GuiBgfxGeometry::getBatchCount() const
	{
		return d_batches.size();
	}

	void GuiBgfxGeometry::setRenderEffect(RenderEffect * effect)
	{
		this->d_effect = effect;
	}

	RenderEffect * GuiBgfxGeometry::getRenderEffect()
	{
		return d_effect;
	}

	void GuiBgfxGeometry::setClippingActive(const bool active)
	{
		d_clippingActive = active;
	}

	bool GuiBgfxGeometry::isClippingActive() const
	{
		return d_clippingActive;
	}

	void GuiBgfxGeometry::setProgramHandle(bgfx::ProgramHandle programHandle, bgfx::UniformHandle uniformHandle)
	{
		program = programHandle;
		uniform = uniformHandle;
	}
	float * GuiBgfxGeometry::getMatrix() const
	{
		return matrix;
	}
}