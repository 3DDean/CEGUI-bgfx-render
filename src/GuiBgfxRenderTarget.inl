#include "GuiBgfxRenderTarget.h"
#include "CEGUI/RenderQueue.h"
#include "GuiBgfxGeometry.h"
#include <bgfx/bgfx.h>
#include <bx/math.h>
#include <debugdraw/DebugDrawImplentation.h>

namespace CEGUI
{
	template <typename T>
	const double GuiBgfxRenderTarget<T>::d_yfov_tan = 0.267949192431123;

	template <typename T>
	GuiBgfxRenderTarget<T>::GuiBgfxRenderTarget(GuiBgfxRenderer& owner) :
		d_owner(owner),
		d_area(0, 0, 0, 0),
		d_viewDistance(0),
		d_matrixValid(false),
		d_matrix{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }
	{
		
	}

	template <typename T>
	void GuiBgfxRenderTarget<T>::draw(const GeometryBuffer & buffer)
	{
		buffer.draw();
	}

	template <typename T>
	void GuiBgfxRenderTarget<T>::draw(const RenderQueue & queue)
	{
		queue.draw();
	}

	template <typename T>
	void GuiBgfxRenderTarget<T>::setArea(const Rectf & area)
	{
		d_area = area;
		d_matrixValid = false;

		RenderTargetEventArgs args(this);
		T::fireEvent(RenderTarget::EventAreaChanged, args);
		//CEGUI_THROW(new RendererException("Set area supported in render target"));
	}

	template <typename T>
	const Rectf & GuiBgfxRenderTarget<T>::getArea() const
	{
		return d_area;
	}

	template <typename T>
	bool GuiBgfxRenderTarget<T>::isImageryCache() const
	{
		return false;
	}

	template <typename T>
	void GuiBgfxRenderTarget<T>::activate()
	{
		//d_owner.activateTarget(this);
		if (!d_matrixValid)
			updateMatrix();
		
		DebugDrawImplentation temp;
		bgfx::setViewTransform(d_owner.getViewID(), d_matrix, d_ortho);
		bgfx::setViewRect(d_owner.getViewID(), 0, 0, d_area.getWidth(), d_area.getHeight());
		//bgfx::setViewRect(d_owner.getViewID(), 0, 0, 1040, 480);
		//bgfx::setPaletteColor(d_owner.getViewID(), (uint32_t)0x00000000);
		bgfx::setViewClear(d_owner.getViewID(), 0);
		temp.begin(1);
		float mtx[16];
		bx::mtxMul(mtx, d_matrix, d_ortho);
		temp.drawFrustum(mtx);
		temp.close();
	}

	template <typename T>
	void GuiBgfxRenderTarget<T>::deactivate()
	{
	}
	static bool bgfxunProject(bx::Vec3 win,
		const float modelMatrix[16],
		const float projMatrix[16],
		const float viewport[4],
		bx::Vec3 *obj);
	static bool bgfxProject(bx::Vec3 obj,
		const float modelMatrix[16],
		const float projMatrix[16],
		const float viewport[4],
		bx::Vec3 *win);






	template <typename T>
	void GuiBgfxRenderTarget<T>::unprojectPoint(const GeometryBuffer & buff, const Vector2f & p_in, Vector2f & p_out) const
	{
		if (!d_matrixValid)
			updateMatrix();


		const GuiBgfxGeometry& gb = static_cast<const GuiBgfxGeometry&>(buff);

		const float vp[4] = {
			static_cast<float>(d_area.left()),
			static_cast<float>(d_area.top()),
			static_cast<float>(d_area.getWidth()),
			static_cast<float>(d_area.getHeight())
		};

		bx::Vec3 in{ 0.0 };

		// unproject the ends of the ray
		bx::Vec3 r1;
		bx::Vec3 r2;
		in.x = vp[2] * 0.5;
		in.y = vp[3] * 0.5;
		in.z = -d_viewDistance;

		float gb_matrixd[16], d_matrixd[16];
		for (uint i = 0; i < 16; ++i)
		{
			gb_matrixd[i] = gb.getMatrix()[i];
			d_matrixd[i] = d_matrix[i];
		}

		bgfxunProject(in, gb_matrixd, d_matrixd, vp,
			&r1);
		in.x = p_in.d_x;
		in.y = vp[3] - p_in.d_y;
		in.z = 0.0;
		bgfxunProject(in, gb_matrixd, d_matrixd, vp,
			&r2);

		// project points to orientate them with GeometryBuffer plane
		bx::Vec3 p1;
		bx::Vec3 p2;
		bx::Vec3 p3;
		in.x = 0.0;
		in.y = 0.0;
		bgfxProject(in, gb_matrixd, d_matrixd, vp,
			&p1);
		in.x = 1.0;
		in.y = 0.0;
		bgfxProject(in, gb_matrixd, d_matrixd, vp,
			&p2);
		in.x = 0.0;
		in.y = 1.0;
		bgfxProject(in, gb_matrixd, d_matrixd, vp,
			&p3);

		// calculate vectors for generating the plane
		const double pv1_x = p2.x - p1.x;
		const double pv1_y = p2.y - p1.y;
		const double pv1_z = p2.z - p1.z;

		const double pv2_x = p3.x - p1.x;
		const double pv2_y = p3.y - p1.y;
		const double pv2_z = p3.z - p1.z;
		// given the vectors, calculate the plane normal
		const double pn_x = pv1_y * pv2_z - pv1_z * pv2_y;
		const double pn_y = pv1_z * pv2_x - pv1_x * pv2_z;
		const double pn_z = pv1_x * pv2_y - pv1_y * pv2_x;
		// calculate plane
		const double pn_len = std::sqrt(pn_x * pn_x + pn_y * pn_y + pn_z * pn_z);
		const double pl_a = pn_x / pn_len;
		const double pl_b = pn_y / pn_len;
		const double pl_c = pn_z / pn_len;
		const double pl_d = -(p1.x * pl_a + p1.y * pl_b + p1.z * pl_c);
		// calculate vector of picking ray
		const double rv_x = r1.x - r2.x;
		const double rv_y = r1.y - r2.y;
		const double rv_z = r1.z - r2.z;
		// calculate intersection of ray and plane
		const double pn_dot_r1 = (r1.x * pn_x + r1.y * pn_y + r1.z * pn_z);
		const double pn_dot_rv = (rv_x * pn_x + rv_y * pn_y + rv_z * pn_z);
		const double tmp1 = pn_dot_rv != 0.0 ? (pn_dot_r1 + pl_d) / pn_dot_rv : 0.0;
		const double is_x = r1.x - rv_x * tmp1;
		const double is_y = r1.y - rv_y * tmp1;

		p_out.d_x = static_cast<float>(is_x);
		p_out.d_y = static_cast<float>(is_y);
		//Inverse of Render view and geometry buffer then an intersect of the plain
		//bx::mulH(input, static_cast<GuiBgfxGeometry*>(buff*) .getInvMatrix);

	}

	template <typename T>
	void GuiBgfxRenderTarget<T>::updateMatrix() const
	{
		const int w = d_area.getWidth();
		const int h = d_area.getHeight();
		const float aspect = w / h;
		const float midx = w * 0.5;
		const float midy = h * 0.5;
		d_viewDistance = midx / (aspect * d_yfov_tan);
		bx::mtxOrtho(d_ortho, -w / 2.0f, w/2.0f, h / 2.0f, -h / 2.0f, 0, d_viewDistance*2.0f, 0, false);

		bx::Vec3 at{ midx, midy, 0.0f };
		bx::Vec3 eye{ midx, midy, (float)-d_viewDistance };
		bx::Vec3 up{ 0.0f, 1.0f, 0.0f };

		bx::mtxLookAt(d_matrix, eye, at, up);


		
		d_matrixValid = true;
	}

	bool bgfxunProject(bx::Vec3 win,
		const float modelMatrix[16],
		const float projMatrix[16],
		const float viewport[4],
		bx::Vec3 *obj)
	{
		float finalMatrix[16];
		float in[4];
		float out[4];

		bx::mtxMul(finalMatrix, modelMatrix, projMatrix);
		bx::mtxInverse(finalMatrix, finalMatrix);

		in[0] = win.x;
		in[1] = win.y;
		in[2] = win.z;
		in[3] = 1.0;

		/* Map x and y from window coordinates */
		in[0] = (in[0] - viewport[0]) / viewport[2];
		in[1] = (in[1] - viewport[1]) / viewport[3];

		/* Map to range -1 to 1 */
		in[0] = in[0] * 2 - 1;
		in[1] = in[1] * 2 - 1;
		in[2] = in[2] * 2 - 1;
		bx::vec4MulMtx(out, in, finalMatrix);
		if (out[3] == 0.0) return(false);
		out[0] /= out[3];
		out[1] /= out[3];
		out[2] /= out[3];

		(*obj).x = out[0];
		(*obj).y = out[1];
		(*obj).z = out[2];
		return(true);
	}

	bool bgfxProject(bx::Vec3 obj,
		const float modelMatrix[16],
		const float projMatrix[16],
		const float viewport[4],
		bx::Vec3 *win)
	{
		float in[4];
		float out[4];

		in[0] = obj.x;
		in[1] = obj.y;
		in[2] = obj.z;
		in[3] = 1.0;
		bx::vec4MulMtx(out, in, modelMatrix);
		bx::vec4MulMtx(in, out, modelMatrix);

		if (in[3] == 0.0) return(false);
		in[0] /= in[3];
		in[1] /= in[3];
		in[2] /= in[3];
		/* Map x, y and z to range 0-1 */
		in[0] = in[0] * 0.5 + 0.5;
		in[1] = in[1] * 0.5 + 0.5;
		in[2] = in[2] * 0.5 + 0.5;

		/* Map x,y to viewport */
		in[0] = in[0] * viewport[2] + viewport[0];
		in[1] = in[1] * viewport[3] + viewport[1];

		(*win).x = in[0];
		(*win).y = in[1];
		(*win).z = in[2];
		return(true);
	}
}