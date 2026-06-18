// RING RACERS
//-----------------------------------------------------------------------------
// Copyright (C) 2023 by James Robert Roman
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <exception>
#include <memory>
#include <stdexcept>

// #include <fmt/format.h>
#include "media/tcb/span.hpp"

#include "m_avrecorder.hpp"

// romoney5: ugh..
extern "C" {
#include "command.h"
#include "doomdef.h"
#include "doomtype.h"
#include "i_sound.h"
#include "m_anigif.h"
#include "m_avrecorder.h"
#include "m_fixed.h"
#include "m_misc.h"
#include "screen.h"	  // vid global
#include "st_stuff.h" // st_palette
#include "v_video.h"  // pLocalPalette

#ifdef HWRENDER
#include "hardware/hw_main.h"
#endif
}

#include "media/vorbis.hpp"
#include "media/vp8.hpp"

using namespace srb2::media;

namespace
{

namespace Res
{

// Using an unscoped enum here so it can implicitly cast to
// int (in CV_PossibleValue_t). Wrap this in a namespace so
// access is still scoped. E.g. Res::kGame

enum : int32_t
{
	kGame, // user chosen resolution, vid.width
	kBase, // smallest version maintaining aspect ratio, vid.width / vid.dupx
	kBase2x,
	kBase4x,
	kWindow, // window size (monitor in fullscreen), vid.realwidth
	kCustom, // movie_custom_resolution
};

}; // namespace Res

CV_PossibleValue_t movie_resolution_cons_t[] = {
	{Res::kGame, "Native"},
	{Res::kBase, "Small"},
	{Res::kBase2x, "Medium"},
	{Res::kBase4x, "Large"},
	{Res::kWindow, "Window"},
	{Res::kCustom, "Custom"},
	{0, NULL}};

CV_PossibleValue_t movie_limit_cons_t[] = {{1, "MIN"}, {INT32_MAX, "MAX"}, {0, "Unlimited"}, {0, NULL}};

}; // namespace

consvar_t cv_movie_resolution = CVAR_INIT("movie_resolution", "Medium", CV_SAVE, movie_resolution_cons_t, NULL);
consvar_t cv_movie_custom_resolution = CVAR_INIT("movie_custom_resolution", "640x400", CV_SAVE, NULL, NULL);

consvar_t cv_movie_fps = CVAR_INIT("movie_fps", "60", CV_SAVE, CV_Natural, NULL);
consvar_t cv_movie_showfps = CVAR_INIT("movie_showfps", "Yes", CV_SAVE, CV_YesNo, NULL);

consvar_t cv_movie_sound = CVAR_INIT("movie_sound", "On", CV_SAVE, CV_OnOff, NULL);

consvar_t cv_movie_duration = CVAR_INIT("movie_duration", "Unlimited", CV_SAVE | CV_FLOAT, movie_limit_cons_t, NULL);

// romoney5: encoder options

static CV_PossibleValue_t vorbis_quality_cons_t[] = {{FLOAT_TO_FIXED(-0.1f), "MIN"}, {FLOAT_TO_FIXED(1.f), "MAX"}, {0, NULL}};
consvar_t cv_vorbis_quality = CVAR_INIT("vorbis_quality", "0", CV_SAVE | CV_FLOAT, vorbis_quality_cons_t, NULL);
static CV_PossibleValue_t vorbis_max_bitrate_cons_t[] = {{-1, "MIN"}, {INT32_MAX, "MAX"}, {0, NULL}};
consvar_t cv_vorbis_max_bitrate = CVAR_INIT("vorbis_max_bitrate", "-1", CV_SAVE, vorbis_max_bitrate_cons_t, NULL);
consvar_t cv_vorbis_nominal_bitrate = CVAR_INIT("vorbis_nominal_bitrate", "-1", CV_SAVE, vorbis_max_bitrate_cons_t, NULL);
consvar_t cv_vorbis_min_bitrate = CVAR_INIT("vorbis_min_bitrate", "-1", CV_SAVE, vorbis_max_bitrate_cons_t, NULL);

static CV_PossibleValue_t vp8_quality_mode_cons_t[] = {{VPX_VBR, "vbr"}, {VPX_CBR, "cbr"}, {VPX_CQ, "cq"}, {VPX_Q, "q"}, {0, NULL}};
consvar_t cv_vp8_quality_mode = CVAR_INIT("vp8_quality_mode", "q", CV_SAVE, vp8_quality_mode_cons_t, NULL);
consvar_t cv_vp8_target_bitrate = CVAR_INIT("vp8_target_bitrate", "800", CV_SAVE, CV_Natural, NULL);
static CV_PossibleValue_t vp8_min_q_cons_t[] = {{4, "MIN"}, {63, "MAX"}, {0, NULL}};
consvar_t cv_vp8_min_q = CVAR_INIT("vp8_min_q", "4", CV_SAVE, vp8_min_q_cons_t, NULL);
consvar_t cv_vp8_max_q = CVAR_INIT("vp8_max_q", "55", CV_SAVE, vp8_min_q_cons_t, NULL);
consvar_t cv_vp8_kf_min = CVAR_INIT("vp8_kf_min", "0", CV_SAVE, CV_Unsigned, NULL);
// static_cast<int>(KeyFrameOption::kAuto) = -1
static CV_PossibleValue_t vp8_kf_max_cons_t[] = {{0, "MIN"}, {INT32_MAX, "MAX"}, {-1, "auto"}, {0, NULL}};
consvar_t cv_vp8_kf_max = CVAR_INIT("vp8_kf_max", "auto", CV_SAVE, vp8_kf_max_cons_t, NULL);
static CV_PossibleValue_t vp8_cpu_used_cons_t[] = {{-16, "MIN"}, {16, "MAX"}, {0, NULL}};
consvar_t cv_vp8_cpu_used = CVAR_INIT("vp8_cpu_used", "0", CV_SAVE, vp8_cpu_used_cons_t, NULL);
static CV_PossibleValue_t vp8_cq_level_cons_t[] = {{0, "MIN"}, {63, "MAX"}, {0, NULL}};
consvar_t cv_vp8_cq_level = CVAR_INIT("vp8_cq_level", "10", CV_SAVE, vp8_cq_level_cons_t, NULL);
// static_cast<int>(DeadlineOption::kInfinite) = 0
static CV_PossibleValue_t vp8_deadline_cons_t[] = {{1, "MIN"}, {INT32_MAX, "MAX"}, {0, "infinite"}, {0, NULL}};
consvar_t cv_vp8_deadline = CVAR_INIT("vp8_deadline", "10", CV_SAVE, vp8_deadline_cons_t, NULL);
static CV_PossibleValue_t vp8_sharpness_cons_t[] = {{0, "MIN"}, {7, "MAX"}, {0, NULL}};
consvar_t cv_vp8_sharpness = CVAR_INIT("vp8_sharpness", "7", CV_SAVE, vp8_sharpness_cons_t, NULL);
static CV_PossibleValue_t vp8_token_parts_cons_t[] = {{0, "MIN"}, {3, "MAX"}, {0, NULL}};
consvar_t cv_vp8_token_parts = CVAR_INIT("vp8_token_parts", "0", CV_SAVE, vp8_token_parts_cons_t, NULL);
static CV_PossibleValue_t vp8_threads_cons_t[] = {{1, "MIN"}, {INT32_MAX, "MAX"}, {0, NULL}};
consvar_t cv_vp8_threads = CVAR_INIT("vp8_threads", "1", CV_SAVE, vp8_threads_cons_t, NULL);


std::shared_ptr<AVRecorder> g_av_recorder;

void M_AVRecorder_AddCommands(void)
{
	CV_RegisterVar(&cv_movie_custom_resolution);
	CV_RegisterVar(&cv_movie_duration);
	CV_RegisterVar(&cv_movie_fps);
	CV_RegisterVar(&cv_movie_resolution);
	CV_RegisterVar(&cv_movie_showfps);
	CV_RegisterVar(&cv_movie_sound);

	// srb2::media::Options::register_all();
	// son
	CV_RegisterVar(&cv_vorbis_quality);
	CV_RegisterVar(&cv_vorbis_max_bitrate);
	CV_RegisterVar(&cv_vorbis_nominal_bitrate);
	CV_RegisterVar(&cv_vorbis_min_bitrate);

	CV_RegisterVar(&cv_vp8_quality_mode);
	CV_RegisterVar(&cv_vp8_target_bitrate);
	CV_RegisterVar(&cv_vp8_min_q);
	CV_RegisterVar(&cv_vp8_max_q);
	CV_RegisterVar(&cv_vp8_kf_min);
	CV_RegisterVar(&cv_vp8_kf_max);
	CV_RegisterVar(&cv_vp8_cpu_used);
	CV_RegisterVar(&cv_vp8_cq_level);
	CV_RegisterVar(&cv_vp8_deadline);
	CV_RegisterVar(&cv_vp8_sharpness);
	CV_RegisterVar(&cv_vp8_token_parts);
	CV_RegisterVar(&cv_vp8_threads);
}

static AVRecorder::Config configure()
{
	AVRecorder::Config cfg {};

	if (cv_movie_duration.value > 0)
	{
		cfg.max_duration = std::chrono::duration<float>(FixedToFloat(cv_movie_duration.value));
	}

	if (cv_gif_maxsize.value > 0)
	{
		cfg.max_size = FixedToFloat(cv_gif_maxsize.value * FRACUNIT) * 1024 * 1024;
	}

	if (sound_started && cv_movie_sound.value)
	{
		cfg.audio = {
			.sample_rate = 44100,
		};
	}

	cfg.video = {
		.width = 0, .height = 0,
		.frame_rate = cv_movie_fps.value,
	};

	AVRecorder::Config::Video& v = *cfg.video;

	auto basex = [&v](int scale)
	{
		v.width = vid.width / vid.dup * scale;
		v.height = vid.height / vid.dup * scale;
	};

	switch (cv_movie_resolution.value)
	{
	case Res::kGame:
		v.width = vid.width;
		v.height = vid.height;
		break;

	case Res::kBase:
		basex(1);
		break;

	case Res::kBase2x:
		basex(2);
		break;

	case Res::kBase4x:
		basex(4);
		break;

	case Res::kWindow:
		v.width = vid.width;
		v.height = vid.height;
		break;

	case Res::kCustom:
		if (sscanf(cv_movie_custom_resolution.string, "%dx%d", &v.width, &v.height) != 2)
		{
			// throw std::invalid_argument(fmt::format(
			// 	"Bad movie_custom_resolution '{}', should be <width>x<height> (e.g. 640x400)",
			// 	cv_movie_custom_resolution.string
			// ));
			throw std::invalid_argument(
				"Bad movie_custom_resolution '{}', should be <width>x<height> (e.g. 640x400)");
		}
		break;

	default:
		assert(false);
		break;
	}

	return cfg;
}

boolean M_AVRecorder_Open(const char* filename)
{
	try
	{
		AVRecorder::Config cfg = configure();

		cfg.file_name = filename;

		g_av_recorder = std::make_shared<AVRecorder>(cfg);

		// I_UpdateAudioRecorder();

		return true;
	}
	catch (const std::exception& ex)
	{
		CONS_Alert(CONS_ERROR, "Exception starting video recorder: %s\n", ex.what());
		return false;
	}
}

void M_AVRecorder_Close(void)
{
	g_av_recorder.reset();

	// I_UpdateAudioRecorder();
}

const char* M_AVRecorder_GetFileExtension(void)
{
	return AVRecorder::file_extension();
}

const char* M_AVRecorder_GetCurrentFormat(void)
{
	assert(g_av_recorder != nullptr);

	return g_av_recorder->format_name();
}

void M_AVRecorder_PrintCurrentConfiguration(void)
{
	assert(g_av_recorder != nullptr);

	g_av_recorder->print_configuration();
}

boolean M_AVRecorder_IsExpired(void)
{
	if (g_av_recorder == nullptr) return true;

	return g_av_recorder->invalid();
}

void M_AVRecorder_DrawFrameRate(void)
{
	if (!cv_movie_showfps.value || !g_av_recorder)
	{
		return;
	}

	g_av_recorder->draw_statistics();
}

float M_AVRecorder_GetSize(void)
{
	if (!g_av_recorder)
	{
		return 0.f;
	}

	return g_av_recorder->size();
}

INT32 M_AVRecorder_GetFrames(void)
{
	if (!g_av_recorder)
	{
		return 0;
	}

	return g_av_recorder->frames();
}

// the parameters make me worry about if this will work with other audio backends
// void M_AVRecorder_AudioEncoder(int channel, void *stream, int len, void *udata)
// {
// 	(void)channel;
// 	(void)udata;
// 	if (M_IsRecordingVideo() && VideoEncoder_IsRecordingAudio())
// 		VideoEncoder_RecordAudio((INT16 *)stream, len);
// }


static void M_SaveFrame_AVRecorder(uint32_t width, uint32_t height, tcb::span<const std::byte> data)
{
	if (M_AVRecorder_IsExpired())
	{
		M_StopMovie();
		return;
	}

	auto frame = g_av_recorder->new_staging_video_frame(width, height, false);
	if (!frame)
	{
		// Not time to submit a frame!
		return;
	}

	auto data_begin = reinterpret_cast<const uint8_t*>(data.data());
	auto data_end = reinterpret_cast<const uint8_t*>(data.data() + data.size_bytes());
	std::copy(data_begin, data_end, frame->screen.begin());
	g_av_recorder->push_staging_video_frame(std::move(frame));
}

void M_AVRecorder_SaveSoftwareScreen(void)
{
	// the only way i can reasonably do this (without abusing 15 c++ language features)
	// is by duplicating code..
	if (M_AVRecorder_IsExpired())
	{
		M_StopMovie();
		return;
	}

	auto frame = g_av_recorder->new_staging_video_frame(vid.width, vid.height, true);
	if (!frame)
	{
		// Not time to submit a frame!
		return;
	}

	tcb::span<RGBA_t> pal(&pLocalPalette[max(st_palette, 0) * 256], 256);
	tcb::span<uint8_t> scr(screens[0], vid.width * vid.height);
	std::copy(pal.begin(), pal.end(), frame->palette.begin());
	std::copy(scr.begin(), scr.end(), frame->screen.begin());

	g_av_recorder->push_staging_video_frame(std::move(frame));
}

void M_AVRecorder_SaveOpenGLScreen(void)
{
	UINT8 *linear = HWR_GetScreenshot();
	M_SaveFrame_AVRecorder(vid.width, vid.height, tcb::as_bytes(tcb::span(linear, 3 * vid.width * vid.height)));
	free(linear);
}
