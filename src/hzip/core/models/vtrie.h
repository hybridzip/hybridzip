/* Efficient +-order context model for text.
 * Uses trie to build vocabulary.
 */


#ifndef HYBRIDZIP_VTRIE_H
#define HYBRIDZIP_VTRIE_H

#include <map>

class VTrieModel {
private:
    struct VTElem {
        uint64_t count;
        std::map<uint8_t, VTElem *> child;
    };

    VTElem *root, *curr;

    static VTElem *newVTElem(uint64_t count = 0) {
        auto elem = new VTElem;
        elem->count = count;
        elem->child = std::map<uint8_t, VTElem *>();
        return elem;
    }

public:
    VTrieModel() {
        root = newVTElem();
        curr = root;
    }

    void update(uint8_t symbol) {

        if (curr->child[symbol] == nullptr) {
            curr->child[symbol] = newVTElem(1);
        }

        curr = curr->child[symbol];
        curr->count++;

        // End-of-word then return to root node.
        if (symbol == ' ') {
            curr = root;
        }
    }

    VTElem *getDist() {
        return curr;
    }

};

#endif
