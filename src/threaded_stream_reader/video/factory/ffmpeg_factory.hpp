#pragma once

#include "factory.hpp"

class FFmpegFactory : public Factory {
public:
    [[nodiscard]] std::unique_ptr<CodecContext> create_codec_context(AVStream* stream) override;
    [[nodiscard]] std::unique_ptr<VideoFrame> create_video_frame(CodecContext* codec_context, const int width, const int height) override;
};
