#ifndef HYBRIDZIP_ARCHIVE_TRIE_H
#define HYBRIDZIP_ARCHIVE_TRIE_H

#include <unordered_map>
#include <string>
#include <sstream>
#include <rainman/rainman.h>
#include <hzip/errors/archive.h>

template<typename Type>
struct hza_trie_node {
    hza_trie_node *parent{};
    std::unordered_map<std::string, hza_trie_node *> children;
    Type value{};
};


template<typename Type>
class hza_trie : public rainman::context {
private:
    hza_trie_node<Type> *root{};

    void erase_node(hza_trie_node<Type> *node) {
        for (auto entry : node->children) {
            erase_node(entry.second);
        }

        rfree(node);
    }

public:
    hza_trie() {
        root = rnew(hza_trie_node<Type>);
    }

    Type get(const std::string &path) {
        std::stringstream ss(path);

        std::string token;

        hza_trie_node<Type> *curr = root;
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

        hza_trie_node<Type> *curr = root;
        while (std::getline(ss, token, '/')) {
            if (token.empty()) {
                continue;
            }

            if (!curr->children.contains(token)) {
                curr->children[token] = rxnew(hza_trie_node<Type>);
                curr->children[token]->parent = curr;
            }

            curr = curr->children[token];
        }

        if (curr->children.size() > 0) {
            throw ArchiveErrors::InvalidOperationException("A directory exists in the given path");
        }

        curr->value = val;
    }

    bool contains(const std::string &prefix) {
        std::stringstream ss(prefix);

        std::string token;

        hza_trie_node<Type> *curr = root;
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

        hza_trie_node<Type> *curr = root;
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

        hza_trie_node<Type> *descendant{};
        while (curr->children.size() < 2 && curr->parent != nullptr) {
            descendant = curr;
            curr = curr->parent;
        }

        erase_node(descendant);
    }

    std::vector<std::string> children(const std::string &prefix) {
        std::stringstream ss(prefix);

        std::string token;

        hza_trie_node<Type> *curr = root;
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

        std::vector<std::string> keys;

        for (auto entry : curr->children) {
            keys.push_back(entry.first);
        }

        return keys;
    }
};

#endif
