#pragma once

#include <vector>
#include <forward_list>
#include <utility>
#include <cstddef>
#include <functional>

template<typename K, typename V>
class HashMap {
public:
    // Entry stores a const key and a mutable value
    struct Entry {
        const K key;
        V value;
        Entry(const K& key, const V& value) : key(key), value(value) {}
    };

    using Bucket = std::forward_list<Entry>;
    using Table  = std::vector<Bucket>;

    // Construct with a number of buckets (must be >= 1)
    HashMap(std::size_t nbuckets = 1024) : buckets_(nbuckets) {
    }

    // Current number of stored entries
    std::size_t size() const{
        return count_;
    }

    // Return pointer to value associated with key, or nullptr if not found.
    V* get(const K& key){
        size_t i = std::hash<K>{}(key) % this->buckets_.size();
        for (auto& entry : this->buckets_[i]){
            if (entry.key == key){
                return &entry.value;
            }
        }
        return nullptr;
    }

    // Insert or update (key,value).
    // Returns true if an existing entry was updated, false if a new entry was inserted.
    bool put(const K& key, const V& value){
        size_t i = std::hash<K>{}(key) % this->buckets_.size();
        for (auto& entry : this->buckets_[i]){
            if (entry.key == key){
                entry.value = value;
                return true;    
            }
        }
        buckets_[i].push_front(Entry(key, value));
        ++count_;
        return false;
    }


    // Convert table contents to a vector of key/value pairs.
    std::vector<std::pair<K, V>> toKeyValuePairs() const {
        std::pair<K,V> paires;
        std::vector<std::pair<K, V>> le_vecteur;
        for (const auto & bucket : this->buckets_){
            for (const auto& entry : bucket){
                paires.first = entry.key;
                paires.second = entry.value;
                le_vecteur.push_back(paires);
            }
        }
        return le_vecteur;
    }
    // Optional: number of buckets
    // std::size_t bucket_count() const;

private:
    Table buckets_;
    std::size_t count_ = 0;
};
