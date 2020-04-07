#ifndef ENACT_INSERTIONORDERMAP_H
#define ENACT_INSERTIONORDERMAP_H

#include <unordered_map>
#include <iostream>

template <class KeyType, class ValueType>
class InsertionOrderMapIter;

template <class KeyType, class ValueType>
class InsertionOrderMapConstIter;

template <class KeyType, class ValueType>
class InsertionOrderMap {
    friend class InsertionOrderMapIter<KeyType, ValueType>;
    friend class InsertionOrderMapConstIter<KeyType, ValueType>;

    std::unordered_map<KeyType, ValueType> m_map{};
    std::vector<KeyType> m_insertionOrder{};

public:
    /* Constructors */
    InsertionOrderMap() = default;
    InsertionOrderMap(std::initializer_list<std::pair<KeyType, ValueType>> values) :
            m_map{values},
            m_insertionOrder{}
    {
        std::transform(begin(values), end(values),
                std::back_inserter(m_insertionOrder),
                [](const auto& pair){ return pair.first; });
    };

    /* Iterators */
    using iterator = InsertionOrderMapIter<KeyType, ValueType>;
    using const_iterator = InsertionOrderMapConstIter<KeyType, ValueType>;

    iterator begin() {
        return iterator{*this, 0};
    }

    iterator end() {
        return iterator{*this, length()};
    }

    const_iterator begin() const {
        return const_iterator{*this, 0};
    }

    const_iterator end() const {
        return const_iterator{*this, length()};
    }

    const_iterator cbegin() const {
        return const_iterator{*this, 0};
    }

    const_iterator cend() const {
        return const_iterator{*this, length()};
    }

    /* Length */
    bool empty() const {
        return length() == 0;
    }

    size_t length() const {
        return m_insertionOrder.size();
    }

    /* Insertion and removal */
    void insert(const std::pair<KeyType, ValueType>& value) {
        m_map.insert(value);
        m_insertionOrder.push_back(value.first);
    }

    void erase(size_t index) {
        m_map.erase(m_insertionOrder[index]);
        m_insertionOrder.erase(index);
    }

    void clear() {
        m_map.clear();
        m_insertionOrder.clear();
    }

    /* Element access */
    ValueType& operator[](const KeyType& key) {
        return m_map[key];
    }

    const ValueType& operator[](const KeyType& key) const {
        return m_map[key];
    }

    std::optional<std::reference_wrapper<ValueType>> at(const KeyType& key) {
        if (contains(key)) return m_map.at(key);
        return {};
    }

    std::optional<std::reference_wrapper<const ValueType>> at(const KeyType& key) const {
        if (contains(key)) return m_map.at(key);
        return {};
    }

    std::optional<std::reference_wrapper<ValueType>> atIndex(size_t index) {
        if (index < length()) return m_map[m_insertionOrder[index]];
        return {};
    }

    std::optional<std::reference_wrapper<const ValueType>> atIndex(size_t index) const {
        if (index < length()) return m_map[m_insertionOrder[index]];
        return {};
    }

    std::optional<size_t> find(const KeyType& key) const {
        size_t index = 0;
        for (const auto& myKeyType : m_insertionOrder) {
            if (myKeyType == key) return index;
            ++index;
        }

        return {};
    }

    /* Checking */
    size_t count(const KeyType& key) const {
        return m_map.count(key);
    }

    bool contains(const KeyType& key) const {
        return count(key) > 0;
    }

    /* Keys and values */
    std::vector<std::reference_wrapper<KeyType>> keys() {
        std::vector<std::reference_wrapper<KeyType>> keys{};
        std::transform(
                m_insertionOrder.begin(), m_insertionOrder.end(),
                std::back_inserter(keys),
                [](auto& key) { return std::ref(key.get()); });
        return keys;
    }

    std::vector<std::reference_wrapper<const KeyType>> keys() const {
        std::vector<std::reference_wrapper<const KeyType>> keys{};
        std::transform(
                m_insertionOrder.begin(), m_insertionOrder.end(),
                std::back_inserter(keys),
                [](const auto& key) { return std::cref(key); });
        return keys;
    }

    std::vector<std::reference_wrapper<ValueType>> values() {
        std::vector<std::reference_wrapper<ValueType>> values;
        std::transform(
                m_insertionOrder.begin(), m_insertionOrder.end(),
                std::back_inserter(values),
                [this](auto& key) { return std::ref(m_map[key]); });
        return values;
    }

    std::vector<std::reference_wrapper<const ValueType>> values() const {
        std::vector<std::reference_wrapper<const ValueType>> values;
        std::transform(
                m_insertionOrder.begin(), m_insertionOrder.end(),
                std::back_inserter(values),
                [this](const auto& key) { return std::cref(m_map.at(key)); });
        return values;
    }
};

template <class KeyType, class ValueType>
class InsertionOrderMapIter {
    InsertionOrderMap<KeyType, ValueType>& m_map;
    size_t m_index;

public:
    InsertionOrderMapIter(InsertionOrderMap<KeyType, ValueType>& map, size_t index) :
            m_map{map},
            m_index{index} {
    }

    std::pair<const KeyType, ValueType>& operator*() {
        return *m_map.m_map.find(m_map.m_insertionOrder[m_index]);
    }

    InsertionOrderMapIter& operator++() {
        ++m_index;
        return *this;
    }

    InsertionOrderMapIter operator++(int) {
        return InsertionOrderMapIter<KeyType, ValueType>{m_map, m_index++};
    }

    InsertionOrderMapIter& operator--() {
        --m_index;
        return *this;
    }

    InsertionOrderMapIter operator--(int) {
        return InsertionOrderMapIter<KeyType, ValueType>{m_map, m_index--};
    }

    bool operator==(const InsertionOrderMapIter& other) const {
        return &m_map == &other.m_map && m_index == other.m_index;
    }

    bool operator!=(const InsertionOrderMapIter& other) const {
        return &m_map != &other.m_map || m_index != other.m_index;
    }
};

template <class KeyType, class ValueType>
class InsertionOrderMapConstIter {
    const InsertionOrderMap<KeyType, ValueType>& m_map;
    size_t m_index;

public:
    InsertionOrderMapConstIter(const InsertionOrderMap<KeyType, ValueType>& map, size_t index) :
            m_map{map},
            m_index{index} {
    }

    const std::pair<const KeyType, ValueType>& operator*() {
        return *m_map.m_map.find(m_map.m_insertionOrder[m_index]);
    };

    InsertionOrderMapConstIter& operator++() {
        ++m_index;
        return *this;
    }

    InsertionOrderMapConstIter operator++(int) {
        return InsertionOrderMapIter<KeyType, ValueType>{m_map, m_index++};
    }

    InsertionOrderMapConstIter& operator--() {
        --m_index;
        return *this;
    }

    InsertionOrderMapConstIter operator--(int) {
        return InsertionOrderMapIter<KeyType, ValueType>{m_map, m_index--};
    }

    bool operator==(const InsertionOrderMapConstIter& other) const {
        return &m_map == &other.m_map && m_index == other.m_index;
    }

    bool operator!=(const InsertionOrderMapConstIter& other) const {
        return &m_map != &other.m_map || m_index != other.m_index;
    }
};

#endif //ENACT_INSERTIONORDERMAP_H
