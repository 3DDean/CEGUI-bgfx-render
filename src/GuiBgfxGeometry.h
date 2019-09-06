#pragma once
#include <CEGUI/CEGUI.h>
#include "GuiBgfxRenderer.h"
#include <vector>
#include "GuiBgfxTexture.h"
#include "spdlog/fmt/ostr.h" // must be included

//using namespace std;

//class GuiBgfxRenderer;
namespace CEGUI {

	class GuiBgfxRenderer;
	class GuiBgfxGeometry : public GeometryBuffer
	{

	public:
		GuiBgfxGeometry(GuiBgfxRenderer & owner);
		~GuiBgfxGeometry();

		// Inherited via GeometryBuffer
		virtual void draw() const override;
		virtual void setTranslation(const Vector3f & v) override;
		virtual void setRotation(const Quaternion & r) override;
		virtual void setPivot(const Vector3f & p) override;
		virtual void setClippingRegion(const Rectf & region) override;
		virtual void appendVertex(const Vertex & vertex) override;
		virtual void appendGeometry(const Vertex * const vbuff, uint vertex_count) override;
		virtual void setActiveTexture(Texture * texture) override;
		virtual void reset() override;
		virtual Texture * getActiveTexture() const override;
		virtual uint getVertexCount() const override;
		virtual uint getBatchCount() const override;
		virtual void setRenderEffect(RenderEffect * effect) override;
		virtual RenderEffect * getRenderEffect() override;
		virtual void setClippingActive(const bool active) override;
		virtual bool isClippingActive() const override;

		void setProgramHandle(bgfx::ProgramHandle programHandle, bgfx::UniformHandle uniformHandle);

		float* getMatrix() const;
	protected:
		bgfx::ProgramHandle program;
		bgfx::UniformHandle uniform;

		//! update cached matrix
		void updateMatrix() const;
		//! Synchronise data in the hardware buffer with what's been added
		void syncHardwareBuffer() const;

		struct GuiBgfxVertex {

			float x, y, z;
			float u, v;
			unsigned char a, g, b, r;

		};
		//TODO Make it so that all of the 
		struct BatchInfo
		{
			const GuiBgfxTexture* texture;
			uint vertexCount;
			bool clip;
		};
		//View to Draw to
		//bgfx::ViewId view = 0;
		//! last texture that was set as active
		GuiBgfxTexture* d_activeTexture;
		//! whether the h/w buffer is in sync with the added geometry
		mutable bool d_bufferSynched;
		//! type of container that tracks BatchInfos.
		typedef std::vector<BatchInfo> BatchList;
		//! list of texture batches added to the geometry buffer
		BatchList d_batches;
		//! type of container used to queue the geometry
		typedef std::vector<GuiBgfxVertex> VertexList;
		//! container where added geometry is stored.
		VertexList d_vertices;
		//! rectangular clip region
		Rectf d_clipRect;
		//! whether clipping will be active for the current batch
		bool d_clippingActive;
		//! translation vector
		Vector3f d_translation;
		//! rotation vector
		Quaternion d_rotation;
		//! pivot point for rotation
		Vector3f d_pivot;
		//! RenderEffect that will be used by the GeometryBuffer
		RenderEffect* d_effect;

		GuiBgfxRenderer& owner;

		bgfx::VertexDecl decl;

		mutable bool d_matrixValid;

		mutable float matrix[16];

		mutable float invMatrix[16];

		mutable bgfx::DynamicVertexBufferHandle vertexHandle;

	};
};
