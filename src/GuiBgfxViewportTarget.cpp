#include "GuiBgfxViewportTarget.h"
#include "CEGUI/Exceptions.h"

namespace CEGUI 
{
	CEGUI::GuiBgfxViewportTarget::GuiBgfxViewportTarget(GuiBgfxRenderer & owner) :
		GuiBgfxRenderTarget<>(owner)
	{
		const bgfx::Stats* stats = bgfx::getStats();
		Rectf init_area(Vector2f(0,0),
						Sizef(stats->width, stats->height));
		setArea(init_area);
	}
	GuiBgfxViewportTarget::GuiBgfxViewportTarget(GuiBgfxRenderer & owner, const Rectf & area) :
		GuiBgfxRenderTarget<>(owner)
	{
		setArea(area);
	}
	bool GuiBgfxViewportTarget::isImageryCache() const
	{
		return false;
	}
}

#include "GuiBgfxRenderTarget.inl"