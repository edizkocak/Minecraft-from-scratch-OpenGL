#include "screen_model_test.h"
#include "model.h"

extern float rx;
extern float ry;
extern float rz;

extern float xa;
extern float ya;
extern float za;



ScreenModelTest::ScreenModelTest(globals::ModelType type,
                         glm::vec3 pos,
                         glm::vec2 rot,
                         glm::vec3 scale)
        : Base(type, pos, rot, scale)
{
}


void ScreenModelTest::draw(Shader& shader){
    Base::draw(shader);
}

void ScreenModelTest::update(){
    model = glm::mat4(1);

    // rotate

    //yaw
    model = glm::rotate(model, rx, VEC_UP);

    //pitch
    model = glm::rotate(model, ry, VEC_RIGHT);

    //roll
    model = glm::rotate(model, rz, VEC_FORWARD);



    // set pos
    model[3][0] = xa;
    model[3][1] = ya;
    model[3][2] = za;

    modelNormal = glm::transpose(glm::inverse(model));



    //model = glm::rotate(model, rx, VEC_UP);
    //Base::update();
}