#include "AStar.h"
#include <queue>
//#include <boost/functional/hash_fwd.hpp>

using std::cout;
using std::endl;

namespace {
    SvMap* map;

    struct pairCompare {
        bool operator()(const std::pair<float, glm::ivec3>& left, const std::pair<float, glm::ivec3>& right) const {
            if(left.first != right.first) return left.first < right.first;

            if(left.second.x != right.second.x) return left.second.x < right.second.x;
            if(left.second.y != right.second.y) return left.second.y < right.second.y;
            return left.second.z < right.second.z;
        }
    };

    struct vecCompare {
        size_t operator()(const glm::ivec3 &k) const {
            return std::hash<int>()(k.x)
                   ^ std::hash<int>()(k.y)
                   ^ std::hash<int>()(k.z);

//            std::size_t seed = 0;
//            boost::hash_combine(seed, k.x);
//            boost::hash_combine(seed, k.y);
//            boost::hash_combine(seed, k.z);
//            return seed;
        }

        bool operator()(const glm::ivec3 &a, const glm::ivec3 &b) const {
            return a.x == b.x
                   && a.y == b.y
                   && a.z == b.z;
        }
    };

    [[maybe_unused]] void printGScore(const std::unordered_map<glm::ivec3, float, vecCompare, vecCompare>& x){
        cout << "gScore:" << endl;
        for(auto &[k, v] : x){
            cout << "\t" << k << ": " << v << endl;
        }
    }

    [[maybe_unused]] void printCameFrom(const std::unordered_map<glm::ivec3, glm::ivec3, vecCompare, vecCompare>& x){
        cout << "cameFrom:" << endl;
        for(auto &[k, v] : x){
            cout << "\t" << k << ": " << v << endl;
        }
    }
}

void AStar::init(SvMap* _map) {
    map = _map;
}

std::vector<glm::ivec3> AStar::neighborsOf(const glm::ivec3 &pos, int heightInBlocks) {
    std::vector<glm::ivec3> result;

    const std::vector<glm::ivec3> planarN = {
            {pos + IVEC_FORWARD},
            {pos + IVEC_RIGHT},
            {pos + IVEC_BACKWARD},
            {pos + IVEC_LEFT},
    };

//    if(map->get({pos.x, pos.y+heightInBlocks, pos.z}) == globals::ModelType::air){
//        // above is air -> jump upwards
//
//        const auto abovePos = pos + IVEC_UP;
//        result.push_back(abovePos);
//    }

    for(const auto& n:planarN){
        if(map->inval(n)) {
            continue;
        }
        const auto typeN = map->get(n);
        if(typeN != globals::ModelType::air){
            // solid block at n

            // if(typeN == globals::ModelType::water) continue; // water can only be at lowest height!

            const auto aboveN = n+IVEC_UP;
            const auto abovePos = pos + IVEC_UP;
            if(map->blockTraversable(abovePos, heightInBlocks) && map->blockTraversable(aboveN, heightInBlocks)){
                // abovePos and aboveN are traversable -> jump forward

                result.push_back(aboveN);
            }
        }else if(map->blockTraversable(n, heightInBlocks)){
            // n is traversable

            const auto belowN = n+IVEC_DOWN;
            // if(map->inval(belowN)) continue; // One can't dig the lowest blocks
            const auto typeBelowN = map->get(belowN);
            if(typeBelowN != globals::ModelType::air){
                // belowN is solid -> move forward

                if(typeBelowN == globals::ModelType::water) continue;
                result.push_back(n);
            }else{
                // belowN is air

                const auto below2N = belowN + IVEC_DOWN;
                const auto typeBelow2N = map->get(below2N);
                if(typeBelow2N != globals::ModelType::air){
                    // 2belowN is solid -> fall forward

                    if(typeBelow2N == globals::ModelType::water) continue;
                    result.push_back(belowN);
                }
            }
        }
    }

    return result;
}

std::deque<glm::ivec3> AStar::search(glm::ivec3 start, glm::ivec3 goal, float height) {
    const int heightInBlocks = (int) glm::ceil(height);

    // Pair <fScore, node>.
    std::pair<float, glm::ivec3> closestToGoal;

    // h(n) := distance(n, goal)
    // h(n) := distance(n, goal) ^ 2
#define H_FUNC(ivecA, ivecB) glm::pow(glm::distance(glm::vec3(ivecA), glm::vec3(ivecB)), 2.f)

    // For node n, gScore[n] is the cost of the cheapest path from start to n currently known.
    // Default value: Infinity.
    std::unordered_map<glm::ivec3, float, vecCompare, vecCompare> gScore{{start, 0.f}};

    // For node n, cameFrom[n] is the node immediately preceding it on the cheapest path from start
    // to n currently known.
    std::unordered_map<glm::ivec3, glm::ivec3, vecCompare, vecCompare> cameFrom{};

    // List of pairs <fScore, node> sorted by fScore.
    // For node n, fScore[n] := gScore[n] + h(n).
    // fScore[n] represents our current best guess as to
    // how short a path from start to finish can be if it goes through n.
    std::set<std::pair<float, glm::ivec3>, pairCompare> openSet;
    {
        float fScore = H_FUNC(start, goal);
        openSet.emplace(fScore, start);
        closestToGoal = {fScore, start};
    }


    while (!openSet.empty()) {
        const auto currentIt = openSet.begin();
        // don't use a reference here as openSet gets modified below
        glm::ivec3 current = (*currentIt).second;
        if (current == goal) {
            // cout << "gScore.size(): " << gScore.size() << endl;

            // reconstruct_path(cameFrom, curr)
            std::deque<glm::ivec3> total_path{current};
            while (cameFrom.count(current)) {
                current = cameFrom.at(current);
                total_path.push_front(current);
            }
            return total_path;
        }
        openSet.erase(currentIt);
        const auto& neighbors = neighborsOf(current, heightInBlocks);
        for (const auto &neighbor: neighbors) {
            float d = H_FUNC(current, neighbor);
            float tentative_gScore = gScore.at(current) + d;
            if(!gScore.count(neighbor)){
                // Discovered a new node. Add it!
                cameFrom[neighbor] = current;
                gScore[neighbor] = tentative_gScore;
                float fScore = tentative_gScore + H_FUNC(neighbor, goal);
                openSet.emplace(fScore, neighbor);
                if(fScore < closestToGoal.first) closestToGoal = {fScore, neighbor};
            }else if (tentative_gScore < gScore.at(neighbor)) {
                // This path to neighbor is better than any previous one. Record it!
                cameFrom[neighbor] = current;
                gScore[neighbor] = tentative_gScore;
                float fScore = tentative_gScore + H_FUNC(neighbor, goal);
                // Remove previous entry of neighbor with worse fScore
                for (auto it=openSet.begin(); it!=openSet.end(); ++it){
                    if((*it).second == neighbor){
                        openSet.erase(it);
                        break;
                    }
                }
                openSet.emplace(fScore, neighbor);
                if(fScore < closestToGoal.first) closestToGoal = {fScore, neighbor};
            }
        }
    }

    // A*: No valid path exists!
    // return {IVEC3_INVALID_POS};

    // However, in our case we take the path leading us closest to the goal!
//    cout << "No valid path!" << endl;
//    cout << "start: " << start << endl;
//    cout << "goal:  " << goal << endl;
//    printGScore(gScore);
//    printCameFrom(cameFrom);

    // reconstruct_path(cameFrom, curr)
    glm::ivec3 current = closestToGoal.second;
    std::deque<glm::ivec3> total_path{current};
    while (cameFrom.count(current)) {
        current = cameFrom.at(current);
        total_path.push_front(current);
    }
    return total_path;

#undef H_FUNC
}
