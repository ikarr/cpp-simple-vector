#pragma once

#include <algorithm>
#include <initializer_list>
#include <stdexcept>
#include <utility>

#include "array_ptr.h"

class ReserveProxyObj {
public:
    ReserveProxyObj(size_t capacity_to_reserve)
        : capacity_to_reserve_(capacity_to_reserve)
    {
    }

    size_t GetCapacity() {
        return capacity_to_reserve_;
    }
    
private:
    size_t capacity_to_reserve_;
};
 
ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    SimpleVector(ReserveProxyObj other)
    {
        Reserve(other.GetCapacity());
    }

    explicit SimpleVector(size_t size)
        : items_(size)
        , size_(size)
        , capacity_(size)
    {
        std::fill(begin(), end(), Type{});
    }

    SimpleVector(size_t size, const Type& value)
        : items_(size)
        , size_(size)
        , capacity_(size)
    {
        std::fill(begin(), end(), value);
    }

    SimpleVector(std::initializer_list<Type> init)
        : items_(init.size())
        , size_(init.size())
        , capacity_(init.size())
    {
        std::move(init.begin(), init.end(), begin());
    }
    
    SimpleVector(const SimpleVector& other)
    {
        SimpleVector tmp(other.size_);
        std::copy(other.begin(), other.end(), tmp.begin());
        swap(tmp);
    }

    // конструктор перемещения
    SimpleVector(SimpleVector&& other)
        : items_(std::exchange(other.items_, ArrayPtr<Type> {}))
        , size_(std::exchange(other.size_, 0))
        , capacity_(std::exchange(other.capacity_, 0))
    {
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (*this != rhs) {
            auto tmp(rhs);
            swap(tmp);
        }
        return *this;
    }

    // перемещающий оператор присваивания
    SimpleVector& operator=(SimpleVector&& rhs) {
        if (*this != rhs) {
            auto tmp(rhs);
            swap(tmp);
        }
        return *this;
    }

    // Вставляет значение value в конец вектора путём копирования
    void PushBack(const Type& value) {
        Insert(end(), value);
    }

    // Вставляет значение value в конец вектора путём перемещения
    void PushBack(Type&& value) {
        Insert(end(), std::move(value));
    }

    // Вставляет значение value в позицию pos путём копирования
    Iterator Insert(ConstIterator pos, const Type& value) {
        size_t index = std::distance(cbegin(), pos);
        if (size_ < capacity_) {
            std::copy_backward(pos, cend(), std::next(end()));
            items_[index] = value;
            ++size_;
            return Iterator(pos);
        } else {
            size_t capacity_new = std::max(size_ + 1, capacity_ * 2);
            ArrayPtr<Type> items_copy(capacity_new);
            auto begin_new = items_copy.Get();
            std::move(cbegin(), pos, begin_new);
            items_copy[index] = value;
            std::advance(begin_new, index + 1);
            std::move(pos, cend(), begin_new);
            items_.swap(items_copy);
            capacity_ = capacity_new;
            ++size_;
            return Iterator(&(items_[index]));
        }
    }
    
    // Вставляет значение value в позицию pos путём перемещения
    Iterator Insert(Iterator pos, Type&& value) {
        size_t index = std::distance(begin(), pos);
        if (size_ < capacity_) {
            std::move_backward(pos, end(), std::next(end()));
            items_[index] = std::move(value);
            ++size_;
            return Iterator(pos);
        } else {
            size_t capacity_new = std::max(size_ + 1, capacity_ * 2);
            ArrayPtr<Type> items_copy(capacity_new);
            auto begin_new = items_copy.Get();
            std::move(begin(), pos, begin_new);
            items_copy[index] = std::move(value);
            std::advance(begin_new, index + 1);
            std::move(pos, end(), begin_new);
            items_.swap(items_copy);
            capacity_ = capacity_new;
            ++size_;
            return Iterator(&(items_[index]));
        }
    }

    void PopBack() noexcept {
        if (!IsEmpty()) {
            --size_;
        }
    }

    Iterator Erase(ConstIterator pos) {
        if (pos == end()) {
            return end();
        }
        std::move(Iterator(pos + 1), end(), Iterator(pos));
        size_--;
        return Iterator(pos);
    }

    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }
    
    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> items_copy(new_capacity);
            auto begin_new = items_copy.Get();
            std::move(begin(), end(), begin_new);
            items_.swap(items_copy);
            capacity_ = new_capacity;
        }
    }
    
    size_t GetSize() const noexcept {
        return size_;
    }

    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    Type& operator[](size_t index) noexcept {
        return items_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        return items_[index];
    }

    Type& At(size_t index) {
        return index < size_
        ? items_[index]
        : throw std::out_of_range("Incorrect argument: SimpleVector::At");
    }

    const Type& At(size_t index) const {
        return index < size_
        ? items_[index]
        : throw std::out_of_range("Incorrect argument: SimpleVector::At");
    }

    void Clear() noexcept {
        size_ = 0;
    }

    void Resize(size_t new_size) {
        if (new_size <= size_) {
            size_ = new_size;
        }
        else if (new_size > size_ && new_size <= capacity_) {
            std::generate(end(), &items_[new_size], []() {
                return Type{};
            });
            size_ = new_size;
        }
        else if (new_size > capacity_) {
            ArrayPtr<Type> new_items(new_size);
            std::move(begin(), end(), &new_items[0]);
            std::generate(&new_items[size_], &new_items[new_size], []() {
                return Type{};
            });
            items_.swap(new_items);
            size_ = new_size;
            capacity_ = std::max(capacity_ * 2, new_size);
        }
    }

    Iterator begin() noexcept {
        return Iterator(items_.Get());
    }

    Iterator end() noexcept {
        return Iterator(items_.Get() + size_);
    }

    ConstIterator begin() const noexcept {
        return ConstIterator(items_.Get());
    }

    ConstIterator end() const noexcept {
        return ConstIterator(items_.Get() + size_);
    }

    ConstIterator cbegin() const noexcept {
        return ConstIterator(items_.Get());
    }

    ConstIterator cend() const noexcept {
        return ConstIterator(items_.Get() + size_);
    }

private:
    ArrayPtr<Type> items_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return lhs.GetSize() == rhs.GetSize() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}