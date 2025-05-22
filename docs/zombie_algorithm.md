# THUẬT TOÁN HỆ THỐNG ZOMBIE, WAVE & ZOMBIE POOL

## 1. GIỚI THIỆU

Tài liệu này giải thích chi tiết các thuật toán được sử dụng trong hệ thống zombie, bao gồm:
- Thuật toán AI Boids (Flocking Behavior) cho zombie
- Thuật toán quản lý wave zombie
- Thuật toán Object Pooling cho quản lý bộ nhớ hiệu quả

## 2. THUẬT TOÁN FLOCKING (BOIDS) CHO ZOMBIE

### 2.1. Tổng quan thuật toán

Thuật toán Boids, được phát triển bởi Craig Reynolds vào năm 1986, mô phỏng hành vi đàn của các sinh vật. Trong game, thuật toán này được áp dụng để tạo hành vi nhóm tự nhiên cho zombie, giúp chúng di chuyển theo đàn với các hành vi như tách biệt, căn chỉnh và kết dính.

#### 2.1.1. Thành phần cơ bản của thuật toán Boids:

1. **Separation (Tách biệt)**: Tránh va chạm với các zombie lân cận.
2. **Alignment (Căn chỉnh)**: Di chuyển theo hướng trung bình của các zombie lân cận.
3. **Cohesion (Kết dính)**: Di chuyển về phía trung tâm của các zombie lân cận.
4. **Hướng về mục tiêu chính (người chơi)**: Định hướng đàn về phía người chơi.

### 2.2. Pseudocode chi tiết

#### 2.2.1. Thuật toán tổng quát Flocking

```
FUNCTION ApplyFlockingBehavior(zombies, baseDirectionX, baseDirectionY)
    // Khởi tạo các vector lực
    separationX = 0, separationY = 0
    alignmentX = 0, alignmentY = 0
    cohesionX = 0, cohesionY = 0
    
    // Tính toán từng lực riêng biệt
    Separate(zombies, separationX, separationY)
    Align(zombies, alignmentX, alignmentY)
    Cohere(zombies, cohesionX, cohesionY)
    
    // Kết hợp các lực với trọng số tương ứng
    directionX = baseDirectionX * PLAYER_ATTRACTION_WEIGHT + 
                 separationX * SEPARATION_WEIGHT + 
                 alignmentX * ALIGNMENT_WEIGHT + 
                 cohesionX * COHESION_WEIGHT
                 
    directionY = baseDirectionY * PLAYER_ATTRACTION_WEIGHT + 
                 separationY * SEPARATION_WEIGHT + 
                 alignmentY * ALIGNMENT_WEIGHT + 
                 cohesionY * COHESION_WEIGHT
                 
    // Trả về hướng mới đã tính
    RETURN directionX, directionY
END FUNCTION
```

#### 2.2.2. Thuật toán Separation (Tách biệt)

```
FUNCTION Separate(zombies, outputX, outputY)
    // Khởi tạo
    outputX = 0
    outputY = 0
    count = 0
    
    // Kiểm tra tất cả zombie khác
    FOR EACH otherZombie IN zombies
        IF otherZombie != thisZombie AND NOT otherZombie.isDead
            // Tính khoảng cách
            distX = x - otherZombie.x
            distY = y - otherZombie.y
            distance = SQRT(distX^2 + distY^2)
            
            // Chỉ áp dụng lực tách cho zombie quá gần
            IF distance < MIN_SEPARATION
                // Lực tách tỉ lệ nghịch với khoảng cách
                outputX += distX / (distance + 1)  // +1 để tránh chia cho 0
                outputY += distY / (distance + 1)
                count++
            END IF
        END IF
    END FOR
    
    // Tính trung bình và chuẩn hóa vector lực
    IF count > 0
        outputX /= count
        outputY /= count
        
        // Chuẩn hóa thành vector đơn vị
        length = SQRT(outputX^2 + outputY^2)
        IF length > 0
            outputX /= length
            outputY /= length
        END IF
    END IF
END FUNCTION
```

#### 2.2.3. Thuật toán Alignment (Căn chỉnh)

```
FUNCTION Align(zombies, outputX, outputY)
    // Khởi tạo
    outputX = 0
    outputY = 0
    count = 0
    
    // Kiểm tra tất cả zombie khác
    FOR EACH otherZombie IN zombies
        IF otherZombie != thisZombie AND NOT otherZombie.isDead
            // Tính khoảng cách
            distX = otherZombie.x - x
            distY = otherZombie.y - y
            distance = SQRT(distX^2 + distY^2)
            
            // Chỉ xét những zombie trong bán kính lân cận
            IF distance < NEIGHBOR_RADIUS
                // Lấy hướng di chuyển của zombie khác
                otherDirX = otherZombie.directionX
                otherDirY = otherZombie.directionY
                
                // Lấy hướng di chuyển chuẩn hóa
                otherLength = SQRT(otherDirX^2 + otherDirY^2)
                IF otherLength > 0
                    outputX += otherDirX / otherLength
                    outputY += otherDirY / otherLength
                    count++
                END IF
            END IF
        END IF
    END FOR
    
    // Tính trung bình và chuẩn hóa
    IF count > 0
        outputX /= count
        outputY /= count
        
        length = SQRT(outputX^2 + outputY^2)
        IF length > 0
            outputX /= length
            outputY /= length
        END IF
    END IF
END FUNCTION
```

#### 2.2.4. Thuật toán Cohesion (Kết dính)

```
FUNCTION Cohere(zombies, outputX, outputY)
    // Khởi tạo
    centerX = 0
    centerY = 0
    count = 0
    outputX = 0
    outputY = 0
    
    // Tìm trung tâm của đàn zombie lân cận
    FOR EACH otherZombie IN zombies
        IF otherZombie != thisZombie AND NOT otherZombie.isDead
            // Tính khoảng cách
            distX = otherZombie.x - x
            distY = otherZombie.y - y
            distance = SQRT(distX^2 + distY^2)
            
            // Chỉ xét trong bán kính lân cận
            IF distance < NEIGHBOR_RADIUS
                // Cộng dồn vị trí
                centerX += otherZombie.x
                centerY += otherZombie.y
                count++
            END IF
        END IF
    END FOR
    
    // Tính vị trí trung tâm và hướng đến đó
    IF count > 0
        // Tính điểm trung tâm
        centerX /= count
        centerY /= count
        
        // Hướng từ zombie đến trung tâm
        outputX = centerX - x
        outputY = centerY - y
        
        // Chuẩn hóa vector
        length = SQRT(outputX^2 + outputY^2)
        IF length > 0
            outputX /= length
            outputY /= length
        END IF
    END IF
END FUNCTION
```

### 2.3. Tối ưu hóa thuật toán Flocking

- **Bán kính láng giềng có giới hạn**: Chỉ xét các zombie trong phạm vi `NEIGHBOR_RADIUS`, giảm độ phức tạp từ O(n²) xuống còn O(k*n) với k là số lượng zombie trung bình trong phạm vi.
- **Cân bằng các trọng số**: Các tham số `SEPARATION_WEIGHT`, `ALIGNMENT_WEIGHT`, `COHESION_WEIGHT` được tinh chỉnh để tạo hành vi tự nhiên.
- **Tối ưu hóa tính toán khoảng cách**: Có thể sử dụng khoảng cách bình phương để tránh phép tính căn bậc hai tốn kém trong vòng lặp.

## 3. THUẬT TOÁN WAVEMANAGER

### 3.1. Tổng quan Wave Manager

WaveManager điều khiển việc sinh zombie theo đợt (wave) với các đặc điểm:
- Mỗi wave có số lượng zombie tăng dần
- Zombie trở nên mạnh hơn theo thời gian (máu, tốc độ, sát thương)
- Sinh zombie theo nhóm nhỏ thay vì cùng một lúc
- Boss wave xuất hiện định kỳ

### 3.2. Pseudocode cho WaveManager

#### 3.2.1. Khởi tạo wave mới

```
FUNCTION StartNextWave()
    // Tăng số wave
    currentWave++
    
    // Kiểm tra mở khóa vũ khí mới
    IF currentWave == RIFLE_UNLOCK_WAVE OR currentWave == SHOTGUN_UNLOCK_WAVE
        newWeaponUnlocked = true
    END IF
    
    // Tính toán tham số cho wave mới
    CalculateWaveParameters()
    
    // Thiết lập trạng thái
    waitingForNextWave = false
END FUNCTION
```

#### 3.2.2. Tính toán tham số wave

```
FUNCTION CalculateWaveParameters()
    // Tính số lượng zombie cho wave này
    zombiesToSpawn = BASE_ZOMBIES_PER_WAVE + (currentWave - 1) * ZOMBIES_INCREASE_PER_WAVE
    
    // Giới hạn số lượng tối đa
    zombiesToSpawn = MIN(zombiesToSpawn, MAX_ZOMBIES_PER_WAVE)
    
    // Nếu là boss wave, tăng số lượng
    IF currentWave % BOSS_WAVE_INTERVAL == 0
        zombiesToSpawn = zombiesToSpawn * 1.5
    END IF
    
    // Cập nhật số lượng zombie còn lại
    zombiesRemaining = zombiesToSpawn
    
    // Tính thời gian giữa các lần spawn, giảm dần theo wave
    spawnDelay = MAX(MIN_SPAWN_DELAY, INITIAL_SPAWN_DELAY - (currentWave - 1) * SPAWN_DELAY_DECREASE)
    
    // Khởi tạo timer
    spawnTimer = spawnDelay
    
    // Khởi tạo thông số nhóm zombie
    currentGroupSize = Random(MIN_GROUP_SIZE, MAX_GROUP_SIZE)
    zombiesInCurrentGroup = 0
END FUNCTION
```

#### 3.2.3. Cập nhật trạng thái wave

```
FUNCTION Update(deltaTime)
    IF waitingForNextWave
        // Đếm ngược thời gian đợi giữa các wave
        waveDelayTimer -= deltaTime
        
        // Nếu hết thời gian, bắt đầu wave mới
        IF waveDelayTimer <= 0
            StartNextWave()
        END IF
    ELSE
        // Đếm ngược thời gian giữa các lần sinh zombie
        IF spawnTimer > 0
            spawnTimer -= deltaTime
        END IF
        
        // Kiểm tra xem wave đã kết thúc chưa
        IF zombiesRemaining <= 0 AND zombiesToSpawn <= 0
            // Chuyển sang trạng thái đợi wave tiếp theo
            waitingForNextWave = true
            waveDelayTimer = WAVE_DELAY
        END IF
    END IF
END FUNCTION
```

#### 3.2.4. Kiểm tra điều kiện sinh zombie

```
FUNCTION ShouldSpawnZombie()
    // Nếu đang đợi wave tiếp theo, không sinh zombie
    IF waitingForNextWave
        RETURN false
    END IF
    
    // Nếu không còn zombie cần sinh, không sinh thêm
    IF zombiesToSpawn <= 0
        RETURN false
    END IF
    
    // Kiểm tra thời gian giữa các lần sinh
    IF spawnTimer <= 0
        // Kiểm tra xem đã sinh đủ zombie trong nhóm hiện tại chưa
        IF zombiesInCurrentGroup >= currentGroupSize
            // Nhóm đã đủ, đặt lại thời gian và chọn kích thước nhóm mới
            spawnTimer = spawnDelay
            currentGroupSize = Random(MIN_GROUP_SIZE, MAX_GROUP_SIZE)
            zombiesInCurrentGroup = 0
            RETURN false
        END IF
        
        // Còn zombie trong nhóm hiện tại, có thể sinh
        RETURN true
    END IF
    
    RETURN false
END FUNCTION
```

#### 3.2.5. Cập nhật sau khi sinh zombie

```
FUNCTION OnZombieSpawned()
    // Giảm số lượng zombie còn lại cần sinh
    zombiesToSpawn--
    
    // Giảm số lượng zombie còn lại trong wave
    zombiesRemaining--
    
    // Tăng số lượng zombie đã sinh trong nhóm hiện tại
    zombiesInCurrentGroup++
END FUNCTION
```

### 3.3. Thuật toán tính toán hệ số scaling theo wave

```
FUNCTION GetHealthMultiplier()
    // Tính hệ số máu theo wave
    multiplier = 1.0 + (currentWave - 1) * HEALTH_INCREASE_PER_WAVE
    
    // Giới hạn hệ số tối đa
    multiplier = MIN(multiplier, MAX_HEALTH_MULTIPLIER)
    
    // Boss có máu cao hơn
    IF IsBossWave()
        multiplier *= BOSS_HEALTH_MULTIPLIER
    END IF
    
    RETURN multiplier
END FUNCTION

FUNCTION GetSpeedMultiplier()
    // Tính hệ số tốc độ theo wave
    multiplier = 1.0 + (currentWave - 1) * SPEED_INCREASE_PER_WAVE
    
    // Giới hạn hệ số tối đa
    multiplier = MIN(multiplier, MAX_SPEED_MULTIPLIER)
    
    RETURN multiplier
END FUNCTION
```

## 4. THUẬT TOÁN ZOMBIE POOL

### 4.1 Tổng quan Object Pooling

Object Pooling là kỹ thuật tối ưu hóa bộ nhớ bằng cách tái sử dụng các đối tượng thay vì tạo mới liên tục. Lợi ích:
- Giảm phân mảnh bộ nhớ
- Giảm thời gian tạo/hủy đối tượng
- Tối ưu hiệu năng khi có nhiều đối tượng

### 4.2. Pseudocode Object Pooling cho Zombie

#### 4.2.1. Khởi tạo ZombiePool

```
FUNCTION ZombiePool(renderer, poolSize)
    // Lưu tham số
    this.renderer = renderer
    this.poolSize = poolSize
    
    // Khởi tạo các container
    pool = new Array(capacity: poolSize)
    activeZombies = new Array(capacity: poolSize)
    isInUse = new Array(size: poolSize, initialValue: false)
    
    // Tạo trước các đối tượng zombie
    PrewarmPool()
END FUNCTION

FUNCTION PrewarmPool()
    // Tạo trước tất cả đối tượng zombie trong pool
    FOR i = 0 TO poolSize - 1
        // Tạo zombie mới với vị trí mặc định
        zombie = new Zombie(renderer, 0, 0)
        
        // Đưa về trạng thái không hoạt động
        zombie.Reset(0, 0)
        
        // Thêm vào pool
        pool[i] = zombie
    END FOR
END FUNCTION
```

#### 4.2.2. Lấy zombie từ pool

```
FUNCTION GetZombie()
    // Tìm zombie không hoạt động trong pool
    FOR i = 0 TO poolSize - 1
        IF NOT isInUse[i]
            // Đánh dấu đã sử dụng
            isInUse[i] = true
            
            // Thêm vào danh sách active
            activeZombies.Add(pool[i])
            
            // Trả về zombie
            RETURN pool[i]
        END IF
    END FOR
    
    // Kiểm tra zombie trong hàng đợi tái chế
    IF NOT recycledZombies.Empty()
        zombie = recycledZombies.Dequeue()
        activeZombies.Add(zombie)
        RETURN zombie
    END IF
    
    // Tất cả zombie đang được sử dụng
    // Có thể mở rộng pool hoặc trả về null
    IF pool.Size() < MAX_POOL_SIZE
        // Tạo thêm zombie mới
        AddZombie()
        RETURN GetZombie()
    END IF
    
    // Hoặc tái sử dụng zombie xa nhất
    furthestZombie = FindFurthestZombie()
    IF furthestZombie != null
        ReturnZombie(furthestZombie)
        RETURN GetZombie()
    END IF
    
    RETURN null
END FUNCTION
```

#### 4.2.3. Trả zombie về pool

```
FUNCTION ReturnZombie(zombie)
    // Tìm zombie trong danh sách active
    index = activeZombies.IndexOf(zombie)
    IF index >= 0
        // Xóa khỏi danh sách active
        activeZombies.RemoveAt(index)
        
        // Tìm vị trí trong pool gốc
        FOR i = 0 TO poolSize - 1
            IF pool[i] == zombie
                // Đánh dấu không sử dụng
                isInUse[i] = false
                
                // Đặt lại trạng thái
                zombie.Reset(0, 0)
                
                BREAK
            END IF
        END FOR
    END IF
END FUNCTION
```

#### 4.2.4. Cập nhật Pool

```
FUNCTION Update(deltaTime, player)
    // Cập nhật tất cả zombie đang hoạt động
    FOR i = activeZombies.Size() - 1 TO 0
        zombie = activeZombies[i]
        
        // Cập nhật zombie
        zombie.Update(deltaTime, player, activeZombies)
        
        // Nếu zombie chết, trả về pool
        IF zombie.IsDead()
            ReturnZombie(zombie)
        END IF
    END FOR
    
    // Tối ưu hóa phân bố zombie
    OptimizeZombieDistribution(player)
END FUNCTION
```

#### 4.2.5. Tối ưu hóa phân bố zombie

```
FUNCTION OptimizeZombieDistribution(player)
    // Tái chế zombie quá xa người chơi khi cần thiết
    IF activeZombies.Size() > poolSize * 0.8  // Pool đang sử dụng > 80%
        // Tái chế những zombie xa nhất
        RecycleDistantZombies(player, MIN_RECYCLE_DISTANCE)
    ELSE
        // Tái chế những zombie rất xa
        RecycleDistantZombies(player, RECYCLE_DISTANCE)
    END IF
END FUNCTION

FUNCTION RecycleDistantZombies(player, maxDistance)
    FOR i = activeZombies.Size() - 1 TO 0
        zombie = activeZombies[i]
        
        // Tính khoảng cách đến người chơi
        distanceToPlayer = Distance(zombie, player)
        
        // Nếu quá xa, tái chế
        IF distanceToPlayer > maxDistance
            // Đưa vào hàng đợi tái chế
            recycledZombies.Enqueue(zombie)
            
            // Trả về pool
            ReturnZombie(zombie)
        END IF
    END FOR
END FUNCTION
```

## 5. PHÂN TÍCH HIỆU SUẤT

### 5.1. Độ phức tạp thuật toán Flocking

- **Độ phức tạp thời gian**: O(n²) trong trường hợp xấu nhất, với n là số lượng zombie
- **Tối ưu hóa**: Giảm xuống O(k*n) với k là số zombie trong phạm vi NEIGHBOR_RADIUS

### 5.2. Độ phức tạp thuật toán Wave Manager

- **Độ phức tạp thời gian**: O(1) cho hầu hết các hàm
- **Độ phức tạp không gian**: O(1) - Chỉ lưu trữ một số biến trạng thái

### 5.3. Độ phức tạp thuật toán Object Pooling

- **GetZombie()**: O(n) trong trường hợp xấu nhất, với n là kích thước của pool
- **ReturnZombie()**: O(n) để tìm zombie trong pool
- **Update()**: O(n) với n là số zombie đang hoạt động

### 5.4. Cải tiến hiệu suất

1. **Spatial Partitioning**: Sử dụng cấu trúc dữ liệu không gian như grid hoặc quadtree để giảm độ phức tạp của thuật toán Flocking xuống O(n log n).
2. **Tối ưu hóa Object Pooling**: Sử dụng cấu trúc dữ liệu hiệu quả hơn như linked list hoặc hash table để truy xuất đối tượng nhanh hơn.
3. **Batch Processing**: Xử lý zombie theo nhóm để tối ưu hóa CPU cache và giảm chi phí chuyển đổi ngữ cảnh.

## 6. KẾT LUẬN

Kết hợp ba thuật toán trên, hệ thống zombie trong game cung cấp trải nghiệm chơi game hấp dẫn với:
- Hành vi thông minh và tự nhiên của zombie nhờ thuật toán Flocking
- Độ khó tăng dần và liên tục thông qua hệ thống Wave
- Hiệu suất ổn định ngay cả với số lượng lớn zombie nhờ Object Pooling

Các thuật toán này minh họa cách áp dụng khoa học máy tính và AI trong phát triển game để tạo ra trải nghiệm người chơi hấp dẫn mà vẫn đảm bảo hiệu suất.
