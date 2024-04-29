#pragma once
#include <utility>

template <typename Type>
class ArrayPtr {
public:
    ArrayPtr() = default;

    ArrayPtr(size_t size) {
        array_ = new Type[size];
    }

    ArrayPtr(ArrayPtr&& other) {
        array_ = std::exchange(other.array_, nullptr);
    }

    ArrayPtr& operator=(ArrayPtr&& rhs) {
        array_ = std::exchange(rhs.array_, nullptr);
        return *this;
    }

    ~ArrayPtr() {
        delete[] array_;
    }

    Type* GetRawPtr() const {
        return array_;
    }

    void Resize(size_t old_size, size_t new_size) {
        Type* temp_ptr = new Type[new_size];
        std::move(array_, array_ + old_size, temp_ptr);
        delete[] array_;
        array_ = temp_ptr;
    }

    void SetRawPtr(Type* ptr) {
        array_ = ptr;
    }

private:
    Type* array_ = nullptr;
};