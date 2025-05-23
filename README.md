# Zombie Annihilation: Last Stand (SDL2/C++)
NGUYỄN PHAN VIỆT HƯNG - 24021502

![Ảnh chụp màn hình Game](docs/images/game_preview.png) **Zombie Annihilation: Last Stand** là một trò chơi bắn súng zombie 2D từ trên xuống đầy hành động, được xây dựng bằng C++ và thư viện SDL2. Sống sót qua những đợt zombie bất tận, nâng cấp vũ khí của bạn và chiến đấu để giành điểm số cao nhất trong trải nghiệm arcade.

## 🌟 Tính Năng Nổi Bật

* **Hành Động Kịch Tính Từ Trên Xuống:** Di chuyển trong các môi trường động và tiêu diệt hàng đàn zombie.
* **Sống Sót Theo Đợt:** Đối mặt với những đợt kẻ thù ngày càng thử thách. Bạn có thể trụ được bao lâu?
* **Tiến Trình Người Chơi:** Nguời chơi mở khóa vũ khí sau từng wave
* **Hệ Thống Camera Động:** Camera mượt mà theo sát hành động của người chơi.
* **Bản Đồ Dạng Lưới (Tile-Based Maps):** Khám phá các bản đồ chi tiết được tạo bằng Tiled.
* **Giao Diện Người Dùng:** Bao gồm Menu Chính, Màn Hình Tải và HUD trong game.
* **Hiệu Ứng Âm Thanh:**  Âm thanh sống động để nâng cao trải nghiệm chơi game.

## 🎮 Cách Chơi

Mục tiêu rất đơn giản: **SỐNG SÓT!**
Bạn bị thả vào một khu vực bị zombie chiếm đóng. Các đợt xác sống sẽ tấn công bạn không ngừng.
Sử dụng sự nhanh nhẹn và hỏa lực của bạn để loại bỏ chúng, thu thập vật phẩm hỗ trợ (nếu có) và chuẩn bị cho đợt tấn công tiếp theo.

### Điều Khiển

* **Di Chuyển:**
    * `W`: Đi Lên
    * `A`: Đi Sang Trái 
    * `S`: Đi Xuống
    * `D`: Đi Sang Phải
* **Chiến Đấu:**
    * `Chuột Trái`: Bắn
* **Khác:**
    * `ESC`: Tạm Dừng Game / Mở Menu. 
    * `F11`: Chuyển đổi trạng thái màn hình.
    * `F1` : Bật chế độ debug.
    * `F5` : Bật tắt tiếng.
## 🛠️ Công Nghệ Sử Dụng

* **Ngôn Ngữ:** C++
* **Thư Viện Cốt Lõi:** SDL2
* **Thư Viện Mở Rộng:**
    * SDL2_image (để tải các định dạng ảnh khác nhau)
    * SDL2_ttf (để hiển thị văn bản và các yếu tố UI)
    * SDL2_mixer (âm thanh bắn súng)
* **Hệ Thống Build:** Makefile 
* **Trình Chỉnh Sửa Bản Đồ (cho thiết kế màn chơi):** Tiled 
## 🚀 Bắt Đầu

Làm theo các hướng dẫn sau để lấy một bản sao của dự án và chạy nó trên máy cục bộ của bạn cho mục đích phát triển và thử nghiệm.

### Yêu Cầu Cần Thiết

Bạn sẽ cần cài đặt các thư viện phát triển SDL2 trên hệ thống của mình:

* **SDL2 (cốt lõi):** `libsdl2-dev`
* **SDL2_image:** `libsdl2-image-dev`
* **SDL2_ttf:** `libsdl2-ttf-dev`
* **SDL2_mixer:** `libsdl2-mixer-dev` (*nếu bạn sử dụng cho âm thanh*)

**Ví dụ cho các hệ thống dựa trên Debian/Ubuntu:**
```bash
sudo apt-get update
sudo apt-get install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-mixer-dev
```
Đối với các hệ thống khác (Windows, macOS), vui lòng tham khảo hướng dẫn cài đặt SDL2 chính thức.

Bạn cũng sẽ cần một trình biên dịch C++ hỗ trợ C++17 (hoặc phiên bản C++ của dự án của bạn) và `make`.

### Cài Đặt & Build

1.  **Sao chép (clone) kho lưu trữ:**
    ```bash
    git clone [URL_KHO_LUU_TRU_GIT_CUA_BAN]
    cd [TEN_THU_MUC_DU_AN_CUA_BAN] # ví dụ: Game-CPP-main
    ```

2.  **Biên dịch trò chơi:**
    Dự án sử dụng Makefile để build.
    ```bash
    make 
    ```
    Nếu make không sử dụng để build được.
    ```bash
    mingw32-make
    ```

3.  **Chạy trò chơi:**
    Sau khi biên dịch thành công, một tệp thực thi sẽ được tạo (ví dụ: `game` hoặc `ZombieAnnihilation`).
    ```bash
    ./game  # Hoặc tên tệp thực thi của bạn
    ```

## 📂 Cấu Trúc Dự Án (Đơn Giản Hóa)

```
Game-CPP-main/
├── assets/             # Tài nguyên game (hình ảnh, bản đồ, font, âm thanh)
│   ├── fonts/
│   ├── images/
│   ├── maps/
│   └── sounds/         # (nếu bạn có file âm thanh)
├── docs/               # Tài liệu, sơ đồ, ảnh chụp màn hình
├── include/            # Các file header cho thư viện SDL (thường nằm trong thư mục src/include của bạn)
├── score/              # File điểm số
├── src/                # Mã nguồn (.cpp và .h)
│   ├── include/        # Các file header của dự án
│   │   ├── Bullet.h
│   │   ├── Camera.h
│   │   ├── ChunkManager.h
│   │   ├── Game.h
│   │   ├── Player.h
│   │   ├── TileMap.h
│   │   ├── UI.h
│   │   ├── WaveManager.h
│   │   ├── Zombie.h
│   │   └── ... (các header khác)
│   ├── *.cpp           # Các file triển khai
│   └── main.cpp        # Điểm vào chính
├── Makefile            # Script build (hoặc file cấu hình build của bạn)
└── README.md           # File này
```


## 🙏 Lời Cảm Ơn

* [Lazy Foo' Productions](http://lazyfoo.net/tutorials/SDL/) vì các hướng dẫn SDL2 tuyệt vời.
* [Tài liệu SDL2](https://wiki.libsdl.org/)
* [Youtube](https://www.youtube.com/watch?v=pj3m3Fu3i5A)
* [AI](Copilot)
---
