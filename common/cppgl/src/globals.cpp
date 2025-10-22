#include "globals.h"

using namespace std;
namespace fs = std::filesystem;

fs::path globals::renderData() {
    return "render-data";
}
float globals::HIDPI_SCALE = 2.f;


// TODO uncomment to debug
//#define PROCESS_VAL(p) case(p): s = #p; break;
//
//std::ostream& operator<<(std::ostream& out, const globals::AnimationType value){
//    using namespace globals;
//    const char* s = nullptr;
//    switch(value){
//        PROCESS_VAL(AnimationType::WALKING)
//        PROCESS_VAL(AnimationType::STANDING)
//        PROCESS_VAL(AnimationType::DIGGING)
//    }
//    return out << s;
//}
//
//std::ostream& operator<<(std::ostream& out, const globals::ModelType value){
//    using namespace globals;
//    const char* s = nullptr;
//    switch(value){
//        PROCESS_VAL(ModelType::air)
//        PROCESS_VAL(ModelType::water)
//        PROCESS_VAL(ModelType::wood)
//        PROCESS_VAL(ModelType::dirt)
//        PROCESS_VAL(ModelType::grass)
//        PROCESS_VAL(ModelType::leaf)
//        PROCESS_VAL(ModelType::stone)
//        PROCESS_VAL(ModelType::torch)
//        PROCESS_VAL(ModelType::cloud0)
//        PROCESS_VAL(ModelType::cloud1)
//        PROCESS_VAL(ModelType::cloud2)
//        PROCESS_VAL(ModelType::cloud3)
//        PROCESS_VAL(ModelType::player)
//        PROCESS_VAL(ModelType::dog)
//        PROCESS_VAL(ModelType::horse)
//        PROCESS_VAL(ModelType::zombie)
//        PROCESS_VAL(ModelType::skeleton)
//    }
//    return out << s;
//}
//
//#undef PROCESS_VAL
