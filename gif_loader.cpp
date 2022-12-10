
#include "gif_loader.h"

#ifdef BIG_ENDIAN_ENABLED
#define GIF_BIGE 1
#endif

#include "./thirdparty/gif_load/gif_load.h"
#include "core/os/file_access.h"
#include "scene/resources/texture.h"

Array GIFLoader::get_images() {
	return _images;
}

Ref<AnimatedTexture> GIFLoader::create_texture(float fps) {
	Ref<AnimatedTexture> tex;
	tex.instance();
	tex->set_fps(0);
	tex->set_flags(0);
	
	float per_frame_time = 1.0 / fps;

	for (int i = 0; i < _images.size(); ++i) {
		Array arr = _images[i];

		ERR_CONTINUE(arr.size() != 2);

		float frames = arr[0];
		Ref<Image> img = arr[1];

		Ref<ImageTexture> imgt;
		imgt.instance();
		imgt->create_from_image(img, 0);

		tex->set_frame_texture(i, imgt);
		tex->set_frame_delay(i, per_frame_time * frames);
	}

	tex->set_frames(_images.size());

	return tex;
}

void GIFLoader::gif_frame(void *data, struct GIF_WHDR *whdr) {
	uint32_t *pict, *prev, x, y, yoff, iter, ifin, dsrc, ddst;
	//uint8_t head[18] = { 0 };
	GIFLoader *loader = (GIFLoader *)data;

#define RGBA(i) ((whdr->bptr[i] == whdr->tran) ? 0 : ((uint32_t)(whdr->cpal[whdr->bptr[i]].R << ((GIF_BIGE) ? 24 : 0)) | (uint32_t)(whdr->cpal[whdr->bptr[i]].G << ((GIF_BIGE) ? 16 : 8)) | (uint32_t)(whdr->cpal[whdr->bptr[i]].B << ((GIF_BIGE) ? 8 : 16)) | ((GIF_BIGE) ? 0xFF : 0xFF000000)))

	if (!whdr->ifrm) {
		//first frame, alloc
		ddst = (uint32_t)(whdr->xdim * whdr->ydim);
		loader->pictd.resize(ddst * sizeof(uint32_t));
		loader->prevd.resize(ddst * sizeof(uint32_t));
	}

	PoolByteArray::Write pictw = loader->pictd.write();

	/** [TODO:] the frame is assumed to be inside global bounds,
				however it might exceed them in some GIFs; fix me. **/
	pict = (uint32_t *)pictw.ptr();
	ddst = (uint32_t)(whdr->xdim * whdr->fryo + whdr->frxo);
	ifin = (!(iter = (whdr->intr) ? 0 : 4)) ? 4 : 5; /** interlacing support **/
	for (dsrc = (uint32_t)-1; iter < ifin; iter++) {
		for (yoff = 16U >> ((iter > 1) ? iter : 1), y = (8 >> iter) & 7; y < (uint32_t)whdr->fryd; y += yoff) {
			for (x = 0; x < (uint32_t)whdr->frxd; x++) {
				if (whdr->tran != (long)whdr->bptr[++dsrc]) {
					pict[(uint32_t)whdr->xdim * y + x + ddst] = RGBA(dsrc);
				}
			}
		}
	}

	pictw.release();

	Array image_arr;
	image_arr.push_back(whdr->time);

	Ref<Image> img;
	img.instance();
	img->create(whdr->xdim, whdr->ydim, false, Image::FORMAT_RGBA8, loader->pictd);
	image_arr.push_back(img);

	loader->_images.push_back(image_arr);

	pict = (uint32_t *)pictw.ptr();

	pictw = loader->pictd.write();
	PoolByteArray::Write prevdw = loader->prevd.write();

	if ((whdr->mode == GIF_PREV) && !loader->last) {
		whdr->frxd = whdr->xdim;
		whdr->fryd = whdr->ydim;
		whdr->mode = GIF_BKGD;
		ddst = 0;
	} else {
		loader->last = (whdr->mode == GIF_PREV) ? loader->last : (unsigned long)(whdr->ifrm + 1);
		pict = (uint32_t *)((whdr->mode == GIF_PREV) ? pictw.ptr() : prevdw.ptr());
		prev = (uint32_t *)((whdr->mode == GIF_PREV) ? prevdw.ptr() : pictw.ptr());

		for (x = (uint32_t)(whdr->xdim * whdr->ydim); --x; pict[x - 1] = prev[x - 1]) {
		}
	}

	if (whdr->mode == GIF_BKGD) { /** cutting a hole for the next frame **/
		for (whdr->bptr[0] = (uint8_t)((whdr->tran >= 0) ? whdr->tran : whdr->bkgd), y = 0, pict = (uint32_t *)pictw.ptr(); y < (uint32_t)whdr->fryd; y++) {
			for (x = 0; x < (uint32_t)whdr->frxd; x++) {
				pict[(uint32_t)whdr->xdim * y + x + ddst] = RGBA(0);
			}
		}
	}

#undef RGBA
}

void GIFLoader::load_gif(const String &file) {
	_images.clear();

	FileAccess *f = FileAccess::open(file, FileAccess::READ);
	ERR_FAIL_COND(!f);

	//gif_load seems to need +2 bytes at the end
	data.resize(f->get_len() + 2);
	f->get_buffer(data.ptrw(), data.size() - 2);
	data.set(data.size() - 2, 0);
	data.set(data.size() - 1, 0);

	memdelete(f);

	GIF_Load((void *)data.ptrw(), (long)data.size(), gif_frame, 0, (void *)this, 0L);

	data.clear();
}

GIFLoader::GIFLoader() {
	size = 0;
	last = 0;
}
GIFLoader::~GIFLoader() {
}

void GIFLoader::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_images"), &GIFLoader::get_images);
	ClassDB::bind_method(D_METHOD("create_texture", "fps"), &GIFLoader::create_texture, 60);
	ClassDB::bind_method(D_METHOD("load_gif", "file"), &GIFLoader::load_gif);
}