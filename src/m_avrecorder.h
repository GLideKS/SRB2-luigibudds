// RING RACERS
//-----------------------------------------------------------------------------
// Copyright (C) 2023 by James Robert Roman
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------

#ifndef M_AVRECORDER_H
#define M_AVRECORDER_H

#include "blua/luaconf.h"
#include "command.h"

#ifdef __cplusplus
extern "C" {
#endif

void M_AVRecorder_AddCommands(void);

const char *M_AVRecorder_GetFileExtension(void);

// True if successully opened.
boolean M_AVRecorder_Open(const char *filename);

void M_AVRecorder_Close(void);

// Check whether AVRecorder is still valid. Call M_AVRecorder_Close if expired.
boolean M_AVRecorder_IsExpired(void);

const char *M_AVRecorder_GetCurrentFormat(void);

void M_AVRecorder_PrintCurrentConfiguration(void);

void M_AVRecorder_DrawFrameRate(void);

float M_AVRecorder_GetSize(void);

INT32 M_AVRecorder_GetFrames(void);

void M_AVRecorder_AudioEncoder(void);

void M_AVRecorder_SaveSoftwareScreen(void);

void M_AVRecorder_SaveOpenGLScreen(void);

extern consvar_t
	cv_movie_custom_resolution,
	cv_movie_duration,
	cv_movie_fps,
	cv_movie_resolution,
	cv_movie_showfps,
	cv_movie_size,
	cv_movie_sound;

// romoney5: encoder options
extern consvar_t
	cv_vorbis_quality,
	cv_vorbis_max_bitrate,
	cv_vorbis_nominal_bitrate,
	cv_vorbis_min_bitrate,

	cv_vp8_quality_mode,
	cv_vp8_target_bitrate,
	cv_vp8_min_q,
	cv_vp8_max_q,
	cv_vp8_kf_min,
	cv_vp8_kf_max,
	cv_vp8_cpu_used,
	cv_vp8_cq_level,
	cv_vp8_deadline,
	cv_vp8_sharpness,
	cv_vp8_token_parts,
	cv_vp8_threads;

#ifdef __cplusplus
}; // extern "C"
#endif

#endif/*M_AVRECORDER_H*/
