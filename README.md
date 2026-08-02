# 3D Güneş Sistemi Simülasyonu

**Solar System Engine**, C++ ve OpenGL ile geliştirilen nesne yönelimli bir 3D grafik ve simülasyon motorudur. Proje, hazır bir oyun motoru kullanmadan CPU tarafındaki uygulama mantığını GPU tarafındaki GLSL shader programlarıyla birleştirerek etkileşimli bir Güneş Sistemi sahnesi oluşturmayı hedefler.

## Amaç

Bu proje ile aşağıdaki temel grafik programlama kavramları uygulanır:

- GLFW ile pencere, OpenGL context ve girdi yönetimi
- GLAD ile modern OpenGL fonksiyonlarının yüklenmesi
- GLSL vertex ve fragment shader programları
- Vertex verisinin GPU'ya VAO/VBO üzerinden gönderilmesi
- Model, View ve Projection (MVP) matrisleri
- Kamera, yörünge ve kendi ekseninde dönüş matematiği
- Texture ve Phong aydınlatma

## Mevcut durum

- OpenGL 3.3 Core pencere altyapısı hazır.
- `Shader` sınıfı shader dosyalarını yükler, derler ve bağlar.
- `Mesh` sınıfı vertex verisini GPU kaynaklarıyla yönetir.
- Temel GLSL shader'ları ve turuncu üçgen çizim hattı çalışır.

## Mimari

```text
Klavye / Fare
      |
      v
C++ uygulama mantığı (CPU)
      |
      v
Model / View / Projection matrisleri
      |
      v
OpenGL + GLSL (GPU)
      |
      v
Ekran çıktısı
```

`Planet` sınıfı yalnızca simülasyon durumunu ve dönüş matrisini hesaplayacaktır. OpenGL kaynaklarının yönetimi `Shader`, `Mesh`, ileride de `Texture` ve `Renderer` sınıflarında kalacaktır.

## Klasör yapısı

```text
source/
├── main.cpp
├── app/                 # Uygulama ömrü ve game loop
├── graphics/
│   ├── Mesh.h
│   ├── Mesh.cpp
│   ├── Shader.h
│   └── Shader.cpp
└── world/               # Planet ve SolarSystem sınıfları

shaders/
├── basic.vert
└── basic.frag

assets/
└── textures/
```

## Teknolojiler

- C++17
- OpenGL 3.3 Core
- GLFW
- GLAD
- GLM
- CMake
- vcpkg

## Derleme

Ön koşul olarak GLFW, GLAD ve GLM'nin vcpkg ile `x64-windows` triplet'ine kurulmuş olması gerekir.

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug
```

Uygulama Debug modunda çalıştırıldığında CMake, gerekli `glfw3.dll` dosyasını çalıştırılabilir dosyanın yanına kopyalar.

## Yol haritası

1. Shader ve mesh altyapısı
2. Kamera sistemi: WASD ve fare kontrolü
3. Küre mesh üretimi
4. `Planet` sınıfı ve yörünge matematiği
5. Güneş, Dünya ve Ay
6. Texture sistemi ve Phong ışıklandırma
7. Diğer gezegenler, yörünge çizgileri ve görsel iyileştirmeler

## Kontroller

- `ESC`: Uygulamayı kapatır
- WASD ve fare kontrolleri kamera sistemi eklendiğinde kullanılacaktır.
