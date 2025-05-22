#include "include/Zombie.h"
#include <cmath>

Zombie::Zombie(SDL_Renderer* renderer, float startX, float startY) 
    : renderer(renderer), x(startX), y(startY), rotation(0.0f),
      health(STARTING_HEALTH), isDead(false), speed(100.0f), isAttacking(false), lastAttackTime(0),
      currentFrame(0), frameTimer(0.0f), frameDuration(DEFAULT_FRAME_DURATION),
      knockbackVelocityX(0.0f), knockbackVelocityY(0.0f), knockbackDuration(0.0f) {
    
    // Khởi tạo hitbox
    hitbox.w = 50;  // chiều rộng của zombie
    hitbox.h = 50;  // chiều cao của zombie
    hitbox.x = static_cast<int>(x - hitbox.w / 2);
    hitbox.y = static_cast<int>(y - hitbox.h / 2);

    LoadTextures();
}

Zombie::~Zombie() {
    // Dọn dẹp các texture hoạt ảnh
    for (SDL_Texture* tex : moveFrames) {
        if (tex) SDL_DestroyTexture(tex);
    }
    for (SDL_Texture* tex : attackFrames) {
        if (tex) SDL_DestroyTexture(tex);
    }
}

void Zombie::LoadTextures() {
    // Tải hoạt ảnh di chuyển
    LoadAnimationSet(moveFrames, "assets/zombie/move/", "zombie_move_", MOVE_FRAME_COUNT);
    // Tải hoạt ảnh tấn công
    LoadAnimationSet(attackFrames, "assets/zombie/attack/", "zombie_attack_", ATTACK_FRAME_COUNT);

    // Thiết lập hình chữ nhật nguồn và đích
    if (!moveFrames.empty() && moveFrames[0]) {
        SDL_QueryTexture(moveFrames[0], nullptr, nullptr, &srcRect.w, &srcRect.h);
        srcRect.x = 0;
        srcRect.y = 0;
        
        // Co dãn sprite để phù hợp với kích thước người chơi (khoảng 64x64)
        float scale = 0.5f;  // Điều chỉnh để phù hợp với kích thước người chơi
        destRect.w = static_cast<int>(srcRect.w * scale);
        destRect.h = static_cast<int>(srcRect.h * scale);
        
        // Cập nhật hitbox để phù hợp với kích thước sprite
        hitbox.w = static_cast<int>(destRect.w * 0.6f);  // Làm hitbox nhỏ hơn sprite một chút
        hitbox.h = static_cast<int>(destRect.h * 0.6f);
    }
}

void Zombie::LoadAnimationSet(std::vector<SDL_Texture*>& frames, const std::string& basePath, 
                            const std::string& prefix, int frameCount) {
    frames.clear();
    for (int i = 0; i < frameCount; ++i) {
        std::string path = basePath + prefix + std::to_string(i) + ".png";
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (!surface) {
            std::cerr << "Failed to load zombie texture: " << path << " Error: " << IMG_GetError() << std::endl;
            continue;
        }
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (!texture) {
            std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
            continue;
        }
        frames.push_back(texture);
    }
}

void Zombie::UpdateAnimation(float deltaTime) {
    frameTimer += deltaTime;
    if (frameTimer >= frameDuration) {
        frameTimer = 0;
        currentFrame++;
        
        const auto& currentFrames = isAttacking ? attackFrames : moveFrames;
        int maxFrames = isAttacking ? ATTACK_FRAME_COUNT : MOVE_FRAME_COUNT;
        
        if (currentFrame >= maxFrames) {
            currentFrame = 0;
            if (isAttacking) {
                // Tùy chọn: Đặt lại trạng thái tấn công sau khi hoàn thành hoạt ảnh
                // isAttacking = false;
            }
        }
    }
}

void Zombie::Update(float deltaTime, Player* player, const std::vector<Zombie*>& zombies) {
    if (isDead) return;

    // Xử lý hiệu ứng bị đẩy lùi
    if (knockbackDuration > 0) {
        x += knockbackVelocityX * deltaTime;
        y += knockbackVelocityY * deltaTime;
        knockbackDuration -= deltaTime;
        
        // Cập nhật hitbox trong khi bị đẩy lùi
        hitbox.x = static_cast<int>(x - hitbox.w / 2);
        hitbox.y = static_cast<int>(y - hitbox.h / 2);
        
        // Nếu thời gian bị đẩy lùi kết thúc, đặt lại vận tốc
        if (knockbackDuration <= 0) {
            knockbackVelocityX = 0;
            knockbackVelocityY = 0;
        }
        
        // Vẫn cập nhật hoạt ảnh ngay cả khi bị đẩy lùi
        UpdateAnimation(deltaTime);
        return;  // Bỏ qua di chuyển bình thường khi đang bị đẩy lùi
    }

    // Lấy vị trí người chơi và tính toán hướng
    float playerX = player->GetX();
    float playerY = player->GetY();
      // Calculate distance to player
    float dx = playerX - x;
    float dy = playerY - y;
    float distanceToPlayer = std::sqrt(dx * dx + dy * dy);

    // Tính toán góc quay để đối mặt với người chơi
    rotation = (atan2(dy, dx) * 180.0f / M_PI);  // Bỏ +90 nếu sprite zombie mặc định quay sang phải

    // Tính toán hướng cơ bản dựa trên độ ưu tiên và đội hình
    if (distanceToPlayer < CLOSE_RANGE) {
        // Tầm gần: Tấn công trực tiếp
        if (distanceToPlayer > 0) {
            dx /= distanceToPlayer;
            dy /= distanceToPlayer;
        }
    } else if (distanceToPlayer < FORMATION_RANGE) {
        // Tầm đội hình: Cố gắng tạo thành vòng tròn xung quanh người chơi
        float angleToPlayer = std::atan2(dy, dx);
        float desiredAngle = angleToPlayer;
        
        // Tìm khoảng trống trong đội hình
        int zombieCount = 0;
        float angleSum = 0;
        for (auto other : zombies) {
            if (other != this && !other->isDead) {
                float otherDx = other->GetX() - playerX;
                float otherDy = other->GetY() - playerY;
                float otherAngle = std::atan2(otherDy, otherDx);
                angleSum += otherAngle;
                zombieCount++;
            }
        }

        if (zombieCount > 0) {
            // Cố gắng phân bố zombie đều xung quanh vòng tròn
            float averageAngle = angleSum / zombieCount;
            desiredAngle = averageAngle + (2 * M_PI / (zombieCount + 1));
        }

        // Tính toán vị trí mong muốn trên vòng tròn
        float targetX = playerX + FORMATION_RADIUS * std::cos(desiredAngle);
        float targetY = playerY + FORMATION_RADIUS * std::sin(desiredAngle);

        // Lấy hướng đến vị trí đội hình
        dx = targetX - x;
        dy = targetY - y;
        float formationDist = std::sqrt(dx * dx + dy * dy);
        if (formationDist > 0) {
            dx /= formationDist;
            dy /= formationDist;
        }
    } else {
        // Ngoài tầm đội hình: Hành vi đuổi theo tiêu chuẩn
        if (distanceToPlayer > 0) {
            dx /= distanceToPlayer;
            dy /= distanceToPlayer;
        }
    }

    // Áp dụng hành vi đàn đông với trọng số thích hợp
    ApplyFlockingBehavior(zombies, dx, dy);

    // Chuẩn hóa hướng di chuyển cuối cùng
    float finalLength = std::sqrt(dx * dx + dy * dy);
    if (finalLength > 0) {
        dx /= finalLength;
        dy /= finalLength;
    }

    // Di chuyển đến mục tiêu
    x += dx * speed * deltaTime;
    y += dy * speed * deltaTime;

    // Cập nhật vị trí hitbox để ở giữa zombie
    hitbox.x = static_cast<int>(x - hitbox.w / 2);
    hitbox.y = static_cast<int>(y - hitbox.h / 2);

    // Cập nhật trạng thái tấn công
    bool wasAttacking = isAttacking;
    if (CheckCollisionWithPlayer(player)) {
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastAttackTime >= ATTACK_COOLDOWN) {
            isAttacking = true;
            lastAttackTime = currentTime;
            player->TakeDamage(WaveConfig::ZOMBIE_BASE_DAMAGE);
            if (!wasAttacking) {
                // Đặt lại hoạt ảnh khi bắt đầu tấn công
                currentFrame = 0;
                frameTimer = 0;
            }
        }
    } else {
        if (wasAttacking) {
            // Đặt lại hoạt ảnh khi ngừng tấn công
            isAttacking = false;
            currentFrame = 0;
            frameTimer = 0;
        }
    }

    // Cập nhật hoạt ảnh
    UpdateAnimation(deltaTime);
}

void Zombie::Render(SDL_Renderer* renderer, Camera* camera) {
    if (isDead) return;

    // Lấy frame hoạt ảnh hiện tại
    const auto& currentFrames = isAttacking ? attackFrames : moveFrames;
    if (currentFrames.empty() || currentFrame >= currentFrames.size() || !currentFrames[currentFrame]) {
        // Hiển thị dự phòng nếu texture không được tải
        SDL_Rect screenRect = {
            static_cast<int>(hitbox.x - camera->GetX()),
            static_cast<int>(hitbox.y - camera->GetY()),
            hitbox.w,
            hitbox.h
        };
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_RenderFillRect(renderer, &screenRect);
        return;
    }

    // Cập nhật vị trí hình chữ nhật đích cho việc render
    destRect.x = static_cast<int>(x - destRect.w / 2 - camera->GetX());
    destRect.y = static_cast<int>(y - destRect.h / 2 - camera->GetY());

    // Render frame hiện tại với góc quay
    SDL_Point center = { destRect.w / 2, destRect.h / 2 };
    SDL_RenderCopyEx(renderer, currentFrames[currentFrame], &srcRect, &destRect, 
                     rotation, &center, SDL_FLIP_NONE);

    // Luôn hiển thị hitbox để debug
    SDL_Rect hitboxScreen = {
        static_cast<int>(hitbox.x - camera->GetX()),
        static_cast<int>(hitbox.y - camera->GetY()),
        hitbox.w,
        hitbox.h
    };
    
    // Vẽ đường viền hitbox màu đỏ
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &hitboxScreen);
    
    // Vẽ thanh máu phía trên zombie
    SDL_Rect healthBar = {
        hitboxScreen.x,
        hitboxScreen.y - 10,
        hitboxScreen.w,
        5
    };
    
    // Thanh máu nền (màu đỏ)
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(renderer, &healthBar);
    
    // Thanh máu phía trước (màu xanh lá)
    healthBar.w = static_cast<int>((health / 5.0f) * hitboxScreen.w);
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderFillRect(renderer, &healthBar);
}

bool Zombie::CheckCollisionWithBullet(Bullet* bullet) {
    if (isDead) return false;
    
    SDL_Rect bulletHitbox = bullet->GetHitbox();
    if (SDL_HasIntersection(&hitbox, &bulletHitbox)) {
        // Tính toán hướng từ đạn đến zombie cho hiệu ứng đẩy lùi
        float dx = x - bullet->GetX();
        float dy = y - bullet->GetY();
        
        // Sử dụng loại đạn để xác định sát thương và lực đẩy lùi
        bool isShotgunPellet = (bullet->GetBulletType() == BulletType::SHOTGUN_PELLET);
          // Gây sát thương với hiệu ứng đẩy lùi theo hướng bay của đạn
        TakeDamageWithKnockback(dx, dy, isShotgunPellet, bullet);
        return true;
    }
    return false;
}

bool Zombie::CheckCollisionWithPlayer(Player* player) {
    if (isDead) return false;

    SDL_Rect playerDestRect = player->GetDestRect();
    return SDL_HasIntersection(&hitbox, &playerDestRect);
}

void Zombie::TakeDamage() {
    if (isDead) return;
    
    health--;
    if (health <= 0) {
        isDead = true;
    }
}

void Zombie::TakeDamageWithKnockback(float damageX, float damageY, bool isShotgunPellet, Bullet* bullet) {
    // Chỉ áp dụng đẩy lùi cho đạn shotgun
    if (isShotgunPellet) {
        float force = WeaponConfig::Shotgun::KNOCKBACK_FORCE * WeaponConfig::Shotgun::KNOCKBACK_MULTIPLIER;
        
        // Tính toán hướng đẩy lùi
        float length = std::sqrt(damageX * damageX + damageY * damageY);
        if (length > 0) {
            knockbackVelocityX = (damageX / length) * force;
            knockbackVelocityY = (damageY / length) * force;
        }
        
        // Thiết lập thời gian đẩy lùi
        knockbackDuration = WeaponConfig::Shotgun::KNOCKBACK_DURATION;
    }

    // Áp dụng sát thương phù hợp dựa trên loại vũ khí
    if (isShotgunPellet) {
        health -= WeaponConfig::Shotgun::PELLET_DAMAGE;
    } else if (bullet && bullet->GetBulletType() == BulletType::RIFLE) {
        health -= WeaponConfig::Rifle::DAMAGE;
    } else {
        health -= WeaponConfig::Pistol::DAMAGE;
    }
    if (health <= 0) {
        isDead = true;
    }
}

void Zombie::ApplyFlockingBehavior(const std::vector<Zombie*>& zombies, float& dx, float& dy) {
    float sepX = 0, sepY = 0;
    float aliX = 0, aliY = 0;
    float cohX = 0, cohY = 0;
    
    // Áp dụng cả ba hành vi đàn đông (flocking behaviors)
    // - Separate: Giúp zombie không bị chen chúc vào nhau
    // - Align: Điều chỉnh hướng di chuyển theo các zombie xung quanh
    // - Cohere: Giúp zombie di chuyển về phía trung tâm của nhóm
    Separate(zombies, sepX, sepY);
    Align(zombies, aliX, aliY);
    Cohere(zombies, cohX, cohY);

    // Cân đối và kết hợp các lực với trọng số tương ứng
    dx = dx * PLAYER_ATTRACTION_WEIGHT +  // Lực cơ bản hướng về người chơi
         sepX * SEPARATION_WEIGHT +       // Lực đẩy xa để tách biệt
         aliX * ALIGNMENT_WEIGHT +        // Lực căn chỉnh hướng di chuyển
         cohX * COHESION_WEIGHT;          // Lực tụ về trung tâm nhóm

    dy = dy * PLAYER_ATTRACTION_WEIGHT +
         sepY * SEPARATION_WEIGHT +
         aliY * ALIGNMENT_WEIGHT +
         cohY * COHESION_WEIGHT;
}

void Zombie::Separate(const std::vector<Zombie*>& zombies, float& dx, float& dy) {
    dx = dy = 0;
    int count = 0;

    for (auto other : zombies) {
        if (other != this && !other->isDead) {
            float distX = x - other->x;
            float distY = y - other->y;
            float distance = std::sqrt(distX * distX + distY * distY);

            if (distance < MIN_SEPARATION) {
                // Thêm lực đẩy tỷ lệ nghịch với khoảng cách
                // Zombie càng gần nhau, lực đẩy càng mạnh
                dx += distX / (distance + 1);  // +1 để tránh chia cho 0
                dy += distY / (distance + 1);
                count++;
            }
        }
    }

    // Tính trung bình lực tách biệt
    if (count > 0) {
        dx /= count;
        dy /= count;
        // Chuẩn hóa vector
        float length = std::sqrt(dx * dx + dy * dy);
        if (length > 0) {
            dx /= length;
            dy /= length;
        }
    }
}

void Zombie::Align(const std::vector<Zombie*>& zombies, float& dx, float& dy) {
    dx = dy = 0;
    int count = 0;

    for (auto other : zombies) {
        if (other != this && !other->isDead) {
            float distX = other->x - x;
            float distY = other->y - y;
            float distance = std::sqrt(distX * distX + distY * distY);

            if (distance < NEIGHBOR_RADIUS) {
                // Lấy hướng di chuyển đã chuẩn hóa của zombie khác
                // để căn chỉnh hướng di chuyển của zombie hiện tại
                float otherDx = other->hitbox.x - other->GetX();
                float otherDy = other->hitbox.y - other->GetY();
                float length = std::sqrt(otherDx * otherDx + otherDy * otherDy);
                if (length > 0) {
                    dx += otherDx / length;
                    dy += otherDy / length;
                    count++;
                }
            }
        }
    }

    // Tính trung bình hướng di chuyển của các zombie lân cận
    if (count > 0) {
        dx /= count;
        dy /= count;
        float length = std::sqrt(dx * dx + dy * dy);
        if (length > 0) {
            dx /= length;
            dy /= length;
        }
    }
}

void Zombie::Cohere(const std::vector<Zombie*>& zombies, float& dx, float& dy) {
    float centerX = 0, centerY = 0;
    int count = 0;

    for (auto other : zombies) {
        if (other != this && !other->isDead) {
            float distX = other->x - x;
            float distY = other->y - y;
            float distance = std::sqrt(distX * distX + distY * distY);

            if (distance < NEIGHBOR_RADIUS) {
                centerX += other->x;
                centerY += other->y;
                count++;
            }
        }
    }

    if (count > 0) {
        // Tính toán trung tâm khối lượng (center of mass)
        centerX /= count;
        centerY /= count;
        
        // Tìm hướng đến trung tâm khối lượng
        dx = centerX - x;
        dy = centerY - y;
        
        // Chuẩn hóa vector
        float length = std::sqrt(dx * dx + dy * dy);
        if (length > 0) {
            dx /= length;
            dy /= length;
        }
    }
}

void Zombie::Reset(float newX, float newY, float speedMultiplier) {
    x = newX;
    y = newY;
    rotation = 0.0f;
    health = WaveConfig::ZOMBIE_BASE_HEALTH;
    isDead = false;
    speed = WaveConfig::ZOMBIE_BASE_SPEED * speedMultiplier;
    isAttacking = false;
    lastAttackTime = 0;
    currentFrame = 0;
    frameTimer = 0.0f;
    knockbackVelocityX = 0.0f;
    knockbackVelocityY = 0.0f;
    knockbackDuration = 0.0f;
    
    // Đặt lại vị trí hitbox
    hitbox.x = static_cast<int>(x - hitbox.w / 2);
    hitbox.y = static_cast<int>(y - hitbox.h / 2);
}
