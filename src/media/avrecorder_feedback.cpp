// RING RACERS
//-----------------------------------------------------------------------------
// Copyright (C) 2023 by James Robert Roman
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------

#include <cassert>
#include <filesystem>
#include <sstream>
#include <string>

// #include <fmt/format.h>

#include "avrecorder_impl.hpp"

extern "C" {
#include "../v_video.h"
}

using namespace srb2::media;

using Impl = AVRecorder::Impl;

namespace
{

constexpr float kMb = 1024.f * 1024.f;

}; // namespace

void Impl::container_dtor_handler(const MediaContainer& container) const
{
	// Note that because this method is called from
	// container_'s destructor, any member variables declared
	// after Impl::container_ should not be accessed by now
	// (since they would have already destructed).

	if (max_size_ && container.size() > *max_size_)
	{
		// const std::string line = fmt::format(
		// 	"Video size has exceeded limit {} > {} ({}%)."
		// 	" This should not happen, please report this bug.\n",
		// 	container.size(),
		// 	*max_size_,
		// 	100.f * (*max_size_ / static_cast<float>(container.size()))
		// );

		CONS_Alert(CONS_WARNING, "Video size has exceeded limit %lu > %lu (%f%%)."
			" This should not happen, please report this bug.\n",
			container.size(),
			*max_size_,
			100.f * (*max_size_ / static_cast<float>(container.size())));
	}

	// std::ostringstream msg;

	// msg << "Video saved: " << std::filesystem::path(container.file_name()).filename().string()
	// 	<< fmt::format(" ({:.2f}", container.size() / kMb);

	// if (max_size_)
	// {
	// 	msg << fmt::format("/{:.2f}", *max_size_ / kMb);
	// }

	// msg << fmt::format(" MB, {:.1f}", container.duration().count());

	// if (max_duration_config_)
	// {
	// 	msg << fmt::format("/{:.1f}", max_duration_config_->count());
	// }

	// msg << " seconds)";

	CONS_Printf("Video saved: %s%s (%.2f/%.2f MB, %.1f/%.1f seconds)\n",
		"movies/", std::filesystem::path(container.file_name()).filename().string().c_str(),
		container.size() / kMb, *max_size_ / kMb,
		container.duration().count(), max_duration_config_->count());
}

void AVRecorder::print_configuration() const
{
	if (impl_->audio_encoder_)
	{
		const auto& a = *impl_->audio_encoder_;

		CONS_Printf("Audio: %s %dch %d Hz\n", a.name(), a.channels(), a.sample_rate());
	}

	if (impl_->video_encoder_)
	{
		const auto& v = *impl_->video_encoder_;

		CONS_Printf(
			"Video: %s %dx%d %d fps %d threads\n",
			v.name(),
			v.width(),
			v.height(),
			v.frame_rate(),
			v.thread_count()
		);
	}
}

void AVRecorder::draw_statistics() const
{
	assert(impl_->video_encoder_ != nullptr);

	auto draw = [](int x, std::string text, int32_t flags = 0)
	{
		V_DrawThinString(
			x,
			190,
			(V_6WIDTHSPACE | V_ALLOWLOWERCASE | V_SNAPTOBOTTOM | V_SNAPTORIGHT) | flags,
			text.c_str()
		);
	};

	const float fps = impl_->video_frame_rate_avg_;
	const float size = impl_->container_->size();

	const int32_t fps_color = [&]
	{
		const int cap = impl_->video_encoder_->frame_rate();

		// red when dropped below 60% of the target
		if (fps > 0.f && fps < (0.6f * cap))
		{
			return V_REDMAP;
		}

		return 0;
	}();

	const int32_t mb_color = [&]
	{
		if (!impl_->max_size_)
		{
			return 0;
		}

		const std::size_t cap = *impl_->max_size_;

		// yellow when within 1 MB of the limit
		if (size >= (cap - kMb))
		{
			return V_YELLOWMAP;
		}

		return 0;
	}();

	V_DrawThinString(
		200, 190, (V_6WIDTHSPACE | V_ALLOWLOWERCASE | V_SNAPTOBOTTOM | V_SNAPTORIGHT) | fps_color,
		va("%.0f", fps));
	V_DrawThinString(
		230, 190, (V_6WIDTHSPACE | V_ALLOWLOWERCASE | V_SNAPTOBOTTOM | V_SNAPTORIGHT),
		va("%.1fs", impl_->container_->duration().count()));
	V_DrawThinString(
		260, 190, (V_6WIDTHSPACE | V_ALLOWLOWERCASE | V_SNAPTOBOTTOM | V_SNAPTORIGHT) | mb_color,
		va("%.1f MB", size / kMb));
	// draw(200, fmt::format("{:.0f}", fps), fps_color);
	// draw(230, fmt::format("{:.1f}s", impl_->container_->duration().count()));
	// draw(260, fmt::format("{:.1f} MB", size / kMb), mb_color);
}
