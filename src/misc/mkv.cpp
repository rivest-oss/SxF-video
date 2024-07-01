#ifndef __sxf__misc__mkv__cpp__
#define __sxf__misc__mkv__cpp__

#include "mkv.hpp"

extern "C" {
	#include <libavcodec/codec_id.h>
	#include <libavcodec/codec_par.h>
	#include <libavformat/avformat.h>
	#include <libavcodec/avcodec.h>
	#include <libavutil/avutil.h>
	#include <libavutil/pixfmt.h>
};

namespace SxF {
	ErrorOr<mkv_stream_t> mkv_open_file(const char *filepath) {
		mkv_stream_t ret = {
			(uintptr_t)nullptr,
			-1,
		};

		AVFormatContext *fmt_ctx = nullptr;

		if(avformat_open_input(&fmt_ctx, filepath, nullptr, nullptr) < 0)
			return Error { "Couldn't open input file" };

		if(avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
			avformat_close_input(&fmt_ctx);
			return Error {
				"Couldn't find stream info"
			};
		}

		AVStream *stream;
		AVCodecParameters *codec_params;

		for(u16 i = 0; i < fmt_ctx->nb_streams; i++) {
			stream = fmt_ctx->streams[i];
			codec_params = stream->codecpar;

			if(
				(codec_params->codec_type == AVMEDIA_TYPE_VIDEO) &
				(codec_params->codec_id == AV_CODEC_ID_RAWVIDEO) &
				(codec_params->format == AV_PIX_FMT_YUV420P)
			) {
				ret.stream_id = (i32)i;
				break;
			}
		};

		if(ret.stream_id < 0) {
			avformat_close_input(&fmt_ctx);
			return Error {
				"Couldn't found a supported video stream (I420)"
			};
		}

		ret.data_ptr = (uintptr_t)fmt_ctx;

		return ret;
	};

	void mkv_close_file(mkv_stream_t stream) {
		if(stream.data_ptr != (uintptr_t)nullptr) {
			AVFormatContext *fmt_ctx = (AVFormatContext *)stream.data_ptr;

			avformat_close_input(&fmt_ctx);

			stream.data_ptr = (uintptr_t)nullptr;
		}
	};
};

#endif
