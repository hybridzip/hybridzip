#ifndef HYBRIDZIP_HZDIRTREE_H
#define HYBRIDZIP_HZDIRTREE_H

#include <map>
#include <iostream>
#include <string>
#include <memory.h>
#include <boost/algorithm/string.hpp>
#include <hzip/log/logger.h>

struct HZDirTreeNode {
    std::string name;
    HZDirTreeNode *parent;
    std::map<std::string, HZDirTreeNode *> childMap;

    HZDirTreeNode() {
        parent = nullptr;
        childMap["."] = this;
        setParentNode(nullptr);
    }

    void setParentNode(HZDirTreeNode *_parent) {
        parent = _parent;
        childMap[".."] = parent;
    }

    void setChildNode(HZDirTreeNode *child) {
        childMap[child->name] = child;
    }

    void mapToNode(std::string _name, HZDirTreeNode *child) {
        childMap[_name] = child;
    }
};

class HZDirTree {
private:

public:
    HZDirTreeNode *root;
    HZDirTreeNode *wkdir;

    HZDirTree() {
        root = new HZDirTreeNode;
        root->name = "/";
        wkdir = root;
    };

    void changeDir(std::string dir) {
        // Split dir.
        std::vector<std::string> result;
        boost::split(result, dir, boost::is_any_of("/"));
        for(auto &elem : result) {
            auto nextDir = wkdir->childMap[elem];
            if (nextDir == nullptr) {
                HZLogger::log(LOGTYPE::WARNING, "Invalid Path.");
                HZLogger::setErrStatus(HZERR::HZIP_PATH_ERROR);
                return;
            }
        }
    }
};

#endif
