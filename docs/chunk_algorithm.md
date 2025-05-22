# THUẬT TOÁN HỆ THỐNG CHUNK & TILEMAP

## 1. GIỚI THIỆU

Tài liệu này mô tả chi tiết các thuật toán được sử dụng trong hệ thống Chunk và TileMap của game, bao gồm:
- Thuật toán quản lý và tải Chunk theo vùng
- Thuật toán hiển thị TileMap hiệu quả
- Thuật toán tải bất đồng bộ
- Thuật toán tối ưu hóa hiệu suất

## 2. THUẬT TOÁN CHUNKMANAGER

### 2.1. Tổng quan ChunkManager

ChunkManager chia toàn bộ bản đồ thành các ô vuông (chunk) có kích thước cố định. Tại mỗi thời điểm, chỉ những chunk nằm trong tầm nhìn của người chơi mới được tải và hiển thị, tối ưu hóa bộ nhớ và hiệu suất.

#### 2.1.1. Các thuật ngữ chính:

- **Chunk**: Một phần của bản đồ thế giới, thường là hình vuông.
- **ChunkCoord**: Tọa độ của chunk trong lưới chunk (khác với tọa độ pixel).
- **ViewDistance**: Số chunk tối đa xung quanh người chơi được tải vào bộ nhớ.
- **Blueprint TileMap**: Mẫu tilemap dùng làm nguyên mẫu cho các chunk mới.

### 2.2. Pseudocode cho các thuật toán chính

#### 2.2.1. Chuyển đổi từ tọa độ thế giới sang tọa độ chunk

```
FUNCTION GetChunkCoordFromWorldPos(worldX, worldY)
    // Kiểm tra nếu chưa có kích thước chunk
    IF chunkWidthPixels == 0 OR chunkHeightPixels == 0
        RETURN {0, 0}
    END IF
    
    // Tính tọa độ chunk dựa trên vị trí trong thế giới
    chunkX = FLOOR(worldX / chunkWidthPixels)
    chunkY = FLOOR(worldY / chunkHeightPixels)
    
    RETURN {chunkX, chunkY}
END FUNCTION
```

#### 2.2.2. Cập nhật danh sách các chunk cần tải/hủy

```
FUNCTION UpdateActiveChunks()
    // Xác định không có player hoặc kích thước chunk không hợp lệ
    IF player == null OR chunkWidthPixels == 0 OR chunkHeightPixels == 0
        RETURN
    END IF
    
    // Lấy tọa độ chunk mới của người chơi
    newPlayerChunkCoord = GetChunkCoordFromWorldPos(player.GetX(), player.GetY())
    
    // Chỉ cập nhật nếu người chơi đã di chuyển sang chunk khác
    IF newPlayerChunkCoord != currentPlayerChunkCoord OR activeChunks.IsEmpty()
        // Cập nhật tọa độ chunk hiện tại của người chơi
        currentPlayerChunkCoord = newPlayerChunkCoord
        
        // Xác định các chunk cần thiết dựa trên vị trí hiện tại và khoảng cách tầm nhìn
        requiredChunks = new Map<ChunkCoord, bool>()
        
        // Duyệt tất cả chunk trong phạm vi tầm nhìn
        FOR yOffset = -viewDistanceChunks TO viewDistanceChunks
            FOR xOffset = -viewDistanceChunks TO viewDistanceChunks
                // Tính tọa độ chunk cần thiết
                requiredCoord = {
                    currentPlayerChunkCoord.x + xOffset,
                    currentPlayerChunkCoord.y + yOffset
                }
                
                // Đánh dấu chunk này là cần thiết
                requiredChunks[requiredCoord] = true
            END FOR
        END FOR
        
        // Tìm và hủy các chunk không còn cần thiết
        toUnload = new Array()
        FOR EACH pair IN activeChunks
            IF NOT requiredChunks.Contains(pair.first)
                // Chunk này không còn cần thiết, thêm vào danh sách hủy
                toUnload.Add(pair.first)
            END IF
        END FOR
        
        // Hủy các chunk không còn cần thiết
        FOR EACH coord IN toUnload
            UnloadChunk(coord.x, coord.y)
        END FOR
        
        // Tải các chunk mới cần thiết
        FOR EACH pair IN requiredChunks
            chunkCoord = pair.first
            
            // Kiểm tra xem chunk đã được tải chưa
            IF NOT activeChunks.Contains(chunkCoord)
                // Tải chunk mới
                LoadChunk(chunkCoord.x, chunkCoord.y)
            END IF
        END FOR
    END IF
END FUNCTION
```

#### 2.2.3. Tải chunk bất đồng bộ

```
FUNCTION LoadChunk(chunkGridX, chunkGridY)
    // Tạo tọa độ chunk
    coord = {chunkGridX, chunkGridY}
    
    // Kiểm tra xem chunk đã tồn tại hoặc đang được tải
    IF activeChunks.Contains(coord) OR loadingTasks.Contains(coord)
        RETURN
    END IF
    
    // Kiểm tra tính hợp lệ của blueprint
    IF blueprintTileMap == null OR chunkWidthPixels == 0 OR chunkHeightPixels == 0
        PRINT "ChunkManager: Không thể tải chunk, blueprint chưa sẵn sàng."
        RETURN
    END IF
    
    // Tạo nhiệm vụ bất đồng bộ để tải chunk
    loadingTasks[coord] = ASYNC_FUNCTION()
        // Tạo TileMap mới cho chunk
        newChunk = new TileMap(renderer)
        
        // Đánh dấu thành công
        success = true
        
        // Tải tileset
        IF NOT newChunk.LoadTileset(baseTilesetPath)
            PRINT "Thread: Không thể tải tileset cho chunk (" + chunkGridX + "," + chunkGridY + ")"
            success = false
        END IF
        
        // Tải dữ liệu map
        IF success AND NOT newChunk.LoadMap(baseMapPath)
            PRINT "Thread: Không thể tải dữ liệu bản đồ cho chunk (" + chunkGridX + "," + chunkGridY + ")"
            success = false
        END IF
        
        // Nếu tải thành công, đưa chunk vào danh sách chờ kích hoạt
        IF success
            LOCK readyChunksMutex
                readyToActivateChunks.Add({coord, newChunk})
            UNLOCK readyChunksMutex
        ELSE
            // Giải phóng chunk không thành công
            DELETE newChunk
        END IF
    END ASYNC_FUNCTION
END FUNCTION
```

#### 2.2.4. Xử lý các chunk đã tải xong

```
FUNCTION ProcessReadyChunks()
    // Danh sách chunk cần kích hoạt ngay
    toActivateNow = new Array()
    
    // Lấy danh sách các chunk đã sẵn sàng (thread-safe)
    LOCK readyChunksMutex
        IF NOT readyToActivateChunks.IsEmpty()
            // Hoán đổi với danh sách trống để xử lý an toàn
            SWAP(toActivateNow, readyToActivateChunks)
        END IF
    UNLOCK readyChunksMutex
    
    // Xử lý từng chunk đã sẵn sàng
    FOR EACH pair IN toActivateNow
        coord = pair.first
        chunk = pair.second
        
        // Xóa khỏi danh sách đang tải
        loadingTasks.Remove(coord)
        
        // Kiểm tra xem chunk này còn cần thiết không
        stillNeeded = false
        
        // Kiểm tra xem chunk có thuộc vùng tầm nhìn hiện tại
        FOR yOffset = -viewDistanceChunks TO viewDistanceChunks
            FOR xOffset = -viewDistanceChunks TO viewDistanceChunks
                IF currentPlayerChunkCoord.x + xOffset == coord.x AND 
                   currentPlayerChunkCoord.y + yOffset == coord.y
                    stillNeeded = true
                    BREAK
                END IF
            END FOR
            IF stillNeeded THEN BREAK
        END FOR
        
        // Xử lý chunk dựa trên nhu cầu
        IF stillNeeded
            // Kiểm tra xem chunk đã tồn tại chưa
            IF NOT activeChunks.Contains(coord)
                // Thêm chunk vào danh sách active
                activeChunks[coord] = chunk
            ELSE
                // Chunk đã tồn tại, giải phóng bản mới
                DELETE chunk
            END IF
        ELSE
            // Chunk không còn cần thiết, giải phóng
            DELETE chunk
        END IF
    END FOR
    
    // Kiểm tra trạng thái các nhiệm vụ đang tải
    FOR iterator = loadingTasks.begin() TO loadingTasks.end()
        future = iterator.second
        
        // Kiểm tra xem nhiệm vụ đã hoàn thành chưa
        IF future.IsValid() AND future.WaitFor(0 seconds) == READY
            // Nhiệm vụ đã hoàn thành, xử lý kết quả/lỗi
            TRY
                future.Get()  // Lấy kết quả để xóa trạng thái future
            CATCH exception
                PRINT "Lỗi từ nhiệm vụ tải chunk (" + iterator.first.x + "," + 
                      iterator.first.y + "): " + exception.message
            END TRY
            
            // Xóa nhiệm vụ khỏi danh sách
            iterator = loadingTasks.Erase(iterator)
        ELSE
            // Nhiệm vụ chưa hoàn thành, chuyển sang nhiệm vụ tiếp theo
            iterator++
        END IF
    END FOR
END FUNCTION
```

#### 2.2.5. Hủy chunk

```
FUNCTION UnloadChunk(chunkGridX, chunkGridY)
    // Tạo tọa độ chunk
    coord = {chunkGridX, chunkGridY}
    
    // Tìm chunk trong danh sách active
    chunkIterator = activeChunks.Find(coord)
    
    // Nếu tìm thấy, giải phóng và xóa khỏi danh sách
    IF chunkIterator != activeChunks.end()
        // Giải phóng bộ nhớ của chunk
        DELETE chunkIterator.second
        
        // Xóa khỏi danh sách active
        activeChunks.Erase(chunkIterator)
    END IF
END FUNCTION
```

#### 2.2.6. Hiển thị các chunk

```
FUNCTION Render(camera)
    // Hiển thị từng chunk đang hoạt động
    FOR EACH pair IN activeChunks
        coord = pair.first
        chunk = pair.second
        
        // Tính vị trí của chunk trong thế giới
        worldX = coord.x * chunkWidthPixels
        worldY = coord.y * chunkHeightPixels
        
        // Hiển thị chunk tại vị trí thế giới tương ứng
        chunk.Render(camera, worldX, worldY)
    END FOR
END FUNCTION
```

## 3. THUẬT TOÁN TILEMAP

### 3.1. Tổng quan TileMap

TileMap quản lý việc hiển thị một lưới các ô vuông (tile) từ một tileset. Mỗi ô trong lưới được gán một giá trị (tile ID) tương ứng với một phần cụ thể của tileset.

#### 3.1.1. Các thành phần chính:

- **Tiles**: Mảng 2D lưu trữ ID của các tile.
- **Tileset**: Texture chứa tất cả các loại tile.
- **TilesetRects**: Danh sách các rectangle nguồn cho các tile.

### 3.2. Pseudocode cho các thuật toán chính

#### 3.2.1. Tải dữ liệu tilemap từ file CSV

```
FUNCTION LoadMap(filePath)
    // Mở file CSV
    file = OpenFile(filePath)
    IF file == null
        PRINT "Không thể mở file map: " + filePath
        RETURN false
    END IF
    
    // Đọc dữ liệu
    lines = new Array()
    WHILE NOT file.IsEndOfFile()
        line = file.ReadLine()
        IF NOT line.IsEmpty()
            lines.Add(line)
        END IF
    END WHILE
    
    // Đóng file
    file.Close()
    
    // Xác định kích thước map
    IF lines.IsEmpty()
        PRINT "File map rỗng!"
        RETURN false
    END IF
    
    // Phân tích dòng đầu tiên để xác định số cột
    firstLine = lines[0]
    columns = CountValues(firstLine, separator: ',')
    
    // Xác định số hàng
    rows = lines.Size()
    
    // Khởi tạo map với kích thước phù hợp
    map = new Array[rows][columns]
    
    // Phân tích mỗi dòng
    FOR y = 0 TO rows - 1
        // Phân tách các giá trị bởi dấu phẩy
        values = SplitString(lines[y], separator: ',')
        
        // Đảm bảo đúng số cột
        FOR x = 0 TO MIN(values.Size(), columns) - 1
            // Chuyển đổi chuỗi thành số
            TRY
                tileID = ConvertToInteger(values[x])
                map[y][x] = tileID
            CATCH
                PRINT "Lỗi chuyển đổi giá trị tile: " + values[x]
                map[y][x] = -1  // Giá trị mặc định (không có tile)
            END TRY
        END FOR
    END FOR
    
    // Cập nhật kích thước map
    mapWidth = columns
    mapHeight = rows
    
    RETURN true
END FUNCTION
```

#### 3.2.2. Tính toán trước các rectangle nguồn

```
FUNCTION PrecomputeTilesetRects()
    // Xác định số hàng trong tileset
    tilesetRows = 0
    
    IF tileset != null
        // Lấy kích thước của tileset
        SDL_QueryTexture(tileset, null, null, tilesetWidth, tilesetHeight)
        
        // Tính số hàng dựa trên kích thước
        tilesetRows = tilesetHeight / tileHeight
    END IF
    
    // Xóa danh sách cũ
    tilesetRects.Clear()
    
    // Tính toán rectangle nguồn cho mỗi tile ID
    FOR id = 0 TO tilesetCols * tilesetRows - 1
        // Tính vị trí của tile trong tileset
        srcX = (id % tilesetCols) * tileWidth
        srcY = (id / tilesetCols) * tileHeight
        
        // Thêm rectangle vào danh sách
        tilesetRects.Add({srcX, srcY, tileWidth, tileHeight})
    END FOR
END FUNCTION
```

#### 3.2.3. Hiển thị tilemap với tối ưu hóa culling

```
FUNCTION Render(camera, worldOffsetX, worldOffsetY)
    // Lấy vị trí camera
    cameraX = camera.GetX()
    cameraY = camera.GetY()
    
    // Lấy kích thước cửa sổ xem
    windowWidth = camera.GetWidth()
    windowHeight = camera.GetHeight()
    
    // Tính tile bắt đầu và kết thúc để chỉ render những tile nằm trong tầm nhìn
    startTileX = MAX(0, FLOOR((cameraX - worldOffsetX) / tileWidth))
    startTileY = MAX(0, FLOOR((cameraY - worldOffsetY) / tileHeight))
    endTileX = MIN(mapWidth - 1, CEIL((cameraX + windowWidth - worldOffsetX) / tileWidth))
    endTileY = MIN(mapHeight - 1, CEIL((cameraY + windowHeight - worldOffsetY) / tileHeight))
    
    // Render từng tile trong tầm nhìn
    FOR y = startTileY TO endTileY
        FOR x = startTileX TO endTileX
            // Lấy ID của tile tại vị trí (x, y)
            tileID = map[y][x]
            
            // Bỏ qua các tile trống
            IF tileID >= 0
                // Tính vị trí hiển thị trên màn hình
                destX = worldOffsetX + x * tileWidth - cameraX
                destY = worldOffsetY + y * tileHeight - cameraY
                
                // Kiểm tra tính hợp lệ của tileID
                IF tileID < tilesetRects.Size()
                    // Tạo rectangle đích
                    destRect = {destX, destY, tileWidth, tileHeight}
                    
                    // Lấy rectangle nguồn
                    srcRect = tilesetRects[tileID]
                    
                    // Hiển thị tile
                    SDL_RenderCopy(renderer, tileset, srcRect, destRect)
                END IF
            END IF
        END FOR
    END FOR
END FUNCTION
```

## 4. THUẬT TOÁN TẢI MỘT PHẦN DỮ LIỆU TỪ FILE

### 4.1. Đọc dữ liệu tile cho một chunk cụ thể từ file CSV lớn

```
FUNCTION LoadTileDataForChunk(chunkCoord)
    // Kích thước của một chunk (số lượng tile)
    chunkTileCount = CHUNK_SIZE * CHUNK_SIZE
    
    // Khởi tạo mảng lưu trữ dữ liệu với giá trị mặc định -1 (không có tile)
    tileData = new Array(size: chunkTileCount, initialValue: -1)
    
    // Mở file CSV
    file = OpenFile(mapFilePath)
    IF file == null
        PRINT "Không thể mở file map: " + mapFilePath
        RETURN tileData
    END IF
    
    // Tính vị trí bắt đầu của chunk trong file CSV
    startRow = chunkCoord.y * CHUNK_SIZE
    startCol = chunkCoord.x * CHUNK_SIZE
    
    // Đọc từng dòng CSV
    currentRow = 0
    line = ""
    
    // Bỏ qua các dòng trước vị trí bắt đầu của chunk
    WHILE currentRow < startRow AND file.ReadLine(line)
        currentRow++
    END WHILE
    
    // Đọc các dòng thuộc chunk
    FOR y = 0 TO CHUNK_SIZE - 1
        // Đọc dòng tiếp theo
        IF NOT file.ReadLine(line)
            BREAK  // Hết file
        END IF
        
        // Phân tách các giá trị bởi dấu phẩy
        values = SplitString(line, separator: ',')
        
        // Bỏ qua các cột trước vị trí bắt đầu của chunk
        startIndex = startCol
        
        // Đọc các cột thuộc chunk
        FOR x = 0 TO CHUNK_SIZE - 1
            colIndex = startIndex + x
            
            // Kiểm tra giới hạn
            IF colIndex < values.Size()
                // Chuyển đổi chuỗi thành số
                TRY
                    tileID = ConvertToInteger(values[colIndex])
                    tileData[y * CHUNK_SIZE + x] = tileID
                CATCH
                    PRINT "Lỗi chuyển đổi giá trị tile: " + values[colIndex]
                END TRY
            END IF
        END FOR
        
        currentRow++
    END FOR
    
    // Đóng file
    file.Close()
    
    RETURN tileData
END FUNCTION
```

## 5. TỐI ƯU HÓA HIỆU SUẤT

### 5.1. Kỹ thuật tối ưu hóa

#### 5.1.1. Frustum Culling (Loại bỏ đối tượng ngoài tầm nhìn)

```
FUNCTION IsChunkVisible(chunkCoord, camera)
    // Tính vị trí và kích thước của chunk trong không gian thế giới
    chunkWorldX = chunkCoord.x * CHUNK_SIZE * TILE_SIZE
    chunkWorldY = chunkCoord.y * CHUNK_SIZE * TILE_SIZE
    chunkWidth = CHUNK_SIZE * TILE_SIZE
    chunkHeight = CHUNK_SIZE * TILE_SIZE
    
    // Tạo rectangle cho chunk
    chunkRect = {chunkWorldX, chunkWorldY, chunkWidth, chunkHeight}
    
    // Tạo rectangle cho camera
    cameraRect = {
        camera.GetX(),
        camera.GetY(),
        camera.GetWidth(),
        camera.GetHeight()
    }
    
    // Kiểm tra giao nhau
    RETURN CheckIntersection(chunkRect, cameraRect)
END FUNCTION
```

#### 5.1.2. Tile Visibility Optimization (Chỉ hiển thị các tile trong tầm nhìn)

Đã được triển khai trong thuật toán Render() của TileMap.

#### 5.1.3. Precomputed Source Rectangles (Cache các rectangle nguồn)

Đã được triển khai trong thuật toán PrecomputeTilesetRects() của TileMap.

#### 5.1.4. Asynchronous Loading (Tải bất đồng bộ)

Đã được triển khai trong thuật toán LoadChunk() của ChunkManager.

#### 5.1.5. Spatial Partitioning (Phân chia không gian)

Toàn bộ hệ thống chunk dựa trên nguyên tắc phân chia không gian.

### 5.2. Phân tích độ phức tạp thuật toán

#### 5.2.1. ChunkManager

- **GetChunkCoordFromWorldPos**: O(1) - Phép tính đơn giản
- **UpdateActiveChunks**: O(v²) - với v là viewDistanceChunks
- **LoadChunk**: O(1) - Khởi tạo tác vụ bất đồng bộ
- **ProcessReadyChunks**: O(r + t) - với r là số chunk sẵn sàng, t là số tác vụ đang tải
- **Render**: O(c) - với c là số chunk đang hoạt động

#### 5.2.2. TileMap

- **LoadMap**: O(W * H) - với W, H là chiều rộng và chiều cao của map
- **PrecomputeTilesetRects**: O(N) - với N là số tile trong tileset
- **Render**: O((ex-sx) * (ey-sy)) - với sx, sy, ex, ey là các chỉ số tile bắt đầu và kết thúc

### 5.3. So sánh với các phương pháp khác

| Phương pháp | Ưu điểm | Nhược điểm |
|-------------|---------|------------|
| **Single Large Map** | Đơn giản, dễ triển khai | Tiêu tốn bộ nhớ, kém hiệu quả với bản đồ lớn |
| **Chunk-based Loading** | Tiết kiệm bộ nhớ, hiệu quả với bản đồ lớn | Phức tạp hơn, cần quản lý tải/hủy chunk |
| **Dynamic Level of Detail** | Tối ưu hóa cao cho khoảng cách xa | Rất phức tạp, không phù hợp với tất cả loại game |

## 6. THUẬT TOÁN MỞ RỘNG BẢN ĐỒ VÔ HẠN

### 6.1. Pseudocode cho hệ thống tạo map procedural

```
FUNCTION GenerateChunkData(chunkCoord)
    // Khởi tạo bộ sinh số ngẫu nhiên với seed dựa trên tọa độ chunk
    seed = Hash(chunkCoord.x, chunkCoord.y)
    random = new RandomGenerator(seed)
    
    // Khởi tạo dữ liệu tile cho chunk
    tileData = new Array(size: CHUNK_SIZE * CHUNK_SIZE)
    
    // Tạo dữ liệu Perlin Noise cho terrain
    noise = GeneratePerlinNoise(chunkCoord, CHUNK_SIZE)
    
    // Áp dụng luật sinh terrain dựa trên noise
    FOR y = 0 TO CHUNK_SIZE - 1
        FOR x = 0 TO CHUNK_SIZE - 1
            index = y * CHUNK_SIZE + x
            noiseValue = noise[index]
            
            // Lựa chọn loại tile dựa trên giá trị noise
            IF noiseValue < 0.3
                tileData[index] = WATER_TILE_ID  // Nước
            ELSE IF noiseValue < 0.4
                tileData[index] = SAND_TILE_ID   // Cát
            ELSE IF noiseValue < 0.7
                tileData[index] = GRASS_TILE_ID  // Cỏ
            ELSE
                tileData[index] = MOUNTAIN_TILE_ID  // Núi
            END IF
            
            // Thêm chi tiết ngẫu nhiên (cây, đá, v.v.)
            IF tileData[index] == GRASS_TILE_ID AND random.Next(0, 100) < 10
                tileData[index] = TREE_TILE_ID  // 10% cơ hội sinh cây trên cỏ
            END IF
        END FOR
    END FOR
    
    RETURN tileData
END FUNCTION
```

### 6.2. Pseudocode cho thuật toán tải chunk động

```
FUNCTION LoadOrGenerateChunkData(chunkCoord)
    // Kiểm tra xem chunk có tồn tại trong file không
    IF FileExists(GetChunkFileName(chunkCoord))
        // Nếu có, tải từ file
        RETURN LoadTileDataForChunk(chunkCoord)
    ELSE
        // Nếu không, tạo procedural
        tileData = GenerateChunkData(chunkCoord)
        
        // Tùy chọn: Lưu chunk mới tạo vào file
        SaveChunkData(chunkCoord, tileData)
        
        RETURN tileData
    END IF
END FUNCTION
```

## 7. KẾT LUẬN

Hệ thống Chunk và TileMap trong game đã triển khai nhiều kỹ thuật tối ưu hóa quan trọng:

1. **Chunking**: Phân chia thế giới thành các phần nhỏ để tải/hủy động.
2. **Visibility Culling**: Chỉ hiển thị những gì người chơi có thể nhìn thấy.
3. **Asynchronous Loading**: Tải dữ liệu ở các thread khác để tránh giật hình.
4. **Caching**: Tính toán trước và lưu các giá trị thường xuyên sử dụng.
5. **Partial File Reading**: Chỉ đọc phần dữ liệu cần thiết từ file lớn.

Kết hợp các kỹ thuật này, hệ thống có thể xử lý bản đồ rất lớn hoặc thậm chí vô hạn mà vẫn duy trì hiệu suất tốt trên nhiều cấu hình máy khác nhau.
