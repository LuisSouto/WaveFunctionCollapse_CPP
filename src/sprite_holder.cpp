#include <sprite_holder.h>

/* Create a unique hash for each pixel in the image.*/
// WARNING: this assumes RBG format, it will not work otherwise
void SpriteHolder::computePixelHashes() {
	pixel_hashes.resize(num_pixels);
	for (size_t i = 0; i < num_pixels; i++) {
		pixel_hash_t pixel_hash = 0;
		for (size_t c = 0; c < channels; c++) {
			pixel_hash += image_pixels[i * channels + c] << (c * 8);
		}
		pixel_hashes[i] = pixel_hash;
	}
}