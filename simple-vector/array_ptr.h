#pragma once

template <typename Type>
class ArrayPtr {
public:
    ArrayPtr() = default;

    explicit ArrayPtr(size_t size) {
        size != 0 ? raw_ptr_ = new Type[size] : raw_ptr_ = nullptr;
    }

    explicit ArrayPtr(Type* raw_ptr) noexcept
        : raw_ptr_(raw_ptr) {
    }

    ArrayPtr(const ArrayPtr&) = delete;
    
    // конструктор перемещения
    ArrayPtr(ArrayPtr&&) = default;

    ~ArrayPtr() {
        delete[] raw_ptr_;
    }

    ArrayPtr& operator=(const ArrayPtr&) = delete;
    
    // перемещающий оператор присваивания
    ArrayPtr& operator=(ArrayPtr&&) = default;

    [[nodiscard]] Type* Release() noexcept {
        Type* array_ptr = raw_ptr_;
        raw_ptr_ = nullptr;
        return array_ptr;
    }

    Type& operator[](size_t index) noexcept {
        return *(raw_ptr_ + index);
    }

    const Type& operator[](size_t index) const noexcept {
        return *(raw_ptr_ + index);
    }

    explicit operator bool() const {
        return (raw_ptr_) ? true : false;
    }

    Type* Get() const noexcept {
        return raw_ptr_;
    }

    void swap(ArrayPtr& other) noexcept {
        Type* tmp_ptr = other.Get();
        other.raw_ptr_ = this->Release();
        raw_ptr_ = tmp_ptr;
    }

private:
    Type* raw_ptr_ = nullptr;
};