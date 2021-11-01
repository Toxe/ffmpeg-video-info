#include "video_content_provider.hpp"

#include <latch>

#include <fmt/core.h>

#include "error/error.hpp"
#include "factory/factory.hpp"
#include "logger/logger.hpp"
#include "video_frame/video_frame.hpp"

VideoContentProvider::VideoContentProvider(Factory* factory, VideoFile& video_file, const int scale_width, const int scale_height)
    : factory_{factory},
      video_frame_scaler_{factory, video_file.video_stream_info(), scale_width, scale_height},
      video_reader_{factory, video_file.audio_stream_info(), video_file.video_stream_info(), scale_width, scale_height}
{
}

VideoContentProvider::~VideoContentProvider()
{
    stop();
}

void VideoContentProvider::run()
{
    if (!is_running_) {
        log_debug("(VideoContentProvider) run");

        std::latch latch{3};

        video_frame_scaler_.run(this, latch);
        video_reader_.run(this, latch);

        latch.arrive_and_wait();

        is_running_ = true;
    }
}

void VideoContentProvider::stop()
{
    if (is_running_) {
        log_debug("(VideoContentProvider) stop");

        video_reader_.stop();
        video_frame_scaler_.stop();

        is_running_ = false;
    }
}

bool VideoContentProvider::finished_video_frames_queue_is_full()
{
    return finished_video_frames_queue_.full();
}

void VideoContentProvider::add_video_frame_for_scaling(std::unique_ptr<VideoFrame> video_frame)
{
    video_frame_scaler_.add_to_queue(std::move(video_frame));
}

void VideoContentProvider::add_finished_video_frame(std::unique_ptr<VideoFrame> video_frame)
{
    log_trace(fmt::format("(VideoContentProvider) new video frame: {} ({} frames now available)", video_frame->print(), finished_video_frames_queue_.size() + 1));

    finished_video_frames_queue_.push(std::move(video_frame));
}

std::tuple<std::unique_ptr<VideoFrame>, int> VideoContentProvider::next_frame(const double playback_position)
{
    std::unique_ptr<VideoFrame> video_frame = finished_video_frames_queue_.pop(playback_position);

    if (video_frame && !finished_video_frames_queue_.full())
        video_reader_.continue_reading();

    return std::make_tuple(std::move(video_frame), static_cast<int>(finished_video_frames_queue_.size()));
}
