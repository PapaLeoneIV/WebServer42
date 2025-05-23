#include "../includes/Treenode.hpp"
#include "../includes/Logger.hpp"


void Treenode::add(Treenode *node) { children.push_back(node); }
std::string &Treenode::getDirective() { return directive; }
std::vector<std::string> &Treenode::getValue() { return value; }
std::vector<Treenode *> &Treenode::getChildren() { return children; }


Treenode::Treenode(std::string directive, std::vector<std::string> value)
{
  this->directive = directive;
  this->value = value;
}

Treenode::~Treenode(){Logger::info("Treenode Destructor() Called!!!");}
