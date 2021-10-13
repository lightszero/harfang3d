// HARFANG(R) Copyright (C) 2021 Emmanuel Julien, NWNC HARFANG. Released under GPL/LGPL/Commercial Licence, see licence.txt for details.

#include "engine/upsample.h"

#include "foundation/file_rw_interface.h"
#include "foundation/format.h"
#include "foundation/projection.h"

#include "engine/assets_rw_interface.h"

namespace hg {

static Upsample _CreateUpsample(const Reader &ir, const ReadProvider &ip, const char *path) {
    Upsample up;
	up.compute = hg::LoadProgram(ir, ip, hg::format("%1/shader/aaa_upsample").arg(path));
	up.u_input = bgfx::createUniform("u_input", bgfx::UniformType::Sampler);
	up.u_attr_lo = bgfx::createUniform("u_attr_lo", bgfx::UniformType::Sampler);
	up.u_attr_hi = bgfx::createUniform("u_attr_hi", bgfx::UniformType::Sampler);
    return up;
}

Upsample CreateUpsampleFromFile(const char *path) { return _CreateUpsample(g_file_reader, g_file_read_provider, path); }
Upsample CreateUpsampleFromAssets(const char *path) { return _CreateUpsample(g_assets_reader, g_assets_read_provider, path); }

void DestroyUpsample(Upsample &up) {
	bgfx_Destroy(up.compute);
	bgfx_Destroy(up.u_input);
	bgfx_Destroy(up.u_attr_lo);
	bgfx_Destroy(up.u_attr_hi);
}

void ComputeUpsample(bgfx::ViewId &view_id, const iRect &rect, const Texture &input, const Texture &attr_lo, const Texture &attr_hi, bgfx::FrameBufferHandle output, const Upsample &up) {
	const bgfx::Caps *caps = bgfx::getCaps();

	bgfx::TransientIndexBuffer idx;
	bgfx::TransientVertexBuffer vtx;
	CreateFullscreenQuad(idx, vtx);

	float ortho[16];
	memcpy(ortho, hg::to_bgfx(hg::Compute2DProjectionMatrix(0.f, 100.f, 1.f, 1.f, false)).data(), sizeof(float[16]));

	bgfx::setViewName(view_id, "Upsample");
	bgfx::setViewRect(view_id, rect.sx, rect.sy, GetWidth(rect), GetHeight(rect));
	bgfx::setViewFrameBuffer(view_id, output);
	bgfx::setViewTransform(view_id, NULL, ortho);
	bgfx::setViewClear(view_id, BGFX_CLEAR_NONE, 0, 1.f, UINT8_MAX);
	bgfx::setTexture(0, up.u_input, input.handle, uint32_t(input.flags));
    bgfx::setTexture(1, up.u_attr_lo, attr_lo.handle, uint32_t(attr_lo.flags));
    bgfx::setTexture(2, up.u_attr_hi, attr_hi.handle, uint32_t(attr_hi.flags));

	bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_DEPTH_TEST_ALWAYS);
	bgfx::setIndexBuffer(&idx);
	bgfx::setVertexBuffer(0, &vtx);
	bgfx::submit(view_id, up.compute);
	view_id++;
}

} // namespace hg
