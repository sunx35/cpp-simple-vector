#pragma once

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include "array_ptr.h"

struct ReserveProxyObj {
public:
    ReserveProxyObj(size_t capacity) {
        capacity_ = capacity;
    }

    size_t capacity_ = 0;
};

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) : array_ptr_(size) {
        size_ = size;
        capacity_ = size;
        std::fill(begin(), end(), Type());
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) : array_ptr_(size) {
        for (size_t i = 0; i < size; ++i) {
            array_ptr_.GetRawPtr()[i] = value;
        }
        size_ = size;
        capacity_ = size;
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) : array_ptr_(init.size()) {
        size_ = init.size();
        capacity_ = init.size();

        auto array_ptr = array_ptr_.GetRawPtr();
        for (auto it = init.begin(); it != init.end(); ++it) {
            *array_ptr = *it;
            ++array_ptr;
        }
    }

    SimpleVector(ReserveProxyObj reserve) : array_ptr_() {
        Reserve(reserve.capacity_);
    }

    SimpleVector(const SimpleVector& other) {
        SimpleVector tmp(other.size_);
        std::copy(other.begin(), other.end(), tmp.begin());
        tmp.size_ = other.size_;
        tmp.capacity_ = other.size_;
        swap(tmp);
    }

    SimpleVector(SimpleVector&& other) {
        array_ptr_ = std::move(other.array_ptr_);
        size_ = std::exchange(other.size_, 0);
        capacity_ = std::exchange(other.capacity_, 0);
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        SimpleVector tmp(rhs);
        swap(tmp);
        return *this;
    }

    SimpleVector& operator=(SimpleVector&& rhs) {
        array_ptr_ = std::move(rhs.array_ptr_);
        size_ = std::exchange(rhs.size_, 0);
        capacity_ = std::exchange(rhs.capacity_, 0);
        return *this;
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        return array_ptr_.GetRawPtr()[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        return array_ptr_.GetRawPtr()[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index is out of range");
        }
        return array_ptr_.GetRawPtr()[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Index is out of range");
        }
        return array_ptr_.GetRawPtr()[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size == size_) {
            return;
        }
        else if (new_size < size_) {
            size_ = new_size;
        }
        else if (new_size > size_ && new_size <= capacity_) {
            for (auto it = array_ptr_.GetRawPtr() + size_; it != array_ptr_.GetRawPtr() + new_size; ++it) {
                *it = Type();
            }
            size_ = new_size;
        }
        else if (new_size > capacity_) {
            array_ptr_.Resize(size_, std::max(new_size, capacity_ * 2));
            for (auto it = array_ptr_.GetRawPtr() + size_; it != array_ptr_.GetRawPtr() + new_size; ++it) {
                *it = Type();
            }
            size_ = new_size;
            capacity_ = std::max(new_size, capacity_ * 2);
        }
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        if (size_ == capacity_) {
            Resize(size_ + 1);
            *(end() - 1) = item;
        }
        else {
            *end() = item;
            ++size_;
        }
    }

    void PushBack(Type&& item) {
        if (size_ == capacity_) {
            Resize(size_ + 1);
            *(end() - 1) = std::move(item);
        }
        else {
            *end() = std::move(item);
            ++size_;
        }
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        size_t pos_num = pos - begin();

        if (size_ == capacity_) {
            Resize(size_ + 1);
            Iterator it = begin() + pos_num;
            std::copy_backward(it, end() - 1, end());
            *it = value;
            return it;
        }
        else {
            Iterator it = begin() + pos_num;
            std::copy_backward(it, end(), end() + 1);
            *it = value;
            ++size_;
            return it;
        }
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        size_t pos_num = pos - begin();

        if (size_ == capacity_) {
            Resize(size_ + 1);
            Iterator it = begin() + pos_num;
            std::move_backward(it, end(), end() + 1);
            *it = std::move(value);
            return it;
        }
        else {
            Iterator it = begin() + pos_num;
            std::move_backward(it, end(), end() + 1);
            *it = std::move(value);
            ++size_;
            return it;
        }
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        if (size_ == 0) {
            return;
        }
        --size_;
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        Iterator non_const_pos = const_cast<Iterator>(pos);
        std::move(non_const_pos + 1, end(), non_const_pos);
        --size_;
        return non_const_pos;
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        Type* temp = other.array_ptr_.GetRawPtr();
        other.array_ptr_.SetRawPtr(array_ptr_.GetRawPtr());
        array_ptr_.SetRawPtr(temp);

        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    void Reserve(size_t new_capacity) {
        if (capacity_ >= new_capacity) {
            return;
        }
        array_ptr_.Resize(size_, new_capacity);
        capacity_ = new_capacity;
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return array_ptr_.GetRawPtr();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return array_ptr_.GetRawPtr() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return array_ptr_.GetRawPtr();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return array_ptr_.GetRawPtr() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return array_ptr_.GetRawPtr();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return array_ptr_.GetRawPtr() + size_;
    }

private:
    ArrayPtr<Type> array_ptr_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    if (&lhs == &rhs) {
        return true;
    }
    if (lhs.GetSize() != rhs.GetSize()) {
        return false;
    }
    return std::equal(lhs.cbegin(), lhs.cend(), rhs.cbegin());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !operator==(lhs, rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !operator<(rhs, lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return operator<(rhs, lhs);
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !operator<(lhs, rhs);
}

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
};