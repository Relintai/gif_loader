
#ifndef GIF_LOADER_H
#define GIF_LOADER_H

#include "core/object/reference.h"

#include "core/io/image.h"

struct GIF_WHDR;
class AnimatedTexture;

class GIFLoader : public Reference {
	GDCLASS(GIFLoader, Reference);

public:
	Array get_images();
    Ref<AnimatedTexture> create_texture(float fps = 60);

	void load_gif(const String &file);

	GIFLoader();
	~GIFLoader();

private:
	static void _bind_methods();

	static void gif_frame(void *data, struct GIF_WHDR *whdr);

	Array _images;

	Vector<uint8_t> data;
	PoolByteArray pictd;
    PoolByteArray prevd;

	unsigned long size;
	unsigned long last;
};

#endif
