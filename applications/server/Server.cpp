#include "Server.h"
#include "Message.h"
#include "network.h"
#include "sv_msg.h"
#include "SvMovable.h"
#include "WeatherType.h"
#include "MathUtil.h"
#include "AStar.h"
#include "daytime.h"

using namespace std;
using namespace boost::asio;
using ip::tcp;

Server::Server(unsigned int port, const std::string& address, int numPlayers)
        : map(SvMap::newHeightmap()),
        acceptor(ioService,
                 tcp::endpoint(tcp::v4(), port)),
        sockets(numPlayers),
        connected(numPlayers, true) {
    cout << "Starting server at " << address << ":" << port << endl;

    AStar::init(&map);
    SvMovable::init(&map);
    SvPlayer::numPlayers = numPlayers;

    // id=0 is reserved for textures
    minId = 1;
    maxId = (1) * (globals::maxMovableObjId / numPlayers + 1) - 1;
    nextId = 1;

    // init clouds
    {
        const std::map<WeatherType, float> cloudPercentage{
                { WeatherType::SUN, 0},
                { WeatherType::PARTIAL_CLOUDY, 0.20f},
                { WeatherType::CLOUDY, 0.40f},
                { WeatherType::THUNDERSTORM, 0.85f},
        };

        float cloudArea = 10 * 5; // area of one cloud
        auto mapArea = float(map.extent.x * map.extent.z); // area of map
        float numClouds100PercentCoverage = glm::clamp(mapArea / cloudArea, 10.f, 100.f);

        for(auto &[key, value] : cloudPercentage){
            numCloudsPerWeather[key] = glm::ceil(value * numClouds100PercentCoverage);
            cout << numCloudsPerWeather[key] << endl;

        }
    }
}

[[noreturn]] void Server::run() {
    connectWithPlayers();

    int ROUNDS=1;
    int roundCt = 0;
    double roundStartMs = gameTimer.look();
#ifdef SERVER_PRINT_MAX
    double roundMax = 0;
#endif

    while(true){
        updateWeather();
        spawnOrRemoveNpcs();
        for(int i = 0; i<SvPlayer::numPlayers; i++){
            receiveAndSend(i);
        }
#ifdef SERVER_PRINT_MAX
        {
            static double lastMs = 0;
            const double ms = gameTimer.look();
            roundMax = glm::max(roundMax, ms - lastMs);
            lastMs = ms;
        }
#endif

        if(++roundCt == ROUNDS){
            const double ms = gameTimer.look();
            const double deltaMs = ms - roundStartMs;
            const double averageRoundMs = deltaMs / (double) ROUNDS;
#ifndef SERVER_PRINT_MAX
            cout << ROUNDS << " rounds average: " << averageRoundMs << "ms" << endl;
#endif
#ifdef SERVER_PRINT_MAX
            cout << ROUNDS << " rounds; average, max: " << averageRoundMs << "ms" << ", " << roundMax << "ms" << endl;
            roundMax = 0;
#endif

            if(deltaMs < 1000){
                ROUNDS *= 10;
            }

            roundStartMs = ms;
            roundCt = 0;
        }
    }
}

void Server::connectWithPlayers() {
    for(int i = 0; i<SvPlayer::numPlayers; i++){
        sockets[i] = std::make_unique<tcp::socket>(ioService);
        cout << "Waiting for player " << i << endl;
        acceptor.accept(*sockets[i]);

        // Always: first read, then send
        std::vector<msg::Msg> msgs = networking::read(*sockets[i]);
        if(msgs.size() != 1){
            throw std::runtime_error("expected 1 msg");
        }
        const auto msg = msgs.at(0);
        if(msg.type != msg::MsgType::SV_LOGIN){
            throw std::runtime_error("expected SV_LOGIN msg");
        }

        const auto playerName = msg.json.at("playerName").get<string>();

        auto startPos = map.randSurfacePos()+IVEC_UP;
        int playerId = (i + 1) * (globals::maxMovableObjId / SvPlayer::numPlayers + 1);
        int playerNextId = playerId+1;
        int playerMaxId = (i + 2) * (globals::maxMovableObjId / SvPlayer::numPlayers + 1) - 1;

        auto player = std::make_shared<SvPlayer>(playerId,
                                                 globals::ModelType::player, startPos,
                                                 playerName,
                                                 i);
        SvPlayer::players[playerId] = player;
        SvPlayer::playerList.push_back(player);

        const auto& answer = msg::server::ClAddPlayer(playerId,
                                                      startPos,
                                                      globals::SERVER_INTERVAL,
                                                      playerNextId,
                                                      playerMaxId,
                                                      int(gameTimer.look()),
                                                      map.size);
        msgQueue[i].insert(msgQueue[i].begin(), answer);
        addMsgToAll(answer, i);

        addInitialMapNpcStateTo(i);

        // Always: first read, then send
        doSend(i);

        cout << "Connection with " << playerName << " successfully established!" << endl;
    }
    cout << "All players are connected!" << endl;
}

void Server::receiveAndSend(int i) {
    // read
    const auto& messages = networking::read(*sockets[i]);

    // handle and generate replies
    for(const auto& msg : messages){
        handleMsg(msg, i);
    }

    // update and generate replies
    updateMovables(i);

    // send
    doSend(i);
}

void Server::doSend(int i) {
    networking::send(*sockets[i], msgQueue[i]);
    msgQueue[i].resize(0);
}

void Server::shutdown() {
    // send exit message to all alive players

    for(int i = 0; i<SvPlayer::numPlayers; i++){
        msgQueue[i].resize(0);
        if(!connected[i]) continue;

        addMsgTo(msg::server::ClExit(), i);

        // Always: first read, then send
        networking::read(*sockets[i]);
        doSend(i);

        connected[i] = false;
    }

    exit(EXIT_SUCCESS);
}

void Server::updateWeather() {
    static const float MIN_CHANGE_TIME = 10 * 1000;  // 10s
    static const float MIN_UPDATE_TIME = 2 * 1000;   // 2s
    static const float MIN_LIGHTING_TIME = 5 * 1000; // 5s

    double ms = gameTimer.look();

    static bool lightnings = false;
    static double lightningStart, lightningEnd;
    static double nextLighting = ms + MIN_LIGHTING_TIME;
    // next weather change
    static double nextChange = ms + MIN_CHANGE_TIME;
    static double nextUpdate = ms + MIN_UPDATE_TIME;
    // list of cloud IDs
    static std::vector<int> clouds;
    static WeatherType currentWeather = WeatherType::SUN;

    if(currentWeather == WeatherType::THUNDERSTORM && lightnings){
        if(ms >= lightningEnd){
            // stop adding lightnings
            lightnings = false;
            addMsgToAll(msg::server::ClThunderstorm(false));
            cout << "stop adding lightnings" << endl;
        }else if(ms >= lightningStart){
            // add lighting
            if(ms>nextLighting){
                nextLighting = ms + MathUtil::rand_0_1() * MIN_LIGHTING_TIME;
                addMsgToAll(msg::server::ClLighting(
                        clouds[int(MathUtil::rand_0_1() * (double) clouds.size())]
                        ));
            }
        }
    }

    if(ms >= nextUpdate){
        // add/remove clouds

        nextUpdate = ms + MIN_UPDATE_TIME * MathUtil::rand_0_1();

        int delta = int(clouds.size()) - numCloudsPerWeather.at(currentWeather);
        if(delta < 0){
            // add cloud

            static const float BORDER_PERCENT = 0.1;
            glm::vec3 pos =
                    MathUtil::randVec3() * glm::vec3(map.extent) * (1 - 2 * BORDER_PERCENT)
                    + glm::vec3(map.extent) * BORDER_PERCENT;

            globals::ModelType type;
            double rand = MathUtil::rand_0_1();
            if(rand<0.25) type = globals::ModelType::cloud0;
            else if(rand<0.5) type = globals::ModelType::cloud1;
            else if(rand<0.75) type = globals::ModelType::cloud2;
            else type = globals::ModelType::cloud3;

            addMsgToAll(msg::server::ClUpdateCloud(nextId, type, {pos.x,pos.z}));
            clouds.push_back(nextId);
            nextId++;
        }else if(delta > 0){
            // remove cloud

            addMsgToAll(msg::server::ClUpdateCloud(clouds[0], globals::ModelType::cloud0, VEC2_INVALID_POS));
            clouds.erase(clouds.begin());
        }
    }

    if(ms < nextChange) return;

    currentWeather = randWeather(currentWeather);

    if(currentWeather == WeatherType::THUNDERSTORM){
        lightnings = true;
        // wait globals::CLOUD_CHANGE_TIME until clouds turned dark
        lightningStart = ms + globals::CLOUD_CHANGE_TIME;
        lightningEnd = lightningStart + float(MIN_CHANGE_TIME + MIN_CHANGE_TIME * MathUtil::rand_0_1());
        // wait globals::CLOUD_CHANGE_TIME until clouds turned bright again
        nextChange = lightningEnd + globals::CLOUD_CHANGE_TIME;
    }else{
        nextChange = float(ms + MIN_CHANGE_TIME + MIN_CHANGE_TIME * MathUtil::rand_0_1() * 3);
    }

    if(currentWeather == WeatherType::THUNDERSTORM){
        addMsgToAll(msg::server::ClThunderstorm(true));
        cout << "started lighting; end in " << nextChange - ms << endl;
    }
}

WeatherType Server::randWeather(WeatherType currentWeather) {
    static const std::map<double, WeatherType> WEATHER_PROBABILITY{
            {0.3, WeatherType::SUN},
            {0.6, WeatherType::PARTIAL_CLOUDY},
            {0.8, WeatherType::CLOUDY},
            {1.0, WeatherType::THUNDERSTORM},
    };

    // new WeatherType different of old type
    while(true){
        double rand = MathUtil::rand_0_1();
        for(auto &[key, value] : WEATHER_PROBABILITY){
            if(rand > key || currentWeather == value) continue;
            return value;
        }
    }
}

void Server::spawnOrRemoveNpcs() {
    double ms = gameTimer.look();
    float fract = daytime::dayFract(ms);
    int targetAnimalCt = daytime::numAnimals(fract);
    int targetEnemyCt = daytime::numEnemies(fract);
    // cout << "Animals|Enemies: " << targetAnimalCt << "|" << targetEnemyCt << endl;

    const auto enemyDt = targetEnemyCt - (int) SvNpc::npcs.size();

    if(enemyDt > 0){
        for(int i = 0; i < enemyDt; i++){
            // spawn enemy

            globals::ModelType type;
            if(MathUtil::rand_0_1() < 0.5){
                type = globals::ModelType::skeleton;
            }else{
                type = globals::ModelType::zombie;
            }

            glm::ivec3 pos = map.randSurfacePos()+IVEC_UP;
            SvNpc::npcs[nextId] = std::make_shared<SvNpc>(nextId, type, pos, gameTimer.look());
            addMsgToAll(msg::server::ClAddMovable(nextId, pos, type));
            nextId++;
        }
    }else if(enemyDt < 0){
        for(int i = 0; i < -enemyDt; i++){
            // rm enemy

            const auto it = SvNpc::npcs.begin();
            addMsgToAll(msg::server::ClRemoveNpc((*it).second->id));
            SvNpc::npcs.erase(it);
        }
    }




    // TODO: spawn/remove animals
}

void Server::updateMovables(int i) {
    double ms = gameTimer.look();
    // double dt_ms = ms - lastUpdateMs;
    lastUpdateMs = ms;

    for(auto &[id, npc] : SvNpc::npcs){
        npc->updatePath(ms);
        npc->update(ms, i);
    }
    for(auto &player:SvPlayer::playerList){
        player->update();
    }
}

void Server::handleMsg(const msg::Msg &msg, int i) {
    switch(msg.type){
        case msg::MsgType::SV_LOGOUT:
            handleSvLogout(msg, i);
            break;
        case msg::MsgType::SV_UPDATE_PLAYER:
            handleSvUpdatePlayer(msg, i);
            break;
        case msg::MsgType::SV_UPDATE_HP:
            handleSvUpdateHp(msg, i);
            break;
        case msg::MsgType::SV_DIG:
            handleSvDig(msg, i);
            break;
        case msg::MsgType::SV_PLACE:
            handleSvPlace(msg, i);
            break;
        case msg::MsgType::SV_ADD_PROJECTILE:
            handleSvAddProjectile(msg, i);
            break;
        case msg::MsgType::SV_UPDATE_PROJECTILE:
            handleSvUpdateProjectile(msg, i);
            break;

        case msg::MsgType::CL_ADD_PLAYER:
        case msg::MsgType::CL_FREEZE:
        case msg::MsgType::CL_UPDATE_INVENTORY:
        case msg::MsgType::CL_ADD_MOVABLE:
        case msg::MsgType::CL_UPDATE_ANIMATED:
        case msg::MsgType::CL_UPDATE_HP:
        case msg::MsgType::CL_UPDATE_PROJECTILE:
        case msg::MsgType::CL_UPDATE_BLOCK:
        case msg::MsgType::CL_UPDATE_CLOUD:
        case msg::MsgType::CL_THUNDERSTORM:
        case msg::MsgType::CL_LIGHTING:
        case msg::MsgType::CL_EXIT:

        case msg::MsgType::SV_LOGIN:
        case msg::MsgType::MESSAGE_TYPE_ERROR:
            throw runtime_error("network error");
    }
}

void Server::handleSvLogout(const msg::Msg &msg, int i) {
    cout << "Received SV_LOGOUT from player " << i << endl;

    msgQueue[i].resize(0);
    addMsgTo(msg::server::ClExit(), i);
    doSend(i);

    connected[i] = false;

    shutdown();
}

void Server::handleSvUpdatePlayer(const msg::Msg &msg, int i) {
    GRAPRA_FROM_JSON(newPos,glm::vec3)
    GRAPRA_FROM_JSON(rot,glm::vec2)
    GRAPRA_FROM_JSON(animationType, globals::AnimationType)
    GRAPRA_FROM_JSON(offsetMs,int)

    auto player = SvPlayer::playerList[i];
    player->pos = newPos;

    addMsgToAll(msg::server::ClUpdateAnimated(player->id, newPos, rot, animationType, offsetMs), i);
}

void Server::handleSvUpdateHp(const msg::Msg &msg, int i) {
    GRAPRA_FROM_JSON(movableObjId,int)
    GRAPRA_FROM_JSON(hpChange,float)

    if(SvPlayer::players.count(movableObjId)){
        auto player = SvPlayer::players.at(movableObjId);
        player->hp += hpChange;

        // check if player died
        if(player->hp <= 0){
            // freeze
            player->frozen = true;
            addMsgTo(msg::server::ClFreeze(true),i);

            // reset player hp
            addMsgTo(msg::server::ClUpdateHp(player->id, -player->hp + globals::hp), i);

            // set to random position
            player->pos = glm::vec3(map.randSurfacePos() + IVEC_UP) + glm::vec3{0.5, 0, 0.5};
            const auto rot = glm::vec2(0,0);
            addMsgTo(msg::server::ClUpdateAnimated(player->id, player->pos, rot, globals::AnimationType::STANDING, 0), i);
        }
    }
    if(SvNpc::npcs.count(movableObjId)){
        auto npc = SvNpc::npcs.at(movableObjId);
        npc->hp += hpChange;

        // remove dead enemies
        if(npc->hp <= 0){
            addMsgToAll(msg::server::ClRemoveNpc(npc->id));
            SvNpc::npcs.erase(movableObjId);
        }
    }
}

void Server::handleSvDig(const msg::Msg &msg, int i) {
    GRAPRA_FROM_JSON(oldBlockType,globals::ModelType)
    GRAPRA_FROM_JSON(pos,glm::ivec3)

    auto actualBlockType = map.get(pos);
    if(actualBlockType != oldBlockType){
        // there is a conflict, force client to revert
        addMsgTo(msg::server::ClUpdateInventory(oldBlockType, -1) ,i);
        addMsgTo(msg::server::ClUpdateBlock(actualBlockType, pos), i);
    }else{
        map.set(pos, globals::ModelType::air);
        addMsgToAll(msg::server::ClUpdateBlock(globals::ModelType::air, pos) ,i);

        // check if any path is affected
        for(auto &[id, npc] : SvNpc::npcs){
            for(const auto &npcPos:npc->path){
                // check if block below animal path is now missing
                if(npcPos + IVEC_DOWN == pos){
                    npc->pathStartMs = std::numeric_limits<int>::min();
                    npc->updatePath(gameTimer.look());
                }
            }
        }
    }
}

void Server::handleSvPlace(const msg::Msg &msg, int i) {
    GRAPRA_FROM_JSON(newBlockType,globals::ModelType)
    GRAPRA_FROM_JSON(pos,glm::ivec3)

    auto actualBlockType = map.get(pos);
    if(actualBlockType != globals::ModelType::air){
        // there is a conflict, force client to revert
        addMsgTo(msg::server::ClUpdateInventory(newBlockType, +1) ,i);
        addMsgTo(msg::server::ClUpdateBlock(actualBlockType, pos), i);
    }else{
        map.set(pos, newBlockType);
        addMsgToAll(msg::server::ClUpdateBlock(newBlockType, pos) ,i);

        // check if any path is affected
        for(auto &[id, npc] : SvNpc::npcs){
            for(const glm::ivec3 &npcPos:npc->path){
                // Animals/enemies are no larger than two blocks
                if(npcPos == pos || npcPos + IVEC_UP == pos){
                    npc->pathStartMs = std::numeric_limits<int>::min();
                    npc->updatePath(gameTimer.look());
                }
            }
        }
    }
}

void Server::handleSvAddProjectile(const msg::Msg &msg, int i) {
    GRAPRA_FROM_JSON(movableObjId,int)
    GRAPRA_FROM_JSON(pos,glm::vec3)
    GRAPRA_FROM_JSON(velocity,glm::vec3)

    addMsgToAll(msg::server::ClUpdateProjectile(movableObjId, pos, velocity),i);
}

void Server::handleSvUpdateProjectile(const msg::Msg &msg, int i) {
    GRAPRA_FROM_JSON(movableObjId,int)
    GRAPRA_FROM_JSON(finalPos,glm::vec3)

    addMsgToAll(msg::server::ClUpdateProjectile(movableObjId, finalPos, {0,0,0}),i);
}

//
// CREATE MESSAGES FOR CLIENTS
//

void Server::addInitialMapNpcStateTo(int idx) {
    // blocks
    for (int i = 0; i < map.extent.x; i++) {
        for (int j = 0; j < map.extent.y; j++) {
            for (int k = 0; k < map.extent.z; k++) {
                const auto block = map.get({i,j,k});
                if(block!=globals::ModelType::air){
                    addMsgTo(msg::server::ClUpdateBlock(block,{i,j,k}),idx);
                }
            }
        }
    }

    // NPCs
    for(auto &[id, npc] : SvNpc::npcs){
        addMsgTo(msg::server::ClAddMovable(npc->id, npc->pos, npc->type), idx);
    }
}

void Server::addMsgToAll(const msg::Msg& msg, int except) {
    for(int i = 0; i<SvPlayer::numPlayers; i++){
        if(i==except) continue;

        msgQueue[i].push_back(msg);
    }
}

void Server::addMsgTo(const msg::Msg& msg, int i) {
    msgQueue[i].push_back(msg);
}
