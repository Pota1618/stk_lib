#include <iostream>
#include <vector>
#include <type_traits>
#include <cassert>

using namespace std;
using ll = long long;

template<typename T, size_t D>
class NdArray {
private:
    vector<NdArray<T, D - 1>> data;
public:
    NdArray() {}
    explicit NdArray(vector<size_t> shape) {
        assert(shape.size() == D);
        size_t n = shape[0];
        shape.erase(shape.begin());
        data = vector<NdArray<T, D - 1>>(n, NdArray<T, D - 1>(shape));
    }
    
    const NdArray<T, D - 1>& operator[](size_t i) const { return data[i]; }
    NdArray<T, D - 1>& operator[](size_t i) { return data[i]; }
};

template<typename T>
class NdArray<T, 1> {
private:
    vector<T> data;
public:
    NdArray() : data(1) {}
    explicit NdArray(vector<size_t> shape) {
        assert(shape.size() == 1);
        data = vector<T>(shape[0]);
    }
    
    const T& operator[](size_t i) const { return data[i]; }
    T& operator[](size_t i) { return data[i]; }
};
