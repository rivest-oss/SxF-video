#ifndef __sxf__misc__video__cpp__
#define __sxf__misc__video__cpp__

#include "video.hpp"
#include <new>

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libavutil/error.h>
	#include <libswscale/swscale.h>
	#include <libavutil/imgutils.h>
};

namespace SxF {
	typedef struct _c_vsptr_t {
		AVFormatContext	*fmt_ctx = nullptr;
		AVStream		*str = nullptr;
		AVCodec			*codec = nullptr;
		AVCodecContext	*codec_ctx = nullptr;
		AVFrame			*av_frame = nullptr;
		AVFrame			*c_frame = nullptr;
		SwsContext		*sws_ctx = nullptr;
		u8				*fb = nullptr;
	} _vsptr_t;

	ErrorOr<void> VideoStream::open(const char *filepath) {
		close();

		_vsptr_t *mp;

		try {
			mp = new _vsptr_t;
		} catch(std::bad_alloc &e) {
			return Error { "Couldn't allocate enough memory for a video context" };
		};

		i_ptr = (uintptr_t)mp;

		int e = avformat_open_input(&mp->fmt_ctx, filepath, NULL, NULL);
		if(e < 0) {
			close();
			
			if(av_strerror(e, err_buff, 128) == 0)
				return Error { err_buff };
			
			return Error { "Couldn't open the provided video file path" };
		}

		e = avformat_find_stream_info(mp->fmt_ctx, NULL);
		if(e < 0) {
			close();
			
			if(av_strerror(e, err_buff, 128) == 0)
				return Error { err_buff };
			
			return Error { "Couldn't find a video stream" };
		}

		AVStream *str;
		for(unsigned int i = 0; i < mp->fmt_ctx->nb_streams; i++) {
			str = mp->fmt_ctx->streams[i];

			if(avcodec_find_decoder(str->codecpar->codec_id) == NULL)
				continue;
			
			if(str->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
				mp->str = str;
				break;
			}
		};
		
		if(mp->str == nullptr) {
			close();
			return Error { "Couldn't find a supported video stream" };
		}
		
		mp->codec = avcodec_find_decoder(mp->str->codecpar->codec_id);
		mp->codec_ctx = avcodec_alloc_context3(mp->codec);
		
		if(mp->codec_ctx == nullptr) {
			close();
			return Error {
				"Couldn't allocate enough memory for a video context"
			};
		}
		
		e = avcodec_parameters_to_context(mp->codec_ctx, mp->str->codecpar);
		if(e < 0) {
			close();
			
			if(av_strerror(e, err_buff, 128) == 0)
				return Error { err_buff };
			
			return Error { "Couldn't copy the video stream context" };
		}
		
		e = avcodec_open2(mp->codec_ctx, mp->codec, nullptr);
		if(e < 0) {
			close();
			
			if(av_strerror(e, err_buff, 128) == 0)
				return Error { err_buff };
			
			return Error { "Couldn't open the video context" };
		}
		
		mp->av_frame = av_frame_alloc();
		mp->c_frame = av_frame_alloc();
		
		if((mp->av_frame == nullptr) | (mp->c_frame == nullptr)) {
			close();
			
			if(av_strerror(e, err_buff, 128) == 0)
				return Error { err_buff };
			
			return Error {
				"Couldn't allocate enough memory for a video frame context"
			};
		}
		
		// SWS_FAST_BILINEAR, SWS_BILINEAR, SWS_CUBIC, SWS_X,
		// SWS_POINT, SWS_AREA, SWS_BICUBLIN, SWS_GAUSS, SWS_SINC,
		// SWS_LANCZOS, SWS_SPLINE
		
		mp->sws_ctx = sws_getContext(	mp->codec_ctx->width,
										mp->codec_ctx->height,
										mp->codec_ctx->pix_fmt,
										mp->codec_ctx->width,
										mp->codec_ctx->height,
										AV_PIX_FMT_RGBA,
										SWS_BILINEAR,
										nullptr, nullptr, nullptr);
		
		if(mp->sws_ctx == nullptr) {
			close();
			return Error { "Couldn't create the SWS context" };
		}
		
		u64 fb_len = av_image_get_buffer_size(	AV_PIX_FMT_RGBA,
												mp->codec_ctx->width,
												mp->codec_ctx->height,
												true);
		
		mp->fb = (u8 *)av_malloc(fb_len);
		if(mp->fb == nullptr) {
			close();
			return Error {
				"Couldn't allocate enough memory for the framebuffer"
			};
		}
		
		av_image_fill_arrays(	mp->c_frame->data,
								mp->c_frame->linesize,
								mp->fb,
								AV_PIX_FMT_RGBA,
								mp->codec_ctx->width,
								mp->codec_ctx->height,
								true);
		
		return Error { nullptr };
	};

	void VideoStream::close(void) {
		if(i_ptr == (uintptr_t)nullptr)
			return;
		
		_vsptr_t *mp = (_vsptr_t *)i_ptr;
		
		if(mp->fb != nullptr)
			av_free(mp->fb);
		if(mp->sws_ctx != nullptr)
			sws_freeContext(mp->sws_ctx);
		if(mp->c_frame != nullptr)
			av_frame_free(&mp->c_frame);
		if(mp->av_frame != nullptr)
			av_frame_free(&mp->av_frame);
		if(mp->codec_ctx != nullptr)
			avcodec_free_context(&mp->codec_ctx);
		if(mp->fmt_ctx != nullptr)
			avformat_close_input(&mp->fmt_ctx);
		
		delete mp;
		i_ptr = (uintptr_t)nullptr;
	};

	ErrorOr<video_frame_t> VideoStream::get_frame(void) {
		// [TODO]
	};
};

#endif
