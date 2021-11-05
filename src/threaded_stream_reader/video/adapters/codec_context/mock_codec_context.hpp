#pragma once

#include "codec_context.hpp"

class MockCodecContext : public CodecContext {
    int next_frame_number_ = 0;

public:
    [[nodiscard]] int width() override;
    [[nodiscard]] int height() override;
    [[nodiscard]] AVPixelFormat pixel_format() override;

    [[nodiscard]] virtual int send_packet(Packet* packet) override;
    [[nodiscard]] virtual std::unique_ptr<Frame> receive_frame(Factory* factory, const double time_base, const int scaled_width, const int scaled_height) override;
};
