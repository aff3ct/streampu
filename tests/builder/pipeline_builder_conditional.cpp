#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <streampu.hpp>
using namespace spu;
using namespace spu::runtime;

void
remove_and_unbound_stage(tools::Pipeline_builder& builder, const size_t unwanted, const size_t size_incrs_pips)
{
    auto& stage_unwanted = builder.get_stage(unwanted);

    auto& sck_incr_in = (stage_unwanted.get_first_tasks().front()->get_module())["increment::in"];
    auto& sck_incr_out = (stage_unwanted.get_last_tasks().back()->get_module())["increment::out"];
    auto& sck_stage_out = builder.get_stage(unwanted - 1)
                            .get_last_tasks()[0]
                            ->get_module()[(unwanted - 1 == 0) ? "initialize::out" : "increment::out"];
    auto& sck_stage_in = builder.get_stage(unwanted + 1)
                           .get_first_tasks()[0]
                           ->get_module()[(unwanted == size_incrs_pips) ? "finalize::in" : "increment::in"];

    builder.remove_stage(stage_unwanted);

    // Unbinding
    sck_incr_in.unbind(sck_stage_out);
    sck_stage_in.unbind(sck_incr_out);

    // Rebinding
    sck_stage_in = sck_stage_out;
}

int
main(int /*argc*/, char** /*argv*/)
{
    std::cout << "######################################################################" << std::endl;
    std::cout << "# Micro-benchmark: Pipeline width builder - conditional construction #" << std::endl;
    std::cout << "######################################################################" << std::endl;
    std::cout << "#" << std::endl;

    const std::vector<size_t> n_incrs_pips({ 1, 1, 4, 1, 1, 3 });
    const std::vector<size_t> unwanteds({ 5, 4, 2, 1 });

    // Modules
    module::Initializer<int> init(1);

    std::vector<std::vector<std::unique_ptr<module::Incrementer<int>>>> incrs_pips;
    for (size_t i = 0; i < n_incrs_pips.size(); i++)
        incrs_pips.push_back({});

    for (size_t i = 0; i < n_incrs_pips.size(); i++)
    {
        for (size_t j = 0; j < n_incrs_pips[i]; j++)
        {
            incrs_pips[i].emplace_back(new module::Incrementer<int>(1));
        }
    }
    module::Finalizer<int> fini(1);

    // Binding
    for (size_t i = 0; i < n_incrs_pips.size(); i++)
    {
        if (i == 0)
            (*incrs_pips[0][0].get())["increment::in"] = init["initialize::out"];
        else
            (*incrs_pips[i][0].get())["increment::in"] = (*incrs_pips[i - 1].back().get())["increment::out"];

        for (size_t j = 1; j < n_incrs_pips[i]; j++)
        {
            (*incrs_pips[i][j].get())["increment::in"] = (*incrs_pips[i][j - 1].get())["increment::out"];
        }
    }
    fini["finalize::in"] = (*incrs_pips.back().back().get())["increment::out"];

    tools::Pipeline_builder builder;
    tools::Pipeline_builder::Stage_builder stage_init;
    stage_init.add_first_task(init("initialize"))
      .add_last_task(init("initialize"))
      .add_excluded_task(init("initialize"))
      .set_threads_pinning(true)
      .set_pinning_policy("PU_0");
    tools::Pipeline_builder::Synchro_builder synchro_init;
    synchro_init.set_buffer_size(4).set_active_waiting(true);

    builder.add_task_for_checking(init("initialize")).add_stage(stage_init);
    builder.set_tasks_for_checking({ &init("initialize") })
      .get_stage(0)
      .set_last_tasks({ &init("initialize") })  // others should be replaced
      .set_first_tasks({ &init("initialize") }) // others should be replaced
      .set_excluded_tasks({});                  // others should be replaced

    std::vector<std::unique_ptr<tools::Pipeline_builder::Stage_builder>> stages;
    for (size_t i = 0; i < n_incrs_pips.size(); i++)
    {
        stages.emplace_back(new tools::Pipeline_builder::Stage_builder());
        stages[i]
          .get()
          ->add_first_task((*incrs_pips[i].front().get())("increment"))
          .add_last_task((*incrs_pips[i].back().get())("increment"));
        builder.add_stage(*stages[i]);
    }

    tools::Pipeline_builder::Stage_builder stage_fini;
    stage_fini.add_first_task(fini("finalize"))
      .add_last_task(fini("finalize"))
      .set_n_threads(1)
      .set_threads_pinning(false);
    builder.add_stage(stage_fini);

    for (size_t unwanted : unwanteds)
        remove_and_unbound_stage(builder, unwanted, incrs_pips.size());

    auto pipeline = builder.build();
    // std::ofstream fs("pipeline.dot");
    // pipeline.export_dot(fs);

    size_t reduction = 0;
    pipeline.exec(
      [&reduction, &fini]()
      {
          reduction = fini.get_final_data()[0][0];
          return 1;
      });

    bool tests_passed = reduction == 7;
    if (tests_passed)
        std::cout << "# " << rang::style::bold << rang::fg::green << "Tests passed!" << rang::style::reset << std::endl;
    else
        std::cout << "# " << rang::style::bold << rang::fg::red << "Tests failed :-(" << rang::style::reset
                  << std::endl;
    return !tests_passed;
}
