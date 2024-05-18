#include <algorithm>

#include "Tools/Algo/Digraph/Digraph_node.hpp"

namespace spu
{
namespace tools
{

template<typename T>
Digraph_node<T>::Digraph_node(std::vector<Digraph_node<T>*> parents,
                              std::vector<Digraph_node<T>*> children,
                              T* content,
                              const size_t depth)
  : parents(parents)
  , children(children)
  , contents(content)
  , depth(depth)
{
}

template<typename T>
Digraph_node<T>::~Digraph_node()
{
}

template<typename T>
bool
Digraph_node<T>::is_no_parent() const
{
    return !this->parents.size();
}

template<typename T>
bool
Digraph_node<T>::is_no_child() const
{
    return !this->children.size();
}

template<typename T>
bool
Digraph_node<T>::is_empty() const
{
    return (this->contents == nullptr);
}

template<typename T>
const std::vector<Digraph_node<T>*>&
Digraph_node<T>::get_parents() const
{
    return this->parents;
}

template<typename T>
const std::vector<Digraph_node<T>*>&
Digraph_node<T>::get_children() const
{
    return this->children;
}

template<typename T>
T*
Digraph_node<T>::get_contents() const
{
    return this->contents;
}

template<typename T>
T*
Digraph_node<T>::get_c() const
{
    return this->get_contents();
}

template<typename T>
void
Digraph_node<T>::set_contents(T* contents)
{
    this->contents = contents;
}

template<typename T>
void
Digraph_node<T>::set_depth(const size_t depth)
{
    this->depth = depth;
}

template<typename T>
size_t
Digraph_node<T>::get_depth() const
{
    return this->depth;
}

template<typename T>
int
Digraph_node<T>::get_child_pos(const Digraph_node<T>& parent) const
{
    const auto parent_it = std::find(this->parents.begin(), this->parents.end(), &parent);
    if (parent_it != this->parents.end())
        for (auto c = 0; c < (int)parent.get_children().size(); c++)
            if (parent.get_children()[c] == this) return c;
    return -1;
}

template<typename T>
int
Digraph_node<T>::get_parent_pos(const Digraph_node<T>& child) const
{
    const auto child_it = std::find(this->children.begin(), this->children.end(), &child);
    if (child_it != this->children.end())
        for (auto f = 0; f < (int)child.get_parents().size(); f++)
            if (child.get_parents()[f] == this) return f;
    return -1;
}

template<typename T>
bool
Digraph_node<T>::cut_child(const size_t pos)
{
    if (pos < this->children.size())
    {
        this->children.erase(this->children.begin() + pos);
        return true;
    }
    else
        return false;
}

template<typename T>
bool
Digraph_node<T>::cut_parent(const size_t pos)
{
    if (pos < this->parents.size())
    {
        this->parents.erase(this->parents.begin() + pos);
        return true;
    }
    else
        return false;
}

template<typename T>
bool
Digraph_node<T>::cut_children()
{
    if (this->children.size())
    {
        for (size_t c = 0; c < this->children.size(); c++)
            delete this->children[c];
        this->children.clear();
        return true;
    }
    else
        return false;
}

template<typename T>
void
Digraph_node<T>::add_child(Digraph_node<T>* child, const int pos)
{
    if ((size_t)pos > this->children.size() || pos == -1)
        this->children.push_back(child);
    else
        this->children.insert(this->children.begin() + pos, child);
}

template<typename T>
void
Digraph_node<T>::add_parent(Digraph_node<T>* parent, const int pos)
{
    if ((size_t)pos > this->parents.size() || pos == -1)
        this->parents.push_back(parent);
    else
        this->parents.insert(this->parents.begin() + pos, parent);
}

}
}
