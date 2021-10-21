#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <stop_token>
#include <thread>
#include <tuple>

extern "C" {
#include <libavcodec/packet.h>
#include <libavformat/avformat.h>
}

#include "auto_delete_ressource.hpp"
#include "video_frame_scaler.hpp"
#include "video_frames_queue.hpp"

struct VideoFrame;

class VideoContentProvider {
    std::mutex mtx_reader_;
    std::mutex mtx_scaler_;
    std::condition_variable_any cv_reader_;
    std::condition_variable_any cv_scaler_;

    std::jthread reader_thread_;
    std::jthread scaler_thread_;

    AVFormatContext* format_context_ = nullptr;
    AVCodecContext* video_codec_context_ = nullptr;
    AVCodecContext* audio_codec_context_ = nullptr;

    auto_delete_ressource<AVPacket> packet_ = {nullptr, nullptr};

    int video_stream_index_ = -1;
    int audio_stream_index_ = -1;

    int scale_width_ = 0;
    int scale_height_ = 0;

    bool is_ready_ = false;

    VideoFramesQueue finished_video_frames_queue_;
    VideoFrameScaler video_frame_scaler_;

    void reader_main(std::stop_token st);
    void scaler_main(std::stop_token st);

    [[nodiscard]] std::optional<VideoFrame*> read();
    [[nodiscard]] VideoFrame* decode_video_packet(const AVPacket* packet, const int scale_width, const int scale_height);

    void add_unscaled_video_frame(VideoFrame* video_frame);
    void add_finished_video_frame(VideoFrame* video_frame);

public:
    VideoContentProvider(AVFormatContext* format_context, AVCodecContext* video_codec_context, AVCodecContext* audio_codec_context, int video_stream_index, int audio_stream_index, const int scale_width, const int scale_height);
    ~VideoContentProvider();

    void run();
    void stop();

    [[nodiscard]] std::tuple<VideoFrame*, int, bool> next_frame(const double playback_position);
};
