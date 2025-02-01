#include "Tools/Builder/Pipeline_builder.hpp"
#include "Runtime/Pipeline/Pipeline.hpp"

using namespace spu;
using namespace spu::tools;

Pipeline_builder&
Pipeline_builder::set_tasks_for_checking(const std::vector<runtime::Task*> tasks)
{
    this->tasks_for_checking.assign(tasks.begin(), tasks.end());
    return *this;
}

Pipeline_builder&
Pipeline_builder::add_task_for_checking(runtime::Task* task)
{
    if (std::find(this->tasks_for_checking.begin(), this->tasks_for_checking.end(), task) !=
        this->tasks_for_checking.end())
    {
        throw invalid_argument(__FILE__, __LINE__, __func__, "The task is already added in tasks for checking.");
    }

    this->tasks_for_checking.push_back(task);

    return *this;
}

Pipeline_builder&
Pipeline_builder::add_task_for_checking(runtime::Task& task)
{
    return this->add_task_for_checking(&task);
}

Pipeline_builder&
Pipeline_builder::remove_task_for_checking(const runtime::Task* task)
{
    this->tasks_for_checking.erase(std::remove(this->tasks_for_checking.begin(), this->tasks_for_checking.end(), task),
                                   this->tasks_for_checking.end());

    return *this;
}

const std::vector<runtime::Task*>
Pipeline_builder::get_tasks_for_checking()
{
    return this->tasks_for_checking;
}

Pipeline_builder&
Pipeline_builder::add_stage(Stage_builder& stage)
{
    if (std::find(this->stages.begin(), this->stages.end(), &stage) != this->stages.end())
    {
        throw invalid_argument(__FILE__, __LINE__, __func__, "The stage is already added.");
    }

    this->stages.push_back(&stage);

    return *this;
}

Pipeline_builder&
Pipeline_builder::remove_stage(const Stage_builder& stage)
{
    this->stages.erase(std::remove(this->stages.begin(), this->stages.end(), &stage), this->stages.end());

    return *this;
}

Pipeline_builder&
Pipeline_builder::remove_stage(const size_t stage_id)
{
    if (stage_id >= this->stages.size())
    {
        throw length_error(__FILE__, __LINE__, __func__, "No stage of index 'stage_id' exists.");
    }

    this->stages.erase(std::remove(this->stages.begin(), this->stages.end(), this->stages[stage_id]),
                       this->stages.end());

    return *this;
}

Pipeline_builder::Stage_builder&
Pipeline_builder::get_stage(const size_t stage_id)
{
    if (stage_id >= this->stages.size())
    {
        throw length_error(__FILE__, __LINE__, __func__, "No stage of index 'stage_id' exists.");
    }

    return *this->stages[stage_id];
}

Pipeline_builder&
Pipeline_builder::configure_interstage_synchro(Synchro_builder& synchro)
{
    if (std::find(this->synchros.begin(), this->synchros.end(), &synchro) != this->synchros.end())
    {
        throw invalid_argument(__FILE__, __LINE__, __func__, "The interstage synchronization is already added.");
    }

    this->synchros.push_back(&synchro);

    return *this;
}

Pipeline_builder::Synchro_builder&
Pipeline_builder::get_interstage_synchro(const size_t synchro_id)
{
    if (synchro_id >= this->synchros.size())
    {
        throw length_error(__FILE__, __LINE__, __func__, "No interstage synchronization of index 'synchro_id' exists.");
    }

    return *this->synchros[synchro_id];
}

runtime::Pipeline
Pipeline_builder::build()
{
    std::vector<
      std::tuple<std::vector<spu::runtime::Task*>, std::vector<spu::runtime::Task*>, std::vector<spu::runtime::Task*>>>
      built_stages;
    std::vector<size_t> threads;
    std::vector<bool> pinning;
    std::vector<size_t> buffer_sizes;
    std::vector<bool> waitings;
    std::string pinning_policy("");

    // Build stages
    for (auto it = this->stages.begin(); it != this->stages.end(); it++)
    {
        Stage_builder* stage = *it;

        built_stages.push_back(std::make_tuple<std::vector<spu::runtime::Task*>,
                                               std::vector<spu::runtime::Task*>,
                                               std::vector<spu::runtime::Task*>>(
          std::vector<spu::runtime::Task*>(stage->get_first_tasks()),
          std::vector<spu::runtime::Task*>(stage->get_last_tasks()),
          std::vector<spu::runtime::Task*>(stage->get_excluded_tasks())));

        threads.push_back(stage->get_n_threads());
        pinning.push_back(stage->is_pinning());
        pinning_policy += stage->get_pinning_policy();
        if (it < this->stages.end() - 1) pinning_policy += "|";
    }

    // Build synchronization between stages
    Synchro_builder synchro_default;
    for (size_t stage_id = 0; stage_id < this->stages.size() - 1; stage_id++)
    {
        if (stage_id < this->synchros.size())
        {
            buffer_sizes.push_back(this->synchros[stage_id]->get_buffer_size());
            waitings.push_back(this->synchros[stage_id]->is_active_waiting());
        }
        else
        {
            buffer_sizes.push_back(synchro_default.get_buffer_size());
            waitings.push_back(synchro_default.is_active_waiting());
        }
    }

    return runtime::Pipeline(
      this->get_tasks_for_checking(), built_stages, threads, buffer_sizes, waitings, pinning, pinning_policy);
}

Pipeline_builder::Stage_builder&
Pipeline_builder::Stage_builder::set_n_threads(const size_t n)
{
    this->n_threads = n;
    return *this;
}

const size_t
Pipeline_builder::Stage_builder::get_n_threads()
{
    return this->n_threads;
}

Pipeline_builder::Stage_builder&
Pipeline_builder::Stage_builder::enable_threads_pinning()
{
    this->pinning = true;
    return *this;
}

Pipeline_builder::Stage_builder&
Pipeline_builder::Stage_builder::disable_threads_pinning()
{
    this->pinning = false;
    return *this;
}

Pipeline_builder::Stage_builder&
Pipeline_builder::Stage_builder::set_threads_pinning(bool pinning)
{
    return (pinning) ? this->enable_threads_pinning() : this->disable_threads_pinning();
}

const bool
Pipeline_builder::Stage_builder::is_pinning()
{
    return this->pinning;
}

Pipeline_builder::Stage_builder&
Pipeline_builder::Stage_builder::set_pinning_policy(const std::string pinning_policy)
{
    this->pinning_policy = pinning_policy;
    return *this;
}

const std::string
Pipeline_builder::Stage_builder::get_pinning_policy()
{
    return this->pinning_policy;
}

Pipeline_builder::Stage_builder&
Pipeline_builder::Stage_builder::set_first_tasks(const std::vector<runtime::Task*> first)
{
    this->first_tasks.assign(first.begin(), first.end());
    return *this;
}

Pipeline_builder::Stage_builder&
Pipeline_builder::Stage_builder::add_first_task(runtime::Task* first)
{
    if (std::find(this->first_tasks.begin(), this->first_tasks.end(), first) != this->first_tasks.end())
    {
        throw invalid_argument(__FILE__, __LINE__, __func__, "The task is already added in first tasks.");
    }

    this->first_tasks.push_back(first);
    return *this;
}

Pipeline_builder::Stage_builder&
Pipeline_builder::Stage_builder::add_first_task(runtime::Task& first)
{
    return this->add_first_task(&first);
}

Pipeline_builder::Stage_builder&
Pipeline_builder::Stage_builder::remove_first_task(const runtime::Task* first)
{
    this->first_tasks.erase(std::remove(this->first_tasks.begin(), this->first_tasks.end(), first),
                            this->first_tasks.end());

    return *this;
}

Pipeline_builder::Stage_builder&
Pipeline_builder::Stage_builder::remove_first_task(const runtime::Task& first)
{
    return this->remove_first_task(&first);
}

const std::vector<runtime::Task*>
Pipeline_builder::Stage_builder::get_first_tasks()
{
    return this->first_tasks;
}

Pipeline_builder::Stage_builder&
Pipeline_builder::Stage_builder::set_last_tasks(const std::vector<runtime::Task*> last)
{
    this->last_tasks.assign(last.begin(), last.end());
    return *this;
}

Pipeline_builder::Stage_builder&
Pipeline_builder::Stage_builder::add_last_task(runtime::Task* last)
{
    if (std::find(this->last_tasks.begin(), this->last_tasks.end(), last) != this->last_tasks.end())
    {
        throw invalid_argument(__FILE__, __LINE__, __func__, "The task is already added in last tasks.");
    }

    this->last_tasks.push_back(last);
    return *this;
}

Pipeline_builder::Stage_builder&
Pipeline_builder::Stage_builder::add_last_task(runtime::Task& last)
{
    return this->add_last_task(&last);
}

Pipeline_builder::Stage_builder&
Pipeline_builder::Stage_builder::remove_last_task(const runtime::Task* last)
{
    this->last_tasks.erase(std::remove(this->last_tasks.begin(), this->last_tasks.end(), last), this->last_tasks.end());

    return *this;
}

Pipeline_builder::Stage_builder&
Pipeline_builder::Stage_builder::remove_last_task(const runtime::Task& last)
{
    return this->remove_last_task(&last);
}

const std::vector<runtime::Task*>
Pipeline_builder::Stage_builder::get_last_tasks()
{
    return this->last_tasks;
}

Pipeline_builder::Stage_builder&
Pipeline_builder::Stage_builder::set_excluded_tasks(const std::vector<runtime::Task*> excluded)
{
    this->excluded_tasks.assign(excluded.begin(), excluded.end());
    return *this;
}

Pipeline_builder::Stage_builder&
Pipeline_builder::Stage_builder::add_excluded_task(runtime::Task* excluded)
{
    if (std::find(this->excluded_tasks.begin(), this->excluded_tasks.end(), excluded) != this->excluded_tasks.end())
    {
        throw invalid_argument(__FILE__, __LINE__, __func__, "The task is already added in excluded tasks.");
    }

    this->excluded_tasks.push_back(excluded);
    return *this;
}

Pipeline_builder::Stage_builder&
Pipeline_builder::Stage_builder::add_excluded_task(runtime::Task& excluded)
{
    return this->add_excluded_task(&excluded);
}

Pipeline_builder::Stage_builder&
Pipeline_builder::Stage_builder::remove_excluded_task(const runtime::Task* excluded)
{
    this->excluded_tasks.erase(std::remove(this->excluded_tasks.begin(), this->excluded_tasks.end(), excluded),
                               this->excluded_tasks.end());

    return *this;
}

Pipeline_builder::Stage_builder&
Pipeline_builder::Stage_builder::remove_excluded_task(const runtime::Task& excluded)
{
    return this->remove_excluded_task(&excluded);
}

const std::vector<runtime::Task*>
Pipeline_builder::Stage_builder::get_excluded_tasks()
{
    return this->excluded_tasks;
}

Pipeline_builder::Synchro_builder&
Pipeline_builder::Synchro_builder::enable_active_waiting()
{
    this->active_waiting = true;
    return *this;
}

Pipeline_builder::Synchro_builder&
Pipeline_builder::Synchro_builder::disable_active_waiting()
{
    this->active_waiting = false;
    return *this;
}

Pipeline_builder::Synchro_builder&
Pipeline_builder::Synchro_builder::set_active_waiting(bool waiting)
{
    return (waiting) ? this->enable_active_waiting() : this->disable_active_waiting();
}

const bool
Pipeline_builder::Synchro_builder::is_active_waiting()
{
    return this->active_waiting;
}

Pipeline_builder::Synchro_builder&
Pipeline_builder::Synchro_builder::set_buffer_size(size_t buffer_size)
{
    this->buffer_size = buffer_size;
    return *this;
}

const size_t
Pipeline_builder::Synchro_builder::get_buffer_size()
{
    return this->buffer_size;
}