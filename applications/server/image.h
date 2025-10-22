#ifndef GRAPRA_IMAGE_H
#define GRAPRA_IMAGE_H

#include <filesystem>

class Image {
public:
    explicit Image(const std::filesystem::path &imageFile);

    ~Image() = default;

public:
    int getWidth() const {
        return width;
    }

    int getHeight() const {
        return height;
    }

    unsigned short getPixel(const int x, const int y) const {
        return doGetPixel(x, y);
    }

    /**
     * See OpenGL "GL_CLAMP_TO_EDGE". <br>
     * https://learnopengl.com/Getting-started/Textures
     */
    unsigned short getPixelClamped(int x, int y) const;

    virtual unsigned short doGetPixel(const int x, const int y) const = 0;

protected:
    int width, height;
};

class PngImage : public Image {
public:
    explicit PngImage(const std::filesystem::path &imageFile);

    ~PngImage();

public:
    unsigned short doGetPixel(const int x, const int y) const override;

private:
    /**
     * Number of 8-bit components per pixel. <br>
     * Number of image components in image file.
     */
    int n;
    uint8_t *data;

public:
    /*
     * In OO programming it is typical to prohibit copying of objects. <br>
     * <br>
     * Jede Klasse, deren Klassendaten Zeiger verwenden, sollte selbst-
     * erstellte Kopierkonstruktoren (copy constructor) und Zuweisungs-Operatoren (assignment operator) enthal-
     * ten. Ansonsten sollte man sie unwirksam machen [...]. <br>
     * <br>
     * ISBN 3-8348-0125-9, May 2006, Grundkurs Software Entwicklung mit C++, 2. Auflage
     * <br>
     * boost::non_copyable or delete copy constructor and assignment operator? https://stackoverflow.com/a/7841332/6334421
     */

    PngImage(const PngImage &obj) = delete;

    PngImage &operator=(const PngImage &obj) = delete;
};

#endif //GRAPRA_IMAGE_H
