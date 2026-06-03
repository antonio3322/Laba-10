#pragma once

#include <algorithm>
#include <iostream>

template <typename T>
class deque {
private:
    T* data_;
    std::size_t capacity_;
    std::size_t size_;
    std::size_t head_;
    std::size_t reallocation_count_;

    std::size_t physical_index(std::size_t logical_index) const {
        return (head_ + logical_index) % capacity_;
    }

    void reallocate(std::size_t new_capacity) {
        T* new_data = new T[new_capacity];
        for (std::size_t i = 0; i < size_; ++i) {
            new_data[i] = (*this)[i];
        }
        delete[] data_;
        data_ = new_data;
        capacity_ = new_capacity;
        head_ = 0;
        ++reallocation_count_;
    }

    void ensure_capacity_for_push() {
        if (size_ == capacity_) {
            reallocate(capacity_ * 2);
        }
    }

    void shrink_if_needed() {
        if (capacity_ > 8 && size_ * 4 <= capacity_) {
            std::size_t next = capacity_ / 2;
            if (next < 8) {
                next = 8;
            }
            if (next != capacity_) {
                reallocate(next);
            }
        }
    }

public:
    template <typename ValueType, bool IsConst>
    class iterator_base {
    private:
        using deque_type = typename std::conditional<IsConst, const deque, deque>::type;
        deque_type* owner_;
        std::size_t index_;

    public:
        using difference_type = long long;
        using value_type = ValueType;
        using pointer = typename std::conditional<IsConst, const ValueType*, ValueType*>::type;
        using reference = typename std::conditional<IsConst, const ValueType&, ValueType&>::type;

        iterator_base() : owner_(nullptr), index_(0) {}
        iterator_base(deque_type* owner, std::size_t index) : owner_(owner), index_(index) {}

        template <bool OtherConst>
        iterator_base(const iterator_base<ValueType, OtherConst>& other)
                requires (IsConst || !OtherConst)
                : owner_(other.owner()), index_(other.index()) {}

        reference operator*() const { return (*owner_)[index_]; }
        pointer operator->() const { return &(*owner_)[index_]; }

        iterator_base& operator++() { ++index_; return *this; }
        iterator_base operator++(int) { iterator_base tmp = *this; ++(*this); return tmp; }
        iterator_base& operator--() { --index_; return *this; }
        iterator_base operator--(int) { iterator_base tmp = *this; --(*this); return tmp; }

        iterator_base& operator+=(difference_type n) { index_ += static_cast<std::size_t>(n); return *this; }
        iterator_base& operator-=(difference_type n) { index_ -= static_cast<std::size_t>(n); return *this; }

        iterator_base operator+(difference_type n) const { iterator_base tmp = *this; tmp += n; return tmp; }
        iterator_base operator-(difference_type n) const { iterator_base tmp = *this; tmp -= n; return tmp; }
        difference_type operator-(const iterator_base& other) const {
            return static_cast<difference_type>(index_) - static_cast<difference_type>(other.index_);
        }

        reference operator[](difference_type n) const { return *(*this + n); }

        bool operator==(const iterator_base& other) const { return owner_ == other.owner_ && index_ == other.index_; }
        bool operator!=(const iterator_base& other) const { return !(*this == other); }
        bool operator<(const iterator_base& other) const { return index_ < other.index_; }
        bool operator>(const iterator_base& other) const { return index_ > other.index_; }
        bool operator<=(const iterator_base& other) const { return index_ <= other.index_; }
        bool operator>=(const iterator_base& other) const { return index_ >= other.index_; }

        deque_type* owner() const { return owner_; }
        std::size_t index() const { return index_; }
    };

    template <typename Iter>
    class reverse_iterator_base {
    private:
        Iter base_;

    public:
        reverse_iterator_base() = default;
        explicit reverse_iterator_base(Iter base) : base_(base) {}

        auto operator*() const -> decltype(*base_) {
            Iter tmp = base_;
            --tmp;
            return *tmp;
        }

        reverse_iterator_base& operator++() { --base_; return *this; }
        reverse_iterator_base operator++(int) { reverse_iterator_base tmp = *this; --base_; return tmp; }
        reverse_iterator_base& operator--() { ++base_; return *this; }
        reverse_iterator_base operator--(int) { reverse_iterator_base tmp = *this; ++base_; return tmp; }

        bool operator==(const reverse_iterator_base& other) const { return base_ == other.base_; }
        bool operator!=(const reverse_iterator_base& other) const { return !(*this == other); }
    };

    using iterator = iterator_base<T, false>;
    using const_iterator = iterator_base<T, true>;
    using reverse_iterator = reverse_iterator_base<iterator>;
    using const_reverse_iterator = reverse_iterator_base<const_iterator>;

    deque() : data_(new T[8]), capacity_(8), size_(0), head_(0), reallocation_count_(0) {}

    deque(const deque& other)
            : data_(new T[other.capacity_]),
              capacity_(other.capacity_),
              size_(other.size_),
              head_(0),
              reallocation_count_(0) {
        for (std::size_t i = 0; i < size_; ++i) {
            data_[i] = other[i];
        }
    }

    deque& operator=(deque other) {
        swap(other);
        return *this;
    }

    ~deque() {
        delete[] data_;
    }

    void swap(deque& other) {
        std::swap(data_, other.data_);
        std::swap(capacity_, other.capacity_);
        std::swap(size_, other.size_);
        std::swap(head_, other.head_);
        std::swap(reallocation_count_, other.reallocation_count_);
    }

    void push_back(const T& value) {
        ensure_capacity_for_push();
        data_[physical_index(size_)] = value;
        ++size_;
    }

    void pop_back() {
        if (size_ == 0) {
            return;
        }
        --size_;
        shrink_if_needed();
    }

    void push_front(const T& value) {
        ensure_capacity_for_push();
        head_ = (head_ + capacity_ - 1) % capacity_;
        data_[head_] = value;
        ++size_;
    }

    T& operator[](std::size_t index) {
        return data_[physical_index(index)];
    }

    const T& operator[](std::size_t index) const {
        return data_[physical_index(index)];
    }

    T& front() { return (*this)[0]; }
    const T& front() const { return (*this)[0]; }
    T& back() { return (*this)[size_ - 1]; }
    const T& back() const { return (*this)[size_ - 1]; }

    bool empty() const { return size_ == 0; }
    std::size_t size() const { return size_; }

    iterator begin() { return iterator(this, 0); }
    const_iterator begin() const { return const_iterator(this, 0); }
    const_iterator cbegin() const { return const_iterator(this, 0); }

    iterator end() { return iterator(this, size_); }
    const_iterator end() const { return const_iterator(this, size_); }
    const_iterator cend() const { return const_iterator(this, size_); }

    reverse_iterator rbegin() { return reverse_iterator(end()); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator crbegin() const { return const_reverse_iterator(cend()); }

    reverse_iterator rend() { return reverse_iterator(begin()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
    const_reverse_iterator crend() const { return const_reverse_iterator(cbegin()); }

    std::size_t reallocation_count() const { return reallocation_count_; }
};
