#include <cmath>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <vector>

using namespace std::chrono;

float getVal(float x, float y) {
  auto xx = (x - 0.5f);
  auto yy = (y - 0.5f);
  return std::sqrtf(xx * xx + yy * yy);
}

class VectorBuffer {
 public:
  VectorBuffer(int width, int height,
               std::chrono::high_resolution_clock::time_point* o_fillStart)
      : pixels_(width * height), width_(width), height_(height) {
    if (o_fillStart) {
      *o_fillStart = std::chrono::high_resolution_clock::now();
    }
    for (int i = 0; i < width; i++) {
      for (int j = 0; j < height; j++) {
        pixels_[j * width + i] = uint8_t(
            getVal(float(i) / float(width), float(j) / float(height)) * 255u);
      }
    }
  }
  void getData(uint8_t** data, size_t* size) {
    *data = &pixels_[0];
    *size = pixels_.size();
  }

 private:
  std::vector<uint8_t> pixels_;
  int width_;
  int height_;
};

class RawBuffer {
 public:
  RawBuffer(int width, int height,
            high_resolution_clock::time_point* o_fillStart)
      : pixels_(new uint8_t[width * height]), width_(width), height_(height) {
    if (o_fillStart) {
      *o_fillStart = high_resolution_clock::now();
    }
    for (int i = 0; i < width; i++) {
      for (int j = 0; j < height; j++) {
        pixels_[j * width + i] = uint8_t(
            getVal(float(i) / float(width), float(j) / float(height)) * 255u);
      }
    }
  }

  void getData(uint8_t** data, size_t* size) {
    *data = pixels_;
    *size = width_ * height_;
  }

  ~RawBuffer() {
    if (pixels_) {
      delete[] pixels_;
      pixels_ = nullptr;
    }
  }

 private:
  uint8_t* pixels_;
  int width_;
  int height_;
};

void benchmarkVector(int size) {
  auto start = high_resolution_clock::now();
  high_resolution_clock::time_point fillStart, startDtor, finishedCreating;
  uint8_t* data;
  size_t bufferSize;
  {
    VectorBuffer b(size, size, &fillStart);
    finishedCreating = high_resolution_clock::now();
    b.getData(&data, &bufferSize);

    startDtor = high_resolution_clock::now();
  }
  auto finishDtor = high_resolution_clock::now();

  auto msAllocTime = duration_cast<milliseconds>(fillStart - start).count();
  auto msFillTime =
      duration_cast<milliseconds>(finishedCreating - fillStart).count();
  auto destructionTime =
      duration_cast<milliseconds>(finishDtor - startDtor).count();

  std::cout << std::setw(5) << size << "|" << std::setw(14) << bufferSize << "|"
            << std::setw(14) << msAllocTime << "|" << std::setw(14)
            << msFillTime << "|" << std::setw(14) << destructionTime
            << std::endl;
}

void benchmarkRaw(int size) {
  auto start = high_resolution_clock::now();
  high_resolution_clock::time_point fillStart, startDtor, finishedCreating;
  uint8_t* data;
  size_t bufferSize;
  {
    RawBuffer b(size, size, &fillStart);
    finishedCreating = high_resolution_clock::now();
    b.getData(&data, &bufferSize);

    startDtor = high_resolution_clock::now();
  }
  auto finishDtor = high_resolution_clock::now();

  auto msAllocTime = duration_cast<milliseconds>(fillStart - start).count();
  auto msFillTime =
      duration_cast<milliseconds>(finishedCreating - fillStart).count();
  auto destructionTime =
      duration_cast<milliseconds>(finishDtor - startDtor).count();

  std::cout << std::setw(5) << size << "|" << std::setw(14) << bufferSize << "|"
            << std::setw(14) << msAllocTime << "|" << std::setw(14)
            << msFillTime << "|" << std::setw(14) << destructionTime
            << std::endl;
}

int main() {
  std::cout << "---------- std::vector image benchmarks ----------"
            << std::endl;
  std::cout << std::setw(5) << "size"
            << "|" << std::setw(14) << "bytes"
            << "|" << std::setw(14) << "alloc ms"
            << "|" << std::setw(14) << "fill ms"
            << "|" << std::setw(14) << "dtor ms" << std::endl;
  benchmarkVector(256);
  benchmarkVector(512);
  benchmarkVector(1024);
  benchmarkVector(2048);
  benchmarkVector(4096);

  std::cout << "\n\n";

  std::cout << "---------- raw image benchmarks ----------" << std::endl;
  std::cout << std::setw(5) << "size"
            << "|" << std::setw(14) << "bytes"
            << "|" << std::setw(14) << "alloc ms"
            << "|" << std::setw(14) << "fill ms"
            << "|" << std::setw(14) << "dtor ms" << std::endl;
  benchmarkRaw(256);
  benchmarkRaw(512);
  benchmarkRaw(1024);
  benchmarkRaw(2048);
  benchmarkRaw(4096);
}