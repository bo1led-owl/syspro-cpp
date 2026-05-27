#pragma once

#include <algorithm>
#include <cassert>
#include <concepts>
#include <functional>
#include <limits>
#include <list>
#include <utility>

namespace internal {
template <std::default_initializable T>
class HeapArray {
    T* items_;
    size_t size_;

    static T* init(size_t n) {
        return new T[n];
    }

public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using size_type = std::size_t;
    using difference_type = ptrdiff_t;
    using iterator = T*;
    using const_iterator = const T*;

    HeapArray() : items_{nullptr}, size_{0} {}

    explicit HeapArray(size_t n) : items_{init(n)}, size_{n} {}

    HeapArray(const HeapArray&) = delete;
    HeapArray& operator=(const HeapArray&) = delete;

    HeapArray(HeapArray&& that)
        : items_{std::exchange(that.items_, nullptr)}, size_{std::exchange(that.size_, 0)} {}

    HeapArray& operator=(HeapArray&& that) {
        std::swap(items_, that.items_);
        std::swap(size_, that.size_);
        return *this;
    }

    ~HeapArray() {
        delete[] items_;
    }

    size_type size() const {
        assert((items_ == nullptr) == (size_ == 0));
        return size_;
    }

    bool empty() const {
        assert((items_ == nullptr) == (size_ == 0));
        return items_ == nullptr;
    }

    T& operator[](size_t i) {
        assert(i < size_);
        return items_[i];
    }

    const T& operator[](size_t i) const {
        assert(i < size_);
        return items_[i];
    }

    T* begin() {
        return items_;
    }

    T* end() {
        assert((items_ == nullptr) == (size_ == 0));
        return items_ + size_;
    }

    const T* begin() const {
        return cbegin();
    }

    const T* end() const {
        return cend();
    }

    const T* cbegin() const {
        return items_;
    }

    const T* cend() const {
        assert((items_ == nullptr) == (size_ == 0));
        return items_ + size_;
    }
};
}  // namespace internal

template <typename T, typename Hash = std::hash<T>, typename Eq = std::equal_to<T>>
    requires std::predicate<Eq, const T&, const T&>
class HashSet {
public:
    using key_type = T;
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using size_type = std::size_t;
    using difference_type = ptrdiff_t;
    using hasher = Hash;
    using key_equal = Eq;

private:
    using Bucket = std::list<T>;
    using Buckets = internal::HeapArray<Bucket>;

    static constexpr float MAX_LOAD_FACTOR = 0.75f;

    Buckets buckets_;
    size_t size_ = 0;
    Hash hash_;
    Eq eq_;

    class Iterator {
        friend class HashSet;

        using B = const Buckets;
        using BucketIter = typename B::const_iterator;
        using ItemIter = typename Bucket::const_iterator;

        B* buckets_ = nullptr;
        BucketIter curBucket_ = nullptr;
        ItemIter curItem_{};

    public:
        using const_reference = const_reference;
        using reference = const_reference;
        using const_pointer = const_pointer;
        using pointer = const_pointer;
        using difference_type = difference_type;
        using value_type = value_type;
        using iterator_category = std::forward_iterator_tag;

        Iterator() = default;

        explicit Iterator(B* buckets, BucketIter curBucket = nullptr, ItemIter curItem = ItemIter())
            : buckets_{buckets}, curBucket_{curBucket}, curItem_{curItem} {
            if (!buckets) {
                curBucket_ = nullptr;
                return;
            }

            if (curBucket_ == nullptr) {
                curBucket_ = buckets_->begin();
            }

            while (curBucket_ != buckets_->end() && curBucket_->empty()) {
                ++curBucket_;
            }

            if (curBucket_ == buckets_->end()) {
                buckets_ = nullptr;
                return;
            }

            curItem_ = curBucket_->begin();
        }

        bool operator==(const Iterator& that) const {
            if (buckets_ == nullptr) {
                return that.buckets_ == nullptr;
            }

            return buckets_ == that.buckets_ && curBucket_ == that.curBucket_ &&
                   curItem_ == that.curItem_;
        }

        reference operator*() const {
            return *curItem_;
        }

        pointer operator->() const {
            return &(*curItem_);
        }

        Iterator& operator++() {
            ++curItem_;
            if (curItem_ != curBucket_->end()) {
                return *this;
            }

            do {
                ++curBucket_;
            } while (curBucket_ != buckets_->end() && curBucket_->empty());

            if (curBucket_ == buckets_->end()) {
                buckets_ = nullptr;
                return *this;
            }

            curItem_ = curBucket_->begin();
            return *this;
        }

        Iterator operator++(int) {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }
    };

    static_assert(std::forward_iterator<Iterator>);

public:
    using iterator = Iterator;
    using const_iterator = Iterator;

    explicit HashSet(Hash hash = Hash(), Eq eq = Eq()) : hash_{hash}, eq_{eq} {}

    HashSet(std::initializer_list<T> items, Hash hash = Hash(), Eq eq = Eq())
        : hash_{hash}, eq_{eq} {
        for (auto&& item : items) {
            insert(item);
        }
    }

    template <std::input_iterator It>
    HashSet(It first, It last, Hash hash = Hash(), Eq eq = Eq()) : hash_{hash}, eq_{eq} {
        while (first != last) {
            insert(*first);
        }
    }

    HashSet(const HashSet& that)
        : buckets_(that.buckets_.size()), size_{that.size_}, hash_{that.hash_}, eq_{that.eq_} {
        std::copy(that.buckets_.begin(), that.buckets_.end(), buckets_.begin());
    }

    HashSet& operator=(const HashSet& that) {
        if (this == &that) {
            return *this;
        }

        buckets_ = Buckets(that.buckets_.size());
        size_ = that.size_;
        hash_ = that.hash_;
        eq_ = that.eq_;

        std::copy(that.buckets_.begin(), that.buckets_.end(), buckets_.begin());

        return *this;
    }

    HashSet(HashSet&& that)
        : buckets_{std::move(that.buckets_)},
          size_{std::exchange(that.size_, 0)},
          hash_{std::move(that.hash_)},
          eq_{std::move(that.eq_)} {}

    HashSet& operator=(HashSet&& that) {
        std::swap(buckets_, that.buckets_);
        std::swap(size_, that.size_);
        std::swap(hash_, that.hash_);
        std::swap(eq_, that.eq_);

        return *this;
    }

    void clear() {
        buckets_ = Buckets{};
    }

    bool empty() const {
        return size_ == 0;
    }

    size_type size() const {
        return size_;
    }

    std::pair<iterator, bool> insert(const T& x) {
        return emplace(x);
    }

    std::pair<iterator, bool> insert(T&& x) {
        if constexpr (std::is_move_constructible_v<T>) {
            return emplace(std::move(x));
        } else {
            return emplace<const T&>(x);
        }
    }

    template <typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        Bucket newItem;
        newItem.emplace_front(std::forward<Args>(args)...);

        auto it = find(newItem.front());
        if (it != end()) {
            return {it, false};
        }

        size_ += 1;
        rehashIfNeeded();

        return {transferBucketHead(*findBucket(newItem.front()), newItem), true};
    }

    const_iterator find(const T& x) const {
        auto bucket = findBucket(x);
        if (bucket == buckets_.end()) {
            return end();
        }

        auto it = std::ranges::find_if(*bucket, [this, &x](const T& y) { return eq_(x, y); });

        if (it == bucket->end()) {
            return const_iterator{};
        }

        return const_iterator{&buckets_, bucket, it};
    }

    bool contains(const T& x) const {
        return find(x) != end();
    }

    iterator erase(const_iterator pos) {
        assert(pos.buckets_ == &buckets_);

        auto res = pos;
        ++res;
        const_cast<Bucket*>(pos.curBucket_)->erase(pos.curItem_);

        return res;
    }

    size_type erase(const T& x) {
        auto bucket = findBucket(x);

        size_type count = 0;

        auto it = bucket->begin();
        while (it != bucket->end()) {
            if (eq_(*it, x)) {
                bucket->erase(it++);
                count += 1;
            } else {
                ++it;
            }
        }

        return count;
    }

    const_iterator begin() const {
        return cbegin();
    }
    const_iterator cbegin() const {
        return const_iterator{&buckets_};
    }

    const_iterator end() const {
        return cend();
    }
    const_iterator cend() const {
        return const_iterator{nullptr};
    }

private:
    float loadFactor() const {
        if (buckets_.size() == 0) [[unlikely]] {
            return std::numeric_limits<float>::infinity();
        }

        return static_cast<float>(size_) / static_cast<float>(buckets_.size());
    }

    static size_t nextCapacity(size_t cur) {
        if (cur == 0) [[unlikely]] {
            return 8;
        }
        return cur * 2;
    }

    void rehashIfNeeded() {
        if (loadFactor() < MAX_LOAD_FACTOR) [[likely]] {
            return;
        }

        size_t newCapacity = nextCapacity(buckets_.size());
        Buckets oldBuckets = std::exchange(buckets_, Buckets(newCapacity));

        for (Bucket& bucket : oldBuckets) {
            while (!bucket.empty()) {
                transferBucketHead(*findBucket(bucket.front()), bucket);
            }
        }
    }

    Buckets::iterator findBucket(const T& item) {
        return const_cast<Buckets::iterator>(const_cast<const HashSet*>(this)->findBucket(item));
    }

    Buckets::const_iterator findBucket(const T& item) const {
        if (buckets_.empty()) {
            return buckets_.end();
        }

        assert(std::popcount(buckets_.size()) == 1);
        size_t hash = hash_(item) & (buckets_.size() - 1);
        return &buckets_[hash];
    }

    iterator transferBucketHead(Bucket& into, Bucket& from) {
        // assert(!buckets_.empty());
        // typename Buckets::iterator into = findBucket(from.front());
        // assert(into != buckets_.end());

        into.splice(into.begin(), from, from.begin());
        return iterator{&buckets_, &into, into.begin()};
    }
};
