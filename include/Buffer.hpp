#ifndef Buffer_HPP
#define Buffer_HPP

#include <vector>
#include <array>

// A header-only type-erased non-owning buffer, similar to c++17's array_view
struct Buffer
{
    void* const data;       // typeless
    const size_t stride;    // in bytes
    const size_t size;      // in bytes
    const size_t num;       // size / stride

    // Main constructor
    Buffer(void* ptr, size_t elementSize, size_t numElements) :
        data(ptr), stride(elementSize), size(elementSize*numElements), num(numElements) { }
    
    // Implicit-casting constructor from vector
    template<class T>
    Buffer(std::vector<T> vector) : Buffer(&vector[0], sizeof(T), vector.size()) { }

    // Implicit-casting constructor from std::array
    template<class T, size_t N>
    Buffer(std::array<T, N> array) : Buffer(&array[0], sizeof(T), N) { }

    // Implicit-casting constructor from raw array
    template<class T, size_t N>
    Buffer(T array[N]) : Buffer(array, sizeof(T), N) { }
    
    // Transform data into another form
    template<class T>
    T* getPointerAs() const { return reinterpret_cast<T*>(data); }
};

#endif
