#include "include/ZombiePool.h"
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <cmath>

ZombiePool::ZombiePool(SDL_Renderer* renderer, size_t poolSize) 
    : renderer(renderer) {
    // Đặt trước không gian cho các vector
    pool.reserve(poolSize);
    activeZombies.reserve(poolSize);
    isInUse.reserve(poolSize);
}

void ZombiePool::AddZombie() {
    // Tạo zombie ban đầu ở ngoài màn hình
    Zombie* zombie = new Zombie(renderer, -1000.0f, -1000.0f);
    pool.push_back(zombie);
    isInUse.push_back(false);  // Đánh dấu là chưa sử dụng ban đầu
}

ZombiePool::~ZombiePool() {
    try {
        for (Zombie* zombie : pool) {
            delete zombie;
        }
        pool.clear();
        activeZombies.clear();
        isInUse.clear();    } catch (...) {
        std::cerr << "ZombiePool: Lỗi trong quá trình dọn dẹp" << std::endl;
    }
}

Zombie* ZombiePool::GetZombie() {
    // Đầu tiên kiểm tra các zombie tái sử dụng
    if (!recycledZombies.empty()) {
        Zombie* zombie = recycledZombies.front();
        recycledZombies.pop();
        activeZombies.push_back(zombie);
        return zombie;
    }

    // Nếu không có zombie tái sử dụng, lấy một zombie từ pool
    try {
        for (size_t i = 0; i < pool.size(); ++i) {
            if (!isInUse[i] && pool[i] != nullptr) {
                isInUse[i] = true;
                activeZombies.push_back(pool[i]);
                
                // Ghi log thống kê sử dụng pool
                size_t activeCount = std::count(isInUse.begin(), isInUse.end(), true);
                if (activeCount > pool.size() * 0.8f) {
                    std::cout << "ZombiePool: Cảnh báo sử dụng cao - " << activeCount << "/" 
                              << pool.size() << " zombie đang hoạt động" << std::endl;
                    // Kích hoạt tái sử dụng tích cực khi pool gần đầy
                    return pool[i];
                }
                
                return pool[i];
            }
        }
        std::cerr << "ZombiePool: Không có zombie khả dụng trong pool" << std::endl;
        return nullptr;
    } catch (const std::exception& e) {
        std::cerr << "ZombiePool: Lỗi trong GetZombie: " << e.what() << std::endl;
        return nullptr;
    }
}

void ZombiePool::ReturnZombie(Zombie* zombie) {
    if (!zombie) {
        std::cerr << "ZombiePool: Cố gắng trả về zombie null" << std::endl;
        return;
    }

    try {
        // Tìm và đánh dấu zombie là không sử dụng
        bool found = false;
        for (size_t i = 0; i < pool.size(); ++i) {
            if (pool[i] == zombie) {
                isInUse[i] = false;
                found = true;
                break;
            }
        }

        if (!found) {
            std::cerr << "ZombiePool: Cố gắng trả về zombie không thuộc pool này" << std::endl;
            return;
        }

        // Xóa khỏi danh sách zombie đang hoạt động sử dụng erase-remove idiom
        activeZombies.erase(
            std::remove(activeZombies.begin(), activeZombies.end(), zombie),
            activeZombies.end()
        );

    } catch (const std::exception& e) {
        std::cerr << "ZombiePool: Lỗi trong ReturnZombie: " << e.what() << std::endl;
    }
}

void ZombiePool::Update(float deltaTime, Player* player) {
    if (!player) {
        std::cerr << "ZombiePool: Player null trong Update" << std::endl;
        return;
    }

    try {
        // Cập nhật khoảng cách zombie và tái sử dụng nếu cần
        UpdateZombieDistances(player);
        
        // Sao chép để tránh sửa đổi trong quá trình lặp
        std::vector<Zombie*> currentActive = activeZombies;
        
        for (Zombie* zombie : currentActive) {
            if (zombie && !zombie->IsDead()) {
                zombie->Update(deltaTime, player, currentActive);
                
                // Nếu zombie chết hoặc quá xa, tái sử dụng nó
                if (zombie->IsDead() || IsZombieTooFar(zombie, player, RECYCLE_DISTANCE)) {
                    ReturnZombie(zombie);
                }
            }
        }

        // Tối ưu hóa phân bố zombie định kỳ
        static float optimizeTimer = 0.0f;
        optimizeTimer += deltaTime;
        if (optimizeTimer >= 1.0f) {  // Tối ưu hóa mỗi giây
            OptimizeZombieDistribution(player);
            optimizeTimer = 0.0f;
        }
    } catch (const std::exception& e) {
        std::cerr << "ZombiePool: Lỗi trong Update: " << e.what() << std::endl;
    }
}

void ZombiePool::Render(SDL_Renderer* renderer, Camera* camera) {
    if (!renderer || !camera) {
        std::cerr << "ZombiePool: Renderer hoặc camera null trong Render" << std::endl;
        return;
    }

    try {
        for (Zombie* zombie : activeZombies) {
            if (zombie && !zombie->IsDead()) {
                zombie->Render(renderer, camera);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "ZombiePool: Lỗi trong Render: " << e.what() << std::endl;
    }
}

void ZombiePool::RecycleDistantZombies(Player* player, float maxDistance) {
    std::vector<Zombie*> zombiesToRecycle;
    
    for (Zombie* zombie : activeZombies) {
        if (IsZombieTooFar(zombie, player, maxDistance)) {
            zombiesToRecycle.push_back(zombie);
        }
    }
    
    for (Zombie* zombie : zombiesToRecycle) {
        ReturnZombie(zombie);
        recycledZombies.push(zombie);
    }
}

void ZombiePool::OptimizeZombieDistribution(Player* player) {
    size_t activeCount = activeZombies.size();
    
    // Nếu chúng ta đang sử dụng quá nhiều zombie, tái sử dụng các zombie ở xa
    if (activeCount > pool.size() * 0.8f) {
        RecycleDistantZombies(player, MIN_RECYCLE_DISTANCE);
    }
    
    // Đảm bảo zombie được phân bố tốt xung quanh người chơi
    std::vector<Zombie*> poorlyPlaced;
    for (Zombie* zombie : activeZombies) {
        if (IsZombieTooFar(zombie, player, OPTIMAL_DISTANCE)) {
            poorlyPlaced.push_back(zombie);
        }
    }
    
    // Định vị lại các zombie bị đặt không tốt
    for (Zombie* zombie : poorlyPlaced) {
        SDL_Point newPos = GetOptimalSpawnPosition(player);
        zombie->Reset(static_cast<float>(newPos.x), static_cast<float>(newPos.y));
    }
}

bool ZombiePool::IsZombieTooFar(const Zombie* zombie, const Player* player, float maxDistance) const {
    if (!zombie || !player) return true;
    
    float dx = zombie->GetX() - player->GetX();
    float dy = zombie->GetY() - player->GetY();
    float distSquared = dx * dx + dy * dy;
    
    return distSquared > maxDistance * maxDistance;
}

void ZombiePool::UpdateZombieDistances(Player* player) {
    // Tái sử dụng các zombie quá xa người chơi
    RecycleDistantZombies(player, RECYCLE_DISTANCE);
}

SDL_Point ZombiePool::GetOptimalSpawnPosition(Player* player) const {
    // Tạo vị trí ở khoảng cách tối ưu từ người chơi
    float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;
    float distance = OPTIMAL_DISTANCE * (0.8f + 0.4f * static_cast<float>(rand()) / RAND_MAX);
    
    SDL_Point pos;
    pos.x = static_cast<int>(player->GetX() + cos(angle) * distance);
    pos.y = static_cast<int>(player->GetY() + sin(angle) * distance);
    
    return pos;
}

void ZombiePool::PrewarmPool() {
    // Tạo một số zombie ban đầu để tránh giật khi lần đầu sinh ra
    for (size_t i = 0; i < pool.size() / 4; ++i) {
        Zombie* zombie = GetZombie();
        if (zombie) {
            ReturnZombie(zombie);
            recycledZombies.push(zombie);
        }
    }
}