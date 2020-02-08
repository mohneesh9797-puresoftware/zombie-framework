#ifndef OBS_DATA_MATRIX2D_HPP
#define OBS_DATA_MATRIX2D_HPP

#include <glm/vec2.hpp>
#include <vector>

#include <framework/utility/essentials.hpp>

namespace Obs::Data {

template <typename T>
struct Matrix2D {
    // Leaves data uninitialized!
    explicit Matrix2D(glm::ivec2 size);

    T Get(int x, int y, T default_) const;
    void TrySet(int x, int y, T value);
    void UnsafeSet(int x, int y, T value);

    glm::ivec2 const size;
    std::vector<float> data;
};

template <typename T>
Matrix2D<T>::Matrix2D(glm::ivec2 size) : size(size) {
    zombie_assert(size.x >= 1 && size.y >= 1);

    data.reserve(size.x * size.y);
}

template <typename T>
T Matrix2D<T>::Get(int x, int y, T default_) const {
    if (x >= 0 && x < size.x && y >= 0 && y < size.y) {
        return data[x * size.y + y];
    }
    else {
        return default_;
    }
}

template <typename T>
void Matrix2D<T>::TrySet(int x, int y, T value) {
    if (x >= 0 && x < size.x && y >= 0 && y < size.y) {
        data[x * size.y + y] = value;
    }
}

template <typename T>
void Matrix2D<T>::UnsafeSet(int x, int y, T value) {
    data[x * size.y + y] = value;
}

}

#endif
