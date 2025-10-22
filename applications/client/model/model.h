#pragma once

#include "cppgl/src/cppgl.h"
#include <glm/gtx/rotate_vector.hpp>


class Model {
public:
    explicit Model(globals::ModelType type,
          glm::vec3 pos = glm::vec3(0),
          glm::vec2 rot = glm::vec2(0),
          glm::vec3 scale = glm::vec3(1));

    /**
     * After changing position and/or orientation of obj3D,
     * call update() before draw().
     * <br>
     * This method updates model and modelNormal.
     */
    virtual void update();

    virtual void updateVisibility();

protected:
    void doUpdateObj();

    void update(glm::vec3 up);

    void load();

public:
    virtual void draw(Shader &shader);

    //
    // getter
    //

    /**
     * @return center of object
     */
    [[nodiscard]] glm::vec3 getPos() const {
        return _pos;
    }

    /**
     * forward
     *
     * @return cross(up, side)
     */
    [[nodiscard]] glm::vec3 getDir() const {
        return _dir;
    }

    /**
     * right side
     *
     * @return cross(dir, up)
     */
    [[nodiscard]] glm::vec3 getSide() const {
        return _side;
    }

    /**
     * @return cross(side, dir)
     */
    [[nodiscard]] glm::vec3 getUp() const {
        return glm::normalize(glm::cross(_side, _dir));
    }

    /**
     * @return absolute rotation (yaw, pitch) in radians
     */
    [[nodiscard]] glm::vec2 getRot() const {
        return {_yaw, _pitch};
    }

    /**
     * @return xyz scale factor of object
     */
    [[nodiscard]] glm::vec3 getScale() const {
        return _scale;
    }

    //
    // setter
    //

    /**
     * absolute move
     */
   
    void setDir(glm::vec3 dir) {
         _dir = dir;
     }
     void setSide(glm::vec3 side) {
        _side = side;
    }
     void setPos(glm::vec3 pos) {
        _pos = pos;
    }

    /**
     * absolute rotation (yaw, pitch) in radians
     */
    void setRot(glm::vec2 rot) {
        _yaw = 0;
        _pitch = 0;

        yaw(rot.x);
        pitch(rot.y);
    }

    void setScale(glm::vec3 scale) {
        _scale = scale;
    }

    //
    // relative move
    //

    void forward(float by) {
        _pos += getDir() * by;
    }

    void backward(float by) {
        forward(-by);
    }

    void rightward(float by) {
        _pos += getSide() * by;
    }

    void leftward(float by) {
        rightward(-by);
    }

    void upward(float by) {
        _pos += getUp() * by;
    }

    void downward(float by) {
        upward(-by);
    }

    //
    // relative rotate
    //

    void yaw(float angle) {
        _yaw += angle;

        _side = glm::normalize(glm::rotate(
                VEC_RIGHT, _yaw, VEC_UP
        ));

        _dir = glm::normalize(glm::rotate(
                VEC_FORWARD, _yaw, VEC_UP
        ));
        _dir = glm::normalize(glm::rotate(
                _dir, _pitch, _side
        ));
    }

    void pitch(float angle) {
        _pitch += angle;

        _pitch = glm::clamp(_pitch, float(-M_PI_2 + 0.01), float(+M_PI_2 - 0.01));

        _dir = glm::normalize(glm::rotate(
                VEC_FORWARD, _yaw, VEC_UP
        ));
        _dir = glm::normalize(glm::rotate(
                _dir, _pitch, _side
        ));
    }


public:
    const globals::ModelType type;
    bool visible = true;
    bool highlighted = false;


    /**
     * xyz scale factor
     */
    glm::vec3 _scale{1};
    glm::vec3 _pos{0, 0, 0};

    glm::mat4 model{1}, modelNormal{1};

private:
    float _yaw = 0, _pitch = 0;
    glm::vec3 _dir = VEC_FORWARD, _side = VEC_RIGHT;

    static std::map<globals::ModelType, std::vector<Mesh>> meshes;

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

    Model(const Model &obj) = delete;

    Model &operator=(const Model &obj) = delete;
};
