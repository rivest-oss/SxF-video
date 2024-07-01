#ifndef __sxf__misc__mkv__hpp__
#define __sxf__misc__mkv__hpp__

#include "../nuclei.hpp"
#include "../io/stream.hpp"

namespace SxF {
	typedef struct {
		uintptr_t	data_ptr;
		i32			stream_id;
	} mkv_stream_t;

	ErrorOr<mkv_stream_t> mkv_open_file(const char *filepath);
	void mkv_close_file(mkv_stream_t stream);
};

#endif
