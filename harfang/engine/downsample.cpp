// HARFANG(R) Copyright (C) 2021 Emmanuel Julien, NWNC HARFANG. Released under GPL/LGPL/Commercial Licence, see licence.txt for details.

#include "engine/downsample.h"

#include "foundation/file_rw_interface.h"
#include "foundation/format.h"
#include "foundation/projection.h"

#include "engine/assets_rw_interface.h"

namespace hg {

static Downsample _CreateDownsample(const Reader &ir, const ReadProvider &ip, const char *path) {
    Downsample down;
	down.compute = hg::LoadProgram(ir, ip, hg::format("%1/shader/aaa_downsample").arg(path));
	down.u_color = bgfx::createUniform("u_color", bgfx::UniformType::Sampler);
	down.u_attr0 = bgfx::createUniform("u_attr0", bgfx::UniformType::Sampler);
	down.u_depth = bgfx::createUniform("u_depth", bgfx::UniformType::Sampler);

	static const uint64_t flags =
		0 | BGFX_TEXTURE_RT | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;

	down.color = {flags, bgfx::createTexture2D(bgfx::BackbufferRatio::Half, false, 1, bgfx::TextureFormat::RGBA32F, flags)};
	bgfx::setName(down.color.handle, "color.downsampled");
    
	down.attr0 = {flags, bgfx::createTexture2D(bgfx::BackbufferRatio::Half, false, 1, bgfx::TextureFormat::RGBA16F, flags)};
	bgfx::setName(down.attr0.handle, "attr0.downsampled");

	down.depth = {flags, bgfx::createTexture2D(bgfx::BackbufferRatio::Half, false, 1, bgfx::TextureFormat::R32F, flags)};
	bgfx::setName(down.depth.handle, "depth.downsampled");

	bgfx::TextureHandle texs[] = {down.color.handle, down.attr0.handle, down.depth.handle};
	down.fb = bgfx::createFrameBuffer(3, texs, true);
	bgfx::setName(down.fb, "Downsample FB");

    return down;
}

Downsample CreateDownsampleFromFile(const char *path) { return _CreateDownsample(g_file_reader, g_file_read_provider, path); }
Downsample CreateDownsampleFromAssets(const char *path) { return _CreateDownsample(g_assets_reader, g_assets_read_provider, path); }

void DestroyDownsample(Downsample &down) {
	bgfx_Destroy(down.compute);
	bgfx_Destroy(down.u_color);
	bgfx_Destroy(down.u_attr0);
	bgfx_Destroy(down.u_depth);
	bgfx_Destroy(down.fb);

	down.compute = BGFX_INVALID_HANDLE;
	down.u_color = BGFX_INVALID_HANDLE;
	down.u_attr0 = BGFX_INVALID_HANDLE;
	down.fb = BGFX_INVALID_HANDLE;
	down.color = BGFX_INVALID_HANDLE;
	down.attr0 = BGFX_INVALID_HANDLE;
	down.depth = BGFX_INVALID_HANDLE;
}

// [todo] use a pyramid
void ComputeDownsample(bgfx::ViewId &view_id, const iRect &rect, const Texture &color, const Texture &attr0, const Texture &depth, const Downsample &down) {
	const bgfx::Caps *caps = bgfx::getCaps();

	bgfx::TransientIndexBuffer idx;
	bgfx::TransientVertexBuffer vtx;
	CreateFullscreenQuad(idx, vtx);

	float ortho[16];
	memcpy(ortho, hg::to_bgfx(hg::Compute2DProjectionMatrix(0.f, 100.f, 1.f, 1.f, false)).data(), sizeof(float[16]));
	
	bgfx::setViewName(view_id, "Downsample");
	bgfx::setViewRect(view_id, rect.sx, rect.sy, GetWidth(rect), GetHeight(rect));
	bgfx::setViewFrameBuffer(view_id, down.fb);
	bgfx::setViewTransform(view_id, NULL, ortho);
	bgfx::setViewClear(view_id, BGFX_CLEAR_NONE, 0, 1.f, UINT8_MAX);
	bgfx::setTexture(0, down.u_color, color.handle, uint32_t(color.flags));
    bgfx::setTexture(1, down.u_attr0, attr0.handle, uint32_t(attr0.flags));
	bgfx::setTexture(2, down.u_depth, depth.handle, uint32_t(depth.flags));

	bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_DEPTH_TEST_ALWAYS);
	bgfx::setIndexBuffer(&idx);
	bgfx::setVertexBuffer(0, &vtx);
	bgfx::submit(view_id, down.compute);
	
	view_id++;
}

} // namespace hg
