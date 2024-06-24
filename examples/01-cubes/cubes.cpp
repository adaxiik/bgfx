/*
 * Copyright 2011-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */

#include "common.h"
#include "bgfx_utils.h"
#include "imgui/imgui.h"

namespace
{

struct PosColorVertex
{
	float m_x;
	float m_y;
	uint32_t m_abgr;

	static void init()
	{
		ms_layout
			.begin()
			.add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0,   4, bgfx::AttribType::Uint8, true)
			.end();
	};

	static bgfx::VertexLayout ms_layout;
};

bgfx::VertexLayout PosColorVertex::ms_layout;

#define COLOR_RED 0xff0000ff
#define COLOR_BLUE 0xffff0000

class ExampleCubes : public entry::AppI
{
public:
	ExampleCubes(const char* _name, const char* _description, const char* _url)
		: entry::AppI(_name, _description, _url)
	{
	}

	void init(int32_t _argc, const char* const* _argv, uint32_t _width, uint32_t _height) override
	{
		Args args(_argc, _argv);

		m_width  = _width;
		m_height = _height;
		m_debug  = BGFX_DEBUG_NONE;
		m_reset  = BGFX_RESET_VSYNC;

		bgfx::Init init;
		init.type     = args.m_type;
		init.vendorId = args.m_pciId;
		init.platformData.nwh  = entry::getNativeWindowHandle(entry::kDefaultWindowHandle);
		init.platformData.ndt  = entry::getNativeDisplayHandle();
		init.platformData.type = entry::getNativeWindowHandleType();
		init.resolution.width  = m_width;
		init.resolution.height = m_height;
		init.resolution.reset  = m_reset;
		bgfx::init(init);

		// Enable debug text.
		bgfx::setDebug(m_debug);

		// Set view 0 clear state.
		bgfx::setViewClear(0
			, BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH
			, 0x303030ff
			, 1.0f
			, 0
			);

		PosColorVertex::init();


		m_program = loadProgram("vs_cubes", "fs_cubes");


		// allocating space for single rectangle
		m_vbh = bgfx::createDynamicVertexBuffer(4, PosColorVertex::ms_layout);
		m_ibh = bgfx::createDynamicIndexBuffer(6);

		imguiCreate();
	}

	virtual int shutdown() override
	{
		imguiDestroy();

		bgfx::destroy(m_vbh);
		bgfx::destroy(m_ibh);

		bgfx::destroy(m_program);

		bgfx::shutdown();

		return 0;
	}

	void render_rect(float x, float y, float w, float h, uint32_t color)
	{
		PosColorVertex v[4] = {
			{ x,   y,   color },
			{ x+w, y,   color },
			{ x+w, y+h, color },
			{ x,   y+h, color }
		};
		uint16_t indices[6] = { 0, 1, 2, 2, 3, 0 };

		// both vertex and index buffers are copied
		bgfx::update(m_vbh, 0, bgfx::copy(v, sizeof(v) ) );

		// index buffer is still the same, but I made it dynamic just for the sake of example
		// I got same result even with static index buffer
		bgfx::update(m_ibh, 0, bgfx::copy(indices, sizeof(indices) ) );

		bgfx::setVertexBuffer(0, m_vbh);
		bgfx::setIndexBuffer(m_ibh);
		bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);

		// submit after each function call
		bgfx::submit(0, m_program);
	}

	bool update() override
	{
		if (!entry::processEvents(m_width, m_height, m_debug, m_reset, &m_mouseState) )
		{
			imguiBeginFrame(m_mouseState.m_mx
				,  m_mouseState.m_my
				, (m_mouseState.m_buttons[entry::MouseButton::Left  ] ? IMGUI_MBUT_LEFT   : 0)
				| (m_mouseState.m_buttons[entry::MouseButton::Right ] ? IMGUI_MBUT_RIGHT  : 0)
				| (m_mouseState.m_buttons[entry::MouseButton::Middle] ? IMGUI_MBUT_MIDDLE : 0)
				,  m_mouseState.m_mz
				, uint16_t(m_width)
				, uint16_t(m_height)
				);

			showExampleDialog(this);
			imguiEndFrame();

			bgfx::touch(0);

			{
				float view[16];
				bx::mtxIdentity(view);

				float proj[16];
				bx::mtxOrtho(proj, 0, m_width, 0, m_height, -1, 1, 0, bgfx::getCaps()->homogeneousDepth);
				bgfx::setViewTransform(0, view, proj);

				bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height) );
			}

			render_rect(100, 100, 200, 200, COLOR_RED);

			// even though first rectangle was submitted,
			// second draw call will overwrite its vertex data
			render_rect(400, 400, 200, 200, COLOR_BLUE);


			bgfx::frame();

			return true;
		}

		return false;
	}

	entry::MouseState m_mouseState;

	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_debug;
	uint32_t m_reset;

	bgfx::ProgramHandle m_program;
	bgfx::DynamicVertexBufferHandle m_vbh;
	bgfx::DynamicIndexBufferHandle m_ibh;

};

} // namespace

ENTRY_IMPLEMENT_MAIN(
	  ExampleCubes
	, "01-cubes"
	, "Rendering simple static mesh."
	, "https://bkaradzic.github.io/bgfx/examples.html#cubes"
	);
