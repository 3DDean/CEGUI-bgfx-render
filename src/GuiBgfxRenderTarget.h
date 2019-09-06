#pragma once
#include "GuiBgfxRenderer.h"
#include "CEGUI/RenderTarget.h"
#include "CEGUI/Rect.h"

namespace CEGUI
{
	template <typename T = RenderTarget>
	class GuiBgfxRenderTarget : public T
	{

	public:
		GuiBgfxRenderTarget(GuiBgfxRenderer& owner);
		//~GuiBgfxRenderTarget();

		virtual void destroy() {}

		// Inherited via RenderTarget
		virtual void draw(const GeometryBuffer & buffer) override;
		virtual void draw(const RenderQueue & queue) override;
		virtual void setArea(const Rectf & area) override;
		virtual const Rectf & getArea() const override;
		virtual bool isImageryCache() const override;
		virtual void activate() override;
		virtual void deactivate() override;
		virtual void unprojectPoint(const GeometryBuffer & buff, const Vector2f & p_in, Vector2f & p_out) const override;

		void setViewId(uint8_t value) { d_viewId = value; }
		uint8_t getViewId() { return d_viewId; }
	protected:
		//! helper that initialises the cached matrix
		virtual void updateMatrix() const;

		GuiBgfxRenderer& d_owner;
		uint8_t d_viewId = 0;

		Rectf d_area;
		//! tangent of the y FOV half-angle; used to calculate viewing distance.
		static const double d_yfov_tan;
		//! saved copy of projection matrix
		mutable float d_matrix[16];
		//! saved copy of projection matrix
		mutable float d_ortho[16];
		//! true if saved matrix is up to date
		mutable bool d_matrixValid;
		//! tracks viewing distance (this is set up at the same time as d_matrix)
		mutable double d_viewDistance;
	};
}
