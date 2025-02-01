/**
 * \file Pipeline_builder.hpp
 * \brief Defines the builder for creating a StreamPU pipeline.
 */
#ifndef PIPELINE_BUILDER_HPP_
#define PIPELINE_BUILDER_HPP_

#include "Runtime/Pipeline/Pipeline.hpp"
#include "Runtime/Task/Task.hpp"
#include <vector>

namespace spu
{
namespace tools
{

/**
 * \class Pipeline_builder
 * \brief Class to build and configure a StreamPU pipeline.
 */
class Pipeline_builder
{
  public:
    Pipeline_builder() = default;

    /**
     * Set a list of tasks to check pipeline construction.
     * Previously set or added tasks are replaced.
     * @param tasks A list of pointers to the tasks to be checked.
     * @return A reference to the modified Pipeline_builder object.
     */
    Pipeline_builder& set_tasks_for_checking(const std::vector<runtime::Task*> tasks);

    /**
     * Add a task to the list for pipeline construction checking.
     * @param task A pointer to the task to be added.
     * @return A reference to the modified Pipeline_builder object.
     */
    Pipeline_builder& add_task_for_checking(runtime::Task* task);

    /**
     * @overload add_task_for_checking(runtime::Task* task)
     */
    Pipeline_builder& add_task_for_checking(runtime::Task& task);

    /**
     * Remove a task to the list for pipeline construction checking.
     * @param task A pointer to the task to be added.
     * @return A reference to the modified Pipeline_builder object.
     */
    Pipeline_builder& remove_task_for_checking(const runtime::Task* task);

    /**
     * Get tasks for pipeline construction checking.
     * @return A list of tasks (pointers) for pipeline construction checking.
     */
    const std::vector<runtime::Task*> get_tasks_for_checking();

    /**
     * \class Stage_builder
     * \brief Nested class to configure the stages of the pipeline.
     */
    class Stage_builder;

    /**
     * Add a stage to the pipeline
     * @param stage A reference to a Stage_builder configuring the new stage.
     * @return A reference to the modified Pipeline_builder object.
     */
    Pipeline_builder& add_stage(Stage_builder& stage);

    /**
     * Remove a stage to the pipeline
     * @param stage A reference to a Stage_builder configuring the stage to be removed.
     * @return A reference to the modified Pipeline_builder object.
     */
    Pipeline_builder& remove_stage(const Stage_builder& stage);

    /**
     * Remove a stage by its index to the pipeline
     * @param stage_id Index to a Stage_builder configuring the stage to be removed.
     * @return A reference to the modified Pipeline_builder object.
     */
    Pipeline_builder& remove_stage(const size_t stage_id);

    /**
     * Get a stage by its index.
     * @param stage_id The index of the stage to be retrieved.
     * @return A reference to the Stage_builder object for the stage.
     */
    Stage_builder& get_stage(const size_t stage);

    /**
     * \class Synchro_builder
     * \brief Nested class to configure the synchronization between two stages.
     */
    class Synchro_builder;

    /**
     * Configure the synchronization of two stages of the pipeline
     * @param synchro A reference to a Synchro_builder configuring the new synchronization.
     * @return A reference to the modified Pipeline_builder object.
     */
    Pipeline_builder& configure_interstage_synchro(Synchro_builder& synchro);

    /**
     * Get a synchronization configuration by its index.
     * @param stage_id The index of the synchronization configuration to be retrieved.
     * @return A reference to the Synchro_builder object for the synchronization configuration.
     */
    Synchro_builder& get_interstage_synchro(const size_t synchro_id);

    /**
     * Build a new Pipeline according to the given configuration.
     * @return An instance of the constructed Pipeline.
     */
    runtime::Pipeline build();

    class Stage_builder
    {
      public:
        Stage_builder() = default;

        /**
         * Set the number of threads for this stage.
         * @param n The number of threads.
         * @return A reference to the modified Stage_builder object.
         */
        Stage_builder& set_n_threads(const size_t n);

        /**
         * Get the number of threads for this stage.
         * @return The number of threads.
         */
        const size_t get_n_threads();

        /**
         * Enable thread pinning on this stage.
         * @return A reference to the modified Stage_builder object.
         */
        Stage_builder& enable_threads_pinning();

        /**
         * Disable thread pinning on this stage.
         * @return A reference to the modified Stage_builder object.
         */
        Stage_builder& disable_threads_pinning();

        /**
         * Enable or disable thread pinning on this stage depending on the boolean parameter.
         * @param pinning Enable or disable thread pinning.
         * @return A reference to the modified Stage_builder object.
         */
        Stage_builder& set_threads_pinning(bool pinning);

        /**
         * Check if thread pinning is enabled on this stage.
         * @return True if thread pinning is enabled, false otherwise.
         */
        const bool is_pinning();

        /**
         * Set the thread pinning policy of this stage.
         * @param pinning_policy A string representing the policy.
         * @return A reference to the modified Stage_builder object.
         */
        Stage_builder& set_pinning_policy(const std::string pinning_policy);

        /**
         * Get the thread pinning policy of this stage.
         * @return A string representing the policy.
         */
        const std::string get_pinning_policy();

        /**
         * Set the first tasks of the stage
         * Previously set or added tasks are replaced.
         * @param first A list of the first tasks.
         * @return A reference to the modified Stage_builder object.
         */
        Stage_builder& set_first_tasks(const std::vector<runtime::Task*> firsts);

        /**
         * Add task as one of the first tasks of the stage.
         * @firsts A pointer to the task.
         * @return A reference to the modified Stage_builder object.
         */
        Stage_builder& add_first_task(runtime::Task* first);

        /**
         * @overload add_first_task(runtime::Task* first)
         */
        Stage_builder& add_first_task(runtime::Task& first);

        /**
         * Remove task from the first tasks of the stage.
         * @first A pointer to the task.
         * @return A reference to the modified Stage_builder object.
         */
        Stage_builder& remove_first_task(const runtime::Task* first);

        /**
         * @overload remove_first_task(const runtime::Task* first)
         */
        Stage_builder& remove_first_task(const runtime::Task& first);

        /**
         * Get the list of the first tasks of the stage
         * @return A list of tasks (pointers)
         */
        const std::vector<runtime::Task*> get_first_tasks();

        /**
         * Set the last tasks of the stage
         * Previously set or added tasks are replaced.
         * @param lasts A list of the last tasks.
         * @return A reference to the modified Stage_builder object.
         */
        Stage_builder& set_last_tasks(const std::vector<runtime::Task*> lasts);

        /**
         * Add task as one of the last tasks of the stage.
         * @last A pointer to the task.
         * @return A reference to the modified Stage_builder object.
         */
        Stage_builder& add_last_task(runtime::Task* last);

        /**
         * @overload add_last_task(runtime::Task* last)
         */
        Stage_builder& add_last_task(runtime::Task& last);

        /**
         * Remove task from the last tasks of the stage.
         * @last A pointer to the task.
         * @return A reference to the modified Stage_builder object.
         */
        Stage_builder& remove_last_task(const runtime::Task* last);

        /**
         * @overload remove_last_task(const runtime::Task* last)
         */
        Stage_builder& remove_last_task(const runtime::Task& last);

        /**
         * Get the list of the last tasks of the stage
         * @return A list of tasks (pointers)
         */
        const std::vector<runtime::Task*> get_last_tasks();

        /**
         * Set the excluded tasks of the stage
         * Previously set or added tasks are replaced.
         * @param excluded A list of the excluded tasks.
         * @return A reference to the modified Stage_builder object.
         */
        Stage_builder& set_excluded_tasks(const std::vector<runtime::Task*> excluded);

        /**
         * Add task as one of the excluded tasks of the stage.
         * @excluded A pointer to the task.
         * @return A reference to the modified Stage_builder object.
         */
        Stage_builder& add_excluded_task(runtime::Task* excluded);

        /**
         * @overload add_excluded_task(runtime::Task* excluded)
         */
        Stage_builder& add_excluded_task(runtime::Task& excluded);

        /**
         * Remove task from the excluded tasks of the stage.
         * @excluded A pointer to the task.
         * @return A reference to the modified Stage_builder object.
         */
        Stage_builder& remove_excluded_task(const runtime::Task* excluded);

        /**
         * @overload remove_excluded_task(const runtime::Task* excluded)
         */
        Stage_builder& remove_excluded_task(const runtime::Task& excluded);

        /**
         * Get the list of the excluded tasks of the stage
         * @return A list of tasks (pointers)
         */
        const std::vector<runtime::Task*> get_excluded_tasks();

      protected:
        std::vector<runtime::Task*> first_tasks;
        std::vector<runtime::Task*> last_tasks;
        std::vector<runtime::Task*> excluded_tasks;
        size_t n_threads = 1;
        bool pinning = false;
        std::string pinning_policy = "";
        size_t buffer_size_after = 1;
        bool active_waiting_after = false;
    };

    class Synchro_builder
    {
      public:
        Synchro_builder() = default;

        /**
         * Enable active waiting between the two stages.
         * @return A reference to the modified Synchro_builder object.
         */
        Synchro_builder& enable_active_waiting();

        /**
         * Disable active waiting between the two stages.
         * @return A reference to the modified Synchro_builder object.
         */
        Synchro_builder& disable_active_waiting();

        /**
         * Enable or disable active waiting depending on the boolean parameter.
         * @param pinning Enable or disable active waiting.
         * @return A reference to the modified Stage_builder object.
         */
        Synchro_builder& set_active_waiting(bool waiting);

        /**
         * Check if active waiting is enabled.
         * @return True if active waiting is enabled, false otherwise.
         */
        const bool is_active_waiting();

        /**
         * Set the buffer size for synchronization between the two stages.
         * @param buffer_size The size of the buffer.
         * @return A reference to the modified Synchro_builder object.
         */
        Synchro_builder& set_buffer_size(size_t buffer_size);

        /**
         * Get the buffer size for synchronization between the two stages.
         * @return The size of the buffer between the two stages.
         */
        const size_t get_buffer_size();

      protected:
        bool active_waiting = false;
        size_t buffer_size = 1;
    };

  protected:
    std::vector<Stage_builder*> stages;
    std::vector<Synchro_builder*> synchros;
    std::vector<runtime::Task*> tasks_for_checking;
};

} // end namespace tools
} // end namespace spu

#endif /* PIPELINE_BUILDER_HPP_ */
