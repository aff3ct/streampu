#include <set>

#include "Module/Set/Set.hpp"
#include "Runtime/Sequence/Sequence.hpp"

namespace spu
{
namespace runtime
{
size_t
Sequence::get_n_threads() const
{
    return this->n_threads;
}

const std::vector<std::vector<runtime::Task*>>&
Sequence::get_firsts_tasks() const
{
    return this->firsts_tasks;
}

const std::vector<std::vector<runtime::Task*>>&
Sequence::get_lasts_tasks() const
{
    return this->lasts_tasks;
}

template<class C>
std::vector<C*>
Sequence::get_modules(const bool set_modules) const
{
    std::vector<C*> ret;
    for (auto& mm : this->all_modules)
        for (auto& m : mm)
        {
            if (set_modules)
            {
                auto c = dynamic_cast<module::Set*>(m);
                if (c != nullptr)
                {
                    auto subret = c->get_sequence().get_modules<C>(set_modules);
                    ret.insert(ret.end(), subret.begin(), subret.end());
                }
            }

            auto c = dynamic_cast<C*>(m);
            if (c != nullptr) ret.push_back(c);
        }

    return ret;
}

template<class C>
std::vector<C*>
Sequence::get_cloned_modules(const C& module_ref) const
{
    bool found = false;
    size_t mid = 0;
    while (mid < this->all_modules[0].size() && !found)
        if (dynamic_cast<C*>(this->all_modules[0][mid]) == &module_ref)
            found = true;
        else
            mid++;

    if (!found)
    {
        std::stringstream message;
        message << "'module_ref' can't be found in the sequence.";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    std::vector<C*> cloned_modules(this->all_modules.size());
    for (size_t tid = 0; tid < this->all_modules.size(); tid++)
    {
        auto c = dynamic_cast<C*>(this->all_modules[tid][mid]);
        if (c == nullptr)
        {
            std::stringstream message;
            message << "'c' can't be 'nullptr', this should never happen.";
            throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
        }
        cloned_modules[tid] = c;
    }
    return cloned_modules;
}

template<class SS>
inline void
Sequence::_init(tools::Digraph_node<SS>* root)
{
    std::stringstream message;
    message << "This should never happen.";
    throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
}

template<>
inline void
Sequence::_init(tools::Digraph_node<runtime::Sub_sequence_const>* root)
{
    this->replicate<runtime::Sub_sequence_const, const module::Module>(root);
    std::vector<tools::Digraph_node<Sub_sequence_const>*> already_deleted_nodes;
    this->delete_tree(root, already_deleted_nodes);
}

template<>
inline void
Sequence::_init(tools::Digraph_node<runtime::Sub_sequence>* root)
{
    std::function<void(tools::Digraph_node<runtime::Sub_sequence>*,
                       std::vector<tools::Digraph_node<runtime::Sub_sequence>*>&)>
      remove_useless_nodes;
    remove_useless_nodes = [&](tools::Digraph_node<runtime::Sub_sequence>* node,
                               std::vector<tools::Digraph_node<runtime::Sub_sequence>*>& already_parsed_nodes)
    {
        if (node != nullptr &&
            std::find(already_parsed_nodes.begin(), already_parsed_nodes.end(), node) == already_parsed_nodes.end())
        {
            auto node_contents = node->get_c();

            if (node->get_parents().size() == 1 && node->get_children().size() == 1 && node_contents->tasks.size() == 0)
            {
                auto parent = node->get_parents().size() ? node->get_parents()[0] : nullptr;
                auto child = node->get_children().size() ? node->get_children()[0] : nullptr;

                auto child_pos = -1;
                if (parent != nullptr)
                {
                    child_pos = node->get_child_pos(*parent);
                    if (child_pos == -1)
                    {
                        std::stringstream message;
                        message << "'child_pos' should be different from '-1'.";
                        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
                    }

                    if (!parent->cut_child((size_t)child_pos))
                    {
                        std::stringstream message;
                        message << "'parent->cut_child(child_pos)' should return true ('child_pos' = " << child_pos
                                << ").";
                        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
                    }
                }

                auto parent_pos = -1;
                if (child != nullptr)
                {
                    parent_pos = node->get_parent_pos(*child);
                    if (parent_pos == -1)
                    {
                        std::stringstream message;
                        message << "'parent_pos' should be different from '-1'.";
                        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
                    }

                    if (!child->cut_parent((size_t)parent_pos))
                    {
                        std::stringstream message;
                        message << "'child->cut_parent(parent_pos)' should return true ('parent_pos' = " << parent_pos
                                << ").";
                        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
                    }
                }

                if (node == root) root = child;

                delete node_contents;
                delete node;

                if (child != nullptr && parent != nullptr)
                {
                    parent->add_child(child, child_pos);
                    child->add_parent(parent, parent_pos);
                }

                node = child;
                if (child != nullptr) node_contents = node->get_c();
            }

            if (node != nullptr &&
                std::find(already_parsed_nodes.begin(), already_parsed_nodes.end(), node) == already_parsed_nodes.end())
            {
                already_parsed_nodes.push_back(node);

                if (node->get_parents().size())
                {
                    size_t min_depth = node->get_parents()[0]->get_depth();
                    for (size_t f = 1; f < node->get_parents().size(); f++)
                        min_depth = std::min(min_depth, node->get_parents()[f]->get_depth());
                    node->set_depth(min_depth + 1);
                }
                else
                    node->set_depth(0);

                for (auto c : node->get_children())
                    remove_useless_nodes(c, already_parsed_nodes);
            }
        }
    };
    std::vector<tools::Digraph_node<runtime::Sub_sequence>*> already_parsed_nodes1;
    remove_useless_nodes(root, already_parsed_nodes1);

    std::function<void(tools::Digraph_node<runtime::Sub_sequence>*,
                       size_t&,
                       std::vector<tools::Digraph_node<runtime::Sub_sequence>*>&,
                       std::map<tools::Digraph_node<runtime::Sub_sequence>*, std::pair<size_t, size_t>>&)>
      init_ss_ids_rec;
    init_ss_ids_rec =
      [&](tools::Digraph_node<runtime::Sub_sequence>* node,
          size_t& ssid,
          std::vector<tools::Digraph_node<runtime::Sub_sequence>*>& already_parsed_nodes,
          std::map<tools::Digraph_node<runtime::Sub_sequence>*, std::pair<size_t, size_t>>& select_parents)
    {
        if (node != nullptr &&
            std::find(already_parsed_nodes.begin(), already_parsed_nodes.end(), node) == already_parsed_nodes.end())
        {
            already_parsed_nodes.push_back(node);
            auto node_contents = node->get_c();
            node_contents->id = ssid++;

            for (auto c : node->get_children())
                switch (c->get_c()->type)
                {
                    case subseq_t::SELECT:
                    {
                        if (select_parents.find(c) == select_parents.end())
                        {
                            size_t n_parents = 0;
                            for (auto f : c->get_parents())
                            {
                                if (f->get_depth() < c->get_depth()) n_parents++;
                            }
                            select_parents[c] = std::make_pair(0, n_parents);
                        }
                        std::get<0>(select_parents[c]) += 1;

                        if (std::get<0>(select_parents[c]) == std::get<1>(select_parents[c]))
                            init_ss_ids_rec(c, ssid, already_parsed_nodes, select_parents);
                        break;
                    }
                    case subseq_t::COMMUTE:
                    {
                        init_ss_ids_rec(c, ssid, already_parsed_nodes, select_parents);
                        break;
                    }
                    case subseq_t::STD:
                    {
                        init_ss_ids_rec(c, ssid, already_parsed_nodes, select_parents);
                        break;
                    }
                    default:
                    {
                        std::stringstream message;
                        message << "This should never happen.";
                        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
                    }
                };
        }
    };
    std::map<tools::Digraph_node<runtime::Sub_sequence>*, std::pair<size_t, size_t>> select_parents;
    std::vector<tools::Digraph_node<runtime::Sub_sequence>*> already_parsed_nodes2;
    size_t ssid = 0;
    init_ss_ids_rec(root, ssid, already_parsed_nodes2, select_parents);

    this->sequences[0] = root;

    std::set<module::Module*> modules_set;
    std::function<void(const tools::Digraph_node<runtime::Sub_sequence>*,
                       std::vector<const tools::Digraph_node<runtime::Sub_sequence>*>&)>
      collect_modules_list;
    collect_modules_list = [&](const tools::Digraph_node<runtime::Sub_sequence>* node,
                               std::vector<const tools::Digraph_node<runtime::Sub_sequence>*>& already_parsed_nodes)
    {
        if (node != nullptr &&
            std::find(already_parsed_nodes.begin(), already_parsed_nodes.end(), node) == already_parsed_nodes.end())
        {
            already_parsed_nodes.push_back(node);
            if (node->get_c())
                for (auto ta : node->get_c()->tasks)
                    modules_set.insert(&ta->get_module());
            for (auto c : node->get_children())
                collect_modules_list(c, already_parsed_nodes);
        }
    };
    std::vector<const tools::Digraph_node<runtime::Sub_sequence>*> already_parsed_nodes3;
    collect_modules_list(root, already_parsed_nodes3);

    for (auto m : modules_set)
        this->all_modules[0].push_back(m);

    this->replicate<runtime::Sub_sequence, module::Module>(root);
}

size_t
Sequence::get_n_frames() const
{
    const auto n_frames = this->all_modules[0][0]->get_n_frames();

    for (auto& mm : this->all_modules)
        for (auto& m : mm)
            if (m->get_n_frames() != n_frames)
            {
                std::stringstream message;
                message << "All the modules do not have the same 'n_frames' value ('m->get_n_frames()' = "
                        << m->get_n_frames() << ", 'n_frames' = " << n_frames << ").";
                throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
            }

    return n_frames;
}

}
}
