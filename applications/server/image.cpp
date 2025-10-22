#include "image.h"
#include <stdexcept>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

using namespace std;
namespace fs = std::filesystem;

Image::Image(const fs::path &imageFile)
        : width(-1), height(-1) {
}

unsigned short Image::getPixelClamped(int x, int y) const {
    // clamp to edge of image area
    {
        x = max(0, x);
        y = max(0, y);

        x = min(width - 1, x);
        y = min(height - 1, y);
    }
    return doGetPixel(x, y);
}

PngImage::PngImage(const fs::path &imageFile) : Image(imageFile) {
    stbi_set_flip_vertically_on_load(0);

    // The pixel data consists of `height` scanlines of `width` pixels
    data = stbi_load(imageFile.c_str(), &width, &height, &n, 0);
    if (!data) {
        throw std::runtime_error("Failed to open image " + imageFile.string() + ". Reason: " + stbi_failure_reason());
    }
}

PngImage::~PngImage() {
    cout << "~PngImage()" << endl;
    stbi_image_free(data);
}

unsigned short PngImage::doGetPixel(const int x, const int y) const {
    int idx = n * (y * width + x);
    return *(data + idx); // in range [0, 255]
}
