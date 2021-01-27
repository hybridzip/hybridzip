#ifndef HYBRIDZIP_ARCHIVE_TRIE_H
#define HYBRIDZIP_ARCHIVE_TRIE_H

#include <unordered_map>
#include <string>
#include <sstream>
#include <rainman/rainman.h>
#include <hzip/errors/archive.h>

template<typename Type>
struct HZ_ArchiveTrieNode {
    bool is_leaf = false;
    HZ_ArchiveTrieNode *parent{};
    std::unordered_map<std::string, HZ_ArchiveTrieNode *> children;
    Type value{};
    std::string key;
};

struct HZ_ArchiveTrieListElement {
    std::string entry{};
    bool is_leaf{};
};


template<typename Type>
class HZ_ArchiveTrie : public rainman::context {
private:
    HZ_ArchiveTrieNode<Type> *root{};

    inline void erase_node(HZ_ArchiveTrieNode<Type> *node) {
        for (auto entry : node->children) {
            erase_node(entry.second);
        }

        rfree(node);
    }

    inline bool is_leaf_node(HZ_ArchiveTrieNode<Type> *node) {
        return node->is_leaf;
    }

    inline bool is_dir_node(HZ_ArchiveTrieNode<Type> *node) {
        return node->children.size() > 0;
    }

public:
    void init() {
        root = rnew(HZ_ArchiveTrieNode<Type>);
    }

    Type get(const std::string &path) {
        std::stringstream ss(path);

        std::string token;

        HZ_ArchiveTrieNode<Type> *curr = root;
        while (std::getline(ss, token, '/')) {
            if (token.empty()) {
                continue;
            }

            if (curr->children.contains(token)) {
                curr = curr->children[token];
            } else {
                throw ArchiveErrors::TargetNotFoundException(path);
            }
        }

        if (curr->children.size() > 0) {
            throw ArchiveErrors::InvalidOperationException("The given path is a directory");
        }

        return curr->value;
    }

    void set(const std::string &path, Type val) {
        std::stringstream ss(path);

        std::string token;

        HZ_ArchiveTrieNode<Type> *curr = root;
        while (std::getline(ss, token, '/')) {
            if (token.empty()) {
                continue;
            }

            if (!curr->children.contains(token)) {
                if (is_leaf_node(curr)) {
                    throw ArchiveErrors::InvalidOperationException("A file exists in the path prefix");
                }

                curr->children[token] = rnew(HZ_ArchiveTrieNode<Type>);
                curr->children[token]->parent = curr;
                curr->children[token]->key = token;
            }

            curr = curr->children[token];
        }

        if (is_dir_node(curr)) {
            throw ArchiveErrors::InvalidOperationException("A directory exists in the given path");
        }

        curr->value = val;
        curr->is_leaf = true;
    }

    bool contains(const std::string &prefix) {
        std::stringstream ss(prefix);

        std::string token;

        HZ_ArchiveTrieNode<Type> *curr = root;
        while (std::getline(ss, token, '/')) {
            if (token.empty()) {
                continue;
            }

            if (curr->children.contains(token)) {
                curr = curr->children[token];
            } else {
                return false;
            }
        }

        return true;
    }

    void erase(const std::string &prefix) {
        std::stringstream ss(prefix);

        std::string token;

        HZ_ArchiveTrieNode<Type> *curr = root;
        while (std::getline(ss, token, '/')) {
            if (token.empty()) {
                continue;
            }

            if (curr->children.contains(token)) {
                curr = curr->children[token];
            } else {
                throw ArchiveErrors::TargetNotFoundException(prefix);
            }
        }

        HZ_ArchiveTrieNode<Type> *descendant = curr;
        while (curr->children.size() < 2 && curr->parent != nullptr) {
            descendant = curr;
            curr = curr->parent;
        }

        if (curr == descendant) {
            curr = curr->parent;
        }

        curr->children.erase(descendant->key);

        erase_node(descendant);
    }

    std::vector<HZ_ArchiveTrieListElement> children(const std::string &prefix) {
        std::stringstream ss(prefix);

        std::string token;

        HZ_ArchiveTrieNode<Type> *curr = root;
        while (std::getline(ss, token, '/')) {
            if (token.empty()) {
                continue;
            }

            if (curr->children.contains(token)) {
                curr = curr->children[token];
            } else {
                throw ArchiveErrors::TargetNotFoundException(prefix);
            }
        }

        std::vector<HZ_ArchiveTrieListElement> keys;

        for (auto entry : curr->children) {
            keys.push_back(HZ_ArchiveTrieListElement{
                    .entry=entry.first,
                    .is_leaf=entry.second->is_leaf,
            });
        }

        return keys;
    }

    std::vector<std::string> all() {
        std::vector<std::string> paths;


    }
};

#endif
