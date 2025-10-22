
#include<cppgl.h>
#include"sky.h"
#include"stb_image.h"
#include <stb_image.h>
#include "map/map.h"
#include "model/player.h"
#include "../common/networking/daytime.h"

extern std::shared_ptr<Map> the_map;
extern std::shared_ptr<Player> the_player;
extern std::shared_ptr<Sun> the_sun;

std::vector<Drawelement> Sky::prototype;
std::vector<Drawelement> Sun::sun_prototype;

glm::ivec3 extend;
//glm::ivec3 extend = glm::ivec3(1,1,1);

Sky::Sky(float scale) : model(1)
{
   if(prototype.empty()){
      auto shader = Shader("sky-shader", "shader/sky.vs", "shader/sky.fs");
      auto meshes = load_meshes_gpu((globals::renderData()/ "Cubes" / "sky" / "sky.obj").string().c_str(), true);
      for(auto m: meshes){
         Sky::prototype.emplace_back(m->name, shader, m);
      }
   }
   model[0][0] = scale;
   model[1][1] = scale;
   model[2][2] = scale;

//   model = glm::rotate(model, (float)M_PI_2, glm::vec3(0,0,1));
}

void Sky::setup_skybox(){

    glGenTextures(1, &day);
    glBindTexture(GL_TEXTURE_CUBE_MAP, day);


    int width, height, nrChannels;
    unsigned char* data;

    for(unsigned int i = 0; i < 6; i++){

       switch(i){
          case 0: data = stbi_load("render-data/Cubes/sky/backk.png", &width, &height, &nrChannels, 0);break;
          case 1: data = stbi_load("render-data/Cubes/sky/frontt.png", &width, &height, &nrChannels, 0);break;
          case 2: data = stbi_load("render-data/Cubes/sky/topp.png", &width, &height, &nrChannels, 0);break;
          case 3: data = stbi_load("render-data/Cubes/sky/bottomm.png", &width, &height, &nrChannels, 0);break;
          case 4: data = stbi_load("render-data/Cubes/sky/rightt.png", &width, &height, &nrChannels, 0);break;
          case 5: data = stbi_load("render-data/Cubes/sky/leftt.png", &width, &height, &nrChannels, 0);break;
       }
   

        
       if(data){
          glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
          stbi_image_free(data);
       }else{
          std::cout <<"failed to load cubemap" << std::endl;
           stbi_image_free(data);

       }

    }
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);    
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);   
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);   
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

     glGenTextures(1, &night);
    glBindTexture(GL_TEXTURE_CUBE_MAP, night);



    for(unsigned int i = 0; i < 6; i++){

       switch(i){
          case 0: data = stbi_load("render-data/Cubes/sky/night/back.png", &width, &height, &nrChannels, 0);break;
          case 1: data = stbi_load("render-data/Cubes/sky/night/front.png", &width, &height, &nrChannels, 0);break;
          case 2: data = stbi_load("render-data/Cubes/sky/night/top.png", &width, &height, &nrChannels, 0);break;
          case 3: data = stbi_load("render-data/Cubes/sky/night/bottom.png", &width, &height, &nrChannels, 0);break;
          case 4: data = stbi_load("render-data/Cubes/sky/night/right.png", &width, &height, &nrChannels, 0);break;
          case 5: data = stbi_load("render-data/Cubes/sky/night/left.png", &width, &height, &nrChannels, 0);break;
       }
   

        
       if(data){
          glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
          stbi_image_free(data);
       }else{
          std::cout <<"failed to load cubemap" << std::endl;
           stbi_image_free(data);

       }

    }
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);    
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);   
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);   
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void Sky::draw(){
    auto ms = (float) the_player->gameTimer.look();
    for(auto& elem : prototype){
      elem->model = model;
      elem->bind();
      //setup light
      float day_frac = daytime::dayFract(ms);
      elem->shader->uniform("time", day_frac);
      elem->shader->uniform("day", day, 9);
      elem->shader->uniform("night", night, 10);
      elem->shader->uniform("sun", false);
      elem->shader->uniform("sun_pos", the_sun->get_sun_pos());
      elem->draw();
      glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
      elem->unbind();
   }
}

void Sky::update() {
    const auto pos = the_player->getPos();
    model[3][0] = pos.x;
    model[3][1] = pos.y;
    model[3][2] = pos.z;
}

Sun::Sun() : model(1)
{
   extend = glm::ivec3(the_map->extent.x,1,the_map->extent.z);//glm::ivec3(the_map->extent.x, the_map->extent.y,the_map->extent.z);
   if(sun_prototype.empty()){
   
      auto shader = Shader("sun-shader", "shader/sky.vs", "shader/sky.fs");
      auto meshes = load_meshes_gpu((globals::renderData()/ "Cubes" / "sky" / "sun.obj").string().c_str(), true);
      for(auto m: meshes){
         Sun::sun_prototype.emplace_back(m->name, shader, m);
      }
   }


   glm::vec3 spawnpos = glm::vec3(-40, 0,(extend.z / 2));
   int scale = 4;
   model[0][0] = scale;
   model[1][1] = scale;
   model[2][2] = scale;

   model[3][0] = -40;
   model[3][1] = 0 ;
   model[3][2] = extend.z/2;

   
   midpoint = glm::vec2(extend.x/2, 0);

 

}
void Sun::draw(){

   for(auto& elem : sun_prototype){
      elem->model = model;
      elem->bind();
      //setup light
      float ms = the_player->gameTimer.look();
      float day_frac = daytime::dayFract(ms);
      elem->shader->uniform("time", day_frac);
      elem->shader->uniform("sun", true);
       elem->shader->uniform("sun_pos", glm::vec3(model[3][0],model[3][1],model[3][2]));
      elem->draw();
      elem->unbind();
   }

}

void Sun::update(){
      float r = float(extend.x/2 + 25);
      float ms = the_player->gameTimer.look();
      float day_frac = daytime::dayFract(ms);

      float a = (float)(2.f * M_PI * day_frac) + (float) (1.5 * M_PI);
  

   model[3][0] = float(extend.x/2 + r * cos(a));
   model[3][1] = 0 + r * sin(a);

   dir = glm::normalize(glm::vec3(float(the_map->extent.x) , 0.f, float(the_map->extent.z) * 0.5f)  - get_sun_pos());
   ambient = glm::vec3(daytime::sunAmbient(day_frac));
  
}

  glm::vec3 Sun::get_sun_pos(){return glm::vec3(model[3][0],model[3][1],model[3][2]);}
