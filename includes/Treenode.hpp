#ifndef TREENODE_HPP
#define TREENODE_HPP

#include <string>
#include <vector>
#include<iostream>

class Treenode{
public:
  Treenode(std::string directive, std::vector<std::string> value);
  ~Treenode();
  void add(Treenode *node);
  std::string &getDirective();
  std::vector<std::string> &getValue();
  std::vector<Treenode *> &getChildren();

private:
  std::string directive;
  std::vector<std::string> value;
  std::vector<Treenode *> children;
};


#endif