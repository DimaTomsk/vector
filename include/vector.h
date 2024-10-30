#pragma once

#include <memory>
#include <type_traits>
#include <stdexcept>

template<typename Allocator>
struct compressed_meta : Allocator {
    size_t capacity_;
    size_t size_;

    compressed_meta() noexcept(std::is_nothrow_constructible_v<Allocator>)
        : Allocator(),
          capacity_(0),
          size_(0) {
    }

    explicit compressed_meta(const Allocator &alloc) noexcept(std::is_nothrow_copy_constructible_v<Allocator>)
        : Allocator(alloc),
          capacity_(0),
          size_(0) {
    }
};

template<typename T, bool reverse>
class vector_iterator {
public:
    vector_iterator(T *ptr) : ptr_(ptr) {
    }

    ptrdiff_t operator-(const vector_iterator &other) const {
        if constexpr (reverse) {
            return other.ptr_ - ptr_;
        } else {
            return ptr_ - other.ptr_;
        }
    }

    vector_iterator &operator+=(ptrdiff_t offset) {
        ptr_ = ptr_add(ptr_, offset);
        return *this;
    }

    vector_iterator &operator-=(ptrdiff_t offset) {
        ptr_ = ptr_add(ptr_, -offset);
        return *this;
    }

    vector_iterator operator+(ptrdiff_t offset) const {
        return {ptr_add(ptr_, offset)};
    }

    vector_iterator operator-(ptrdiff_t offset) const {
        return {ptr_add(ptr_, -offset)};
    }

    vector_iterator &operator++() {
        ptr_ = ptr_add(ptr_, 1);
        return *this;
    }

    vector_iterator operator++(int) {
        vector_iterator result{ptr_};
        ptr_ = ptr_add(ptr_, 1);
        return result;
    }

    vector_iterator &operator--() {
        ptr_ = ptr_add(ptr_, -1);
        return *this;
    }

    vector_iterator operator--(int) {
        vector_iterator result{ptr_};
        ptr_ = ptr_add(ptr_, -1);
        return result;
    }

    T &operator*() const {
        return *ptr_;
    }

    T *operator->() const {
        return ptr_;
    }

    bool operator==(const vector_iterator &other) const { return ptr_ == other.ptr_; }

private:
    T *ptr_;

    static T *ptr_add(T *ptr, ptrdiff_t offset) {
        if constexpr (reverse) {
            return ptr - offset;
        } else {
            return ptr + offset;
        }
    }
};


template<typename T, typename Allocator=std::allocator<T> >
    requires std::is_copy_constructible_v<T>
class vector {
    using meta = compressed_meta<Allocator>;

public:
    using iterator = vector_iterator<T, false>;
    using const_iterator = vector_iterator<std::add_const_t<T>, false>;
    using reverse_iterator = vector_iterator<T, true>;
    using const_reverse_iterator = vector_iterator<std::add_const_t<T>, true>;

    vector() noexcept(std::is_nothrow_constructible_v<meta>)
        : meta_(),
          data_(nullptr) {
    }

    explicit vector(const Allocator &alloc) noexcept(std::is_nothrow_copy_constructible_v<meta>)
        : meta_(alloc),
          data_(nullptr) {
    }

    vector(size_t count, const T &value, const Allocator &alloc = Allocator()) : vector(alloc) {
        resize(count, value);
    }

    explicit vector(size_t count, const Allocator &alloc = Allocator()) : vector(alloc) {
        resize(count);
    }

    template<class InputIt>
        requires (!std::is_integral_v<InputIt>)
    vector(InputIt first, InputIt last, const Allocator &alloc = Allocator()) : vector(alloc) {
        reserve(distance(first, last));
        for (; first != last; ++first) {
            emplace_back(*first);
        }
    }

    vector(const vector &other) : vector(other.begin(), other.end(), other.get_allocator()) {
    }

    vector(const vector &other, const Allocator &alloc) : vector(other.begin(), other.end(), alloc) {
    }

    vector(vector &&other) noexcept(noexcept(std::is_nothrow_move_constructible_v<Allocator>))
        : meta_(std::move(other.meta_)),
          data_(other.data_) {
        other.data_ = nullptr;
        other.meta_.capacity_ = 0;
        other.meta_.size_ = 0;
    }

    vector(vector &&other, const Allocator &alloc) : vector(alloc) {
        data_ = other.data_;
        other.data_ = nullptr;

        meta_.size_ = other.meta_.size_;
        meta_.capacity_ = other.meta_.capacity_;
        other.meta_.size_ = 0;
        other.meta_.capacity_ = 0;
    }

    vector(std::initializer_list<T> init, const Allocator &alloc = Allocator())
        : vector(init.begin(), init.end(), alloc) {
    }

    ~vector() {
        clear();
        if (meta_.capacity_ > 0) {
            std::allocator_traits<Allocator>::deallocate(meta_, data_, meta_.capacity_);
        }
    }

    vector &operator=(vector &&other) noexcept {
        if (this != &other) {
            swap(other);
            other.clear();
        }
        return *this;
    }

    vector &operator=(const vector &other) {
        if (this != &other) {
            clear();
            reserve(other.capacity());
            for (const auto &elem: other) {
                emplace_back(elem);
            }
        }
        return *this;
    }


    vector &operator=(std::initializer_list<T> init) {
        clear();
        reserve(init.size());
        for (const auto &elem: init) {
            emplace_back(elem);
        }
        return *this;
    }


    void resize(size_t new_size) {
        while (size() > new_size) {
            pop_back();
        }
        if (size() < new_size) {
            reserve(new_size);
            while (size() < new_size) {
                emplace_back();
            }
        }
    }

    void resize(size_t new_size, const T &value) {
        while (size() > new_size) {
            pop_back();
        }
        if (size() < new_size) {
            reserve(new_size);
            while (size() < new_size) {
                emplace_back(value);
            }
        }
    }

    void reserve(size_t new_cap) {
        if (new_cap <= meta_.capacity_) {
            return;
        }
        reserve_impl(new_cap);
    }

    void push_back(T &&value) {
        if (capacity() == size()) {
            reserve(2 * capacity() + 1);
        }
        std::allocator_traits<Allocator>::construct(meta_, data_ + size(), std::forward<T>(value));
        ++meta_.size_;
    }

    void clear() {
        while (size() > 0) {
            pop_back();
        }
    }

    template<typename... Args>
    T &emplace_back(Args &&... args) {
        if (capacity() == size()) {
            reserve(2 * capacity() + 1);
        }
        std::allocator_traits<Allocator>::construct(meta_, data_ + size(), std::forward<Args>(args)...);
        ++meta_.size_;
        return back();
    }

    void pop_back() {
        std::allocator_traits<Allocator>::destroy(meta_, data_ + size() - 1);
        --meta_.size_;
    }

    void assign(size_t count, const T &value) {
        clear();
        reserve(count);
        for (size_t i = 0; i < count; ++i) {
            emplace_back(value);
        }
    }

    void shrink_to_fit() {
        if (size() < capacity()) {
            reserve_impl(size());
        }
    }

    void swap(vector &other) noexcept {
        std::swap(meta_, other.meta_);
        std::swap(data_, other.data_);
    }

    template<class InputIt>
        requires (std::is_same_v<std::remove_reference_t<decltype(*std::declval<InputIt>())>, T>)
    void assign(InputIt first, InputIt last) {
        clear();
        reserve(distance(first, last));
        for (; first != last; ++first) {
            emplace_back(*first);
        }
    }

    void assign(std::initializer_list<T> ilist) {
        assign(ilist.begin(), ilist.end());
    }

    [[nodiscard]] const Allocator &get_allocator() const {
        return meta_;
    }

    T &at(size_t i) {
        if (i >= size()) {
            throw std::out_of_range("vector out of range");
        }
        return data_[i];
    }

    const T &at(size_t i) const {
        if (i >= size()) {
            throw std::out_of_range("vector out of range");
        }
        return data_[i];
    }

    [[nodiscard]] T &operator[](size_t i) {
        return data_[i];
    }

    [[nodiscard]] const T &operator[](size_t i) const {
        return data_[i];
    }

    [[nodiscard]] T &front() {
        return data_[0];
    }

    [[nodiscard]] const T &front() const {
        return data_[0];
    }

    [[nodiscard]] T &back() {
        return data_[meta_.size_ - 1];
    }

    [[nodiscard]] const T &back() const {
        return data_[meta_.size_ - 1];
    }

    [[nodiscard]] T *data() {
        return data_;
    }

    [[nodiscard]] const T *data() const {
        return data_;
    }

    [[nodiscard]] iterator begin() { return {data_}; }
    [[nodiscard]] const_iterator begin() const { return {data_}; }
    [[nodiscard]] const_iterator cbegin() const { return {data_}; }

    [[nodiscard]] iterator end() { return {data_ + size()}; }
    [[nodiscard]] const_iterator end() const { return {data_ + size()}; }
    [[nodiscard]] const_iterator cend() const { return {data_ + size()}; }

    [[nodiscard]] reverse_iterator rbegin() { return {data_ + size() - 1}; }
    [[nodiscard]] const_reverse_iterator rbegin() const { return {data_ + size() - 1}; }
    [[nodiscard]] const_reverse_iterator crbegin() const { return {data_ + size() - 1}; }

    [[nodiscard]] reverse_iterator rend() { return {data_ - 1}; }
    [[nodiscard]] const_reverse_iterator rend() const { return {data_ - 1}; }
    [[nodiscard]] const_reverse_iterator crend() const { return {data_ - 1}; }

    [[nodiscard]] size_t size() const noexcept { return meta_.size_; }

    [[nodiscard]] bool empty() const noexcept { return size() == 0; }
    [[nodiscard]] size_t capacity() const noexcept { return meta_.capacity_; }

    [[nodiscard]] bool operator==(const vector &other) const {
        if (size() != other.size()) {
            return false;
        }
        for (size_t i = 0; i < size(); ++i) {
            if (data_[i] != other[i]) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] std::strong_ordering operator<=>(const vector &other) const {
        for (size_t i = 0; i < size() && i < other.size(); ++i) {
            if (const std::strong_ordering cmp = (*this)[i] <=> other[i]; cmp != std::strong_ordering::equal) {
                return cmp;
            }
        }
        return size() <=> other.size();
    }

private:
    meta meta_;
    T *data_;

    void reserve_impl(size_t new_cap) {
        T *new_memory = std::allocator_traits<Allocator>::allocate(meta_, new_cap);
        size_t i = 0;
        try {
            while (i < size()) {
                if constexpr (std::is_move_constructible_v<T>) {
                    std::allocator_traits<Allocator>::construct(meta_, new_memory + i, std::move(data_[i]));
                } else {
                    std::allocator_traits<Allocator>::construct(meta_, new_memory + i, data_[i]);
                }
                std::allocator_traits<Allocator>::destroy(meta_, data_ + i);
                ++i;
            }
        } catch (std::exception &) {
            for (size_t j = 0; j < i; ++j) {
                if constexpr (std::is_move_constructible_v<T>) {
                    std::allocator_traits<Allocator>::construct(meta_, data_ + j, new_memory[j]);
                } else {
                    std::allocator_traits<Allocator>::construct(meta_, data_ + j, new_memory[j]);
                }
                std::allocator_traits<Allocator>::destroy(meta_, new_memory + j);
            }
            throw;
        }

        std::allocator_traits<Allocator>::deallocate(meta_, data_, capacity());

        data_ = new_memory;
        meta_.capacity_ = new_cap;
    }

    template<typename Iter>
    static ptrdiff_t distance(Iter a, Iter b) {
        if constexpr (std::is_same_v<Iter, iterator>
                      || std::is_same_v<Iter, const_iterator>
                      || std::is_same_v<Iter, reverse_iterator>
                      || std::is_same_v<Iter, const_reverse_iterator>) {
            return b - a;
        } else {
            return std::distance(a, b);
        }
    }
};
