#ifndef __sxf__misc__video__hpp__
#define __sxf__misc__video__hpp__

#include "../nuclei.hpp"
#include "../io/stream.hpp"

namespace SxF {
	typedef struct {
		u32	width;
		u32	height;
		f32	time;
		u8	*data;
	} video_frame_t;

	class VideoStream {
		private:
			uintptr_t i_ptr = (uintptr_t)NULL;
			char err_buff[128];

		public:
			ErrorOr<void> open(const char *filepath);
			void close(void);

			ErrorOr<video_frame_t> get_frame(void);
	};
};

#endif
