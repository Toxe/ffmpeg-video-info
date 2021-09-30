#include <cstdlib>
#include <filesystem>
#include <memory>
#include <string_view>

#include <fmt/core.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

void die(const char* error_text)
{
    fmt::print("error: {}\n", error_text);
    std::exit(2);
}

std::string_view eval_args(int argc, char* argv[])
{
    if (argc < 2)
        die("missing filename");

    if (!std::filesystem::exists(argv[1]))
        die("file not found");

    return argv[1];
}

int main(int argc, char* argv[])
{
    std::string_view filename = eval_args(argc, argv);

    std::unique_ptr<AVFormatContext, void (*)(AVFormatContext*)> format_context(avformat_alloc_context(), [](AVFormatContext* fmt_ctx) { avformat_close_input(&fmt_ctx); });

    if (!format_context)
        die("avformat_alloc_context");

    auto p1 = format_context.get();

    if (avformat_open_input(&p1, filename.data(), nullptr, nullptr) < 0)
        die("avformat_open_input");

    fmt::print("filename: {}\n", format_context->url);
    fmt::print("format: {}\n", format_context->iformat->long_name);
    fmt::print("duration: {}\n", format_context->duration);

    if (avformat_find_stream_info(format_context.get(), nullptr) < 0)
        die("avformat_find_stream_info");

    fmt::print("number of streams: {}\n", format_context->nb_streams);
    fmt::print("bit rate: {}\n", format_context->bit_rate);

    for (unsigned int i = 0; i < format_context->nb_streams; ++i) {
        auto codec_params = format_context->streams[i]->codecpar;
        auto codec = avcodec_find_decoder(codec_params->codec_id);

        fmt::print("stream #{} ({}):\n", i, av_get_media_type_string(codec_params->codec_type));
        fmt::print("    codec: {}\n", codec->long_name);
        fmt::print("    bit rate: {}\n", codec_params->bit_rate);

        if (codec_params->codec_type == AVMEDIA_TYPE_VIDEO)
            fmt::print("    video: {}x{}\n", codec_params->width, codec_params->height);
        else if (codec_params->codec_type == AVMEDIA_TYPE_AUDIO)
            fmt::print("    audio: {} channels, {} sample rate\n", codec_params->channels, codec_params->sample_rate);
    }
}
