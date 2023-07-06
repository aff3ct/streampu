
#include <iostream>
#include <cstdlib>
#include <memory>
#include <vector>
#include <string>
#include <atomic>
#include <getopt.h>

#include <aff3ct-core.hpp>
using namespace aff3ct;
using namespace aff3ct::runtime;

int main(int argc, char** argv)
{
	option longopts[] = {
		{"n-threads", required_argument, NULL, 't'},
		{"n-inter-frames", required_argument, NULL, 'f'},
		{"sleep-time", required_argument, NULL, 's'},
		{"data-length", required_argument, NULL, 'd'},
		{"n-exec", required_argument, NULL, 'e'},
		{"dot-filepath", required_argument, NULL, 'o'},
		{"copy-mode", no_argument, NULL, 'c'},
		{"print-stats", no_argument, NULL, 'p'},
		{"step-by-step", no_argument, NULL, 'b'},
		{"debug", no_argument, NULL, 'g'},
		{"subseq", no_argument, NULL, 'u'},
		{"verbose", no_argument, NULL, 'v'},
		{"help", no_argument, NULL, 'h'},
		{0}};

	size_t n_threads = 1;//std::thread::hardware_concurrency();
	size_t n_inter_frames = 1;
	size_t sleep_time_us = 0;
	size_t data_length = 1000000;
	size_t n_exec = 100;
	std::string dot_filepath;
	bool no_copy_mode = true;
	bool print_stats = false;
	bool step_by_step = false;
	bool debug = false;
	bool subseq = false;
	bool verbose = false;

	while (1)
	{
		const int opt = getopt_long(argc, argv, "t:f:s:d:e:o:cpbguvh", longopts, 0);
		if (opt == -1)
			break;
		switch (opt)
		{
			case 't':
				n_threads = atoi(optarg);
				break;
			case 'f':
				n_inter_frames = atoi(optarg);
				break;
			case 's':
				sleep_time_us = atoi(optarg);
				break;
			case 'd':
				data_length = atoi(optarg);
				break;
			case 'e':
				n_exec = atoi(optarg);
				break;
			case 'o':
				dot_filepath = std::string(optarg);
				break;
			case 'c':
				no_copy_mode = false;
				break;
			case 'p':
				print_stats = true;
				break;
			case 'b':
				step_by_step = true;
				break;
			case 'g':
				debug = true;
				break;
			case 'u':
				subseq = true;
				break;
			case 'v':
				verbose = true;
				break;
			case 'h':
				std::cout << "usage: " << argv[0] << " [options]" << std::endl;
				std::cout << std::endl;
				std::cout << "  -t, --n-threads       "
				          << "Number of threads to run in parallel                                  "
				          << "[" << n_threads << "]" << std::endl;
				std::cout << "  -f, --n-inter-frames  "
				          << "Number of frames to process in one task                               "
				          << "[" << n_inter_frames << "]" << std::endl;
				std::cout << "  -s, --sleep-time      "
				          << "Sleep time duration in one task (microseconds)                        "
				          << "[" << sleep_time_us << "]" << std::endl;
				std::cout << "  -d, --data-length     "
				          << "Size of data to process in one task (in bytes)                        "
				          << "[" << data_length << "]" << std::endl;
				std::cout << "  -e, --n-exec          "
				          << "Number of sequence executions                                         "
				          << "[" << n_exec << "]" << std::endl;
				std::cout << "  -o, --dot-filepath    "
				          << "Path to dot output file                                               "
				          << "[" << (dot_filepath.empty() ? "empty" : "\"" + dot_filepath + "\"") << "]" << std::endl;
				std::cout << "  -c, --copy-mode       "
				          << "Enable to copy data in sequence (performance will be reduced)         "
				          << "[" << (no_copy_mode ? "true" : "false") << "]" << std::endl;
				std::cout << "  -b, --step-by-step    "
				          << "Enable step-by-step sequence execution (performance will be reduced)  "
				          << "[" << (step_by_step ? "true" : "false") << "]" << std::endl;
				std::cout << "  -p, --print-stats     "
				          << "Enable to print per task statistics (performance will be reduced)     "
				          << "[" << (print_stats ? "true" : "false") << "]" << std::endl;
				std::cout << "  -g, --debug           "
				          << "Enable task debug mode (print socket data)                            "
				          << "[" << (debug ? "true" : "false") << "]" << std::endl;
				std::cout << "  -u, --subseq          "
				          << "Enable subsequence in the executed sequence                           "
				          << "[" << (subseq ? "true" : "false") << "]" << std::endl;
				std::cout << "  -v, --verbose         "
				          << "Enable verbose mode                                                   "
				          << "[" << (verbose ? "true" : "false") << "]" << std::endl;
				std::cout << "  -h, --help            "
				          << "This help                                                             "
				          << "[false]" << std::endl;
				exit(0);
				break;
			default:
				break;
		}
	}

	std::cout << "#################################" << std::endl;
	std::cout << "# Micro-benchmark: Simple chain #" << std::endl;
	std::cout << "#################################" << std::endl;
	std::cout << "#" << std::endl;

	std::cout << "# Command line arguments:" << std::endl;
	std::cout << "#   - n_threads      = " << n_threads << std::endl;
	std::cout << "#   - n_inter_frames = " << n_inter_frames << std::endl;
	std::cout << "#   - sleep_time_us  = " << sleep_time_us << std::endl;
	std::cout << "#   - data_length    = " << data_length << std::endl;
	std::cout << "#   - n_exec         = " << n_exec << std::endl;
	std::cout << "#   - dot_filepath   = " << (dot_filepath.empty() ? "[empty]" : dot_filepath.c_str()) << std::endl;
	std::cout << "#   - no_copy_mode   = " << (no_copy_mode ? "true" : "false") << std::endl;
	std::cout << "#   - print_stats    = " << (print_stats ? "true" : "false") << std::endl;
	std::cout << "#   - step_by_step   = " << (step_by_step ? "true" : "false") << std::endl;
	std::cout << "#   - debug          = " << (debug ? "true" : "false") << std::endl;
	std::cout << "#   - subseq         = " << (subseq ? "true" : "false") << std::endl;
	std::cout << "#   - verbose        = " << (verbose ? "true" : "false") << std::endl;
	std::cout << "#" << std::endl;

	// modules creation
	module::Initializer<uint8_t> initializer(data_length);
	module::Finalizer  <uint8_t> finalizer  (data_length);

    // Définition des module d'incrémentation standard/IO
	std::vector<std::shared_ptr<module::Incrementer_io<uint8_t>>> incs_io(10);
    std::vector<std::shared_ptr<module::Incrementer<uint8_t>>> incs(20); 
	
    // Initialisation des modules inout_output!
    for (size_t s = 0; s < incs.size(); s++)
	{
        incs[s].reset(new module::Incrementer<uint8_t>(data_length));
        incs[s]->set_ns(sleep_time_us * 1000);
        incs[s]->set_custom_name("Inc" + std::to_string(s));
	}

	// Initialisation des modules IO

	 for (size_t s = 0; s < incs_io.size(); s++)
	{
		incs_io[s].reset(new module::Incrementer_io<uint8_t>(data_length));
		incs_io[s]->set_ns(sleep_time_us * 1000);
        incs_io[s]->set_custom_name("Inc_io" + std::to_string(s));
        
	}

	std::shared_ptr<runtime::Sequence> partial_sequence;
	std::shared_ptr<module::Subsequence> subsequence;

	// sockets binding
	if (!subseq)
	{
		(*incs[0])[module::inc::sck::increment::in] = initializer[module::ini::sck::initialize::out]; // Création de l'initialize et bind à la première socket !
		 // On bind la moitié des input/output entre elle !
		size_t s = 0;
		for (s; s < incs.size()/2 - 1; ++s)
			(*incs[s+1])[module::inc::sck::increment::in] = (*incs[s])[module::inc::sck::increment::out];

		// Réalisation de la connection hybride ! 
		(*incs_io[0])[module::inc_io::sck::increment_io::inout] =  (*incs[s])[module::inc::sck::increment::out]; 
		
		// Bind des IO entre elle 
		size_t s_io =0;
		for (s_io; s_io < incs_io.size() -1; ++s_io)
			(*incs_io[s_io+1])[module::inc_io::sck::increment_io::inout] = (*incs_io[s_io])[module::inc_io::sck::increment_io::inout];
		
		// Réalisation de la seconde connection
		s++;
		(*incs[s])[module::inc::sck::increment::in] = (*incs_io[s_io])[module::inc_io::sck::increment_io::inout];

		// Réalisation de la seconde interconnection ! 
		for (s; s < incs.size() - 1; ++s)
			(*incs[s+1])[module::inc::sck::increment::in] = (*incs[s])[module::inc::sck::increment::out];


		finalizer[module::fin::sck::finalize::in] = (*incs[incs.size()-1])[module::inc::sck::increment::out]; // Connection à la socket finalizer !
	}
    // Code à ne pas modifier pour le moment car pas intéressant !
	else
	{
		/*for (size_t s = 0; s < incs.size() -1; s++)
			(*incs[s+1])[module::inc_io::sck::increment_io::inout] = (*incs[s])[module::inc_io::sck::increment_io::inout];

		partial_sequence.reset(new runtime::Sequence((*incs[0])[module::inc_io::tsk::increment_io],
		                                             (*incs[incs.size() -1])[module::inc_io::tsk::increment_io]));
		subsequence.reset(new module::Subsequence(*partial_sequence));
		(*subsequence)[module::ssq::tsk::exec    ][ 0] = initializer   [module::ini::sck::initialize::out];
		finalizer     [module::fin::sck::finalize::in] = (*subsequence)[module::ssq::tsk::exec      ][  1];*/
	}

	runtime::Sequence sequence_chain(initializer[module::ini::tsk::initialize], n_threads);
	sequence_chain.set_n_frames(n_inter_frames);
	sequence_chain.set_no_copy_mode(no_copy_mode);

	auto tid = 0;
	for (auto cur_initializer : sequence_chain.get_cloned_modules<module::Initializer<uint8_t>>(initializer))
	{
		std::vector<std::vector<uint8_t>> init_data(n_inter_frames, std::vector<uint8_t>(data_length, 0));
		for (size_t f = 0; f < n_inter_frames; f++)
			std::fill(init_data[f].begin(), init_data[f].end(), tid * n_inter_frames +f);
		cur_initializer->set_init_data(init_data);
		tid++;
	}

	if (!dot_filepath.empty())
	{
		std::ofstream file(dot_filepath);
		sequence_chain.export_dot(file);
	}

	// configuration of the sequence tasks
	for (auto& mod : sequence_chain.get_modules<module::Module>(false)) for (auto& tsk : mod->tasks)
	{
		tsk->reset          (           );
		tsk->set_debug      (debug      ); // disable the debug mode
		tsk->set_debug_limit(16         ); // display only the 16 first bits if the debug mode is enabled
		tsk->set_stats      (print_stats); // enable the statistics
		tsk->set_fast       (true       ); // enable the fast mode (= disable the useless verifs in the tasks)
	}

	if (verbose)
	{
		std::cout << std::endl;
		std::cout << "Helper information:" << std::endl;
		std::cout << "-------------------" << std::endl;
		aff3ct::tools::help(initializer);
		for (size_t s = 0; s < incs.size(); s++)
			aff3ct::tools::help(*incs[s]);
		aff3ct::tools::help(finalizer);
	}

		/*----------------------------------------------------------------------------------------------------------------------*/
	// On essaye de récuprer une matrice de sockets FWD pour l'utiliser dans le pipeline 
	std::vector<runtime::Socket*> liste_fwd;

	// Il faut parcourir les thrads
	for (size_t i=0 ; i< sequence_chain.get_tasks_per_threads().size();++i){
		
		// Il faut parcourir les tasks de chaque threaed
		for(size_t j=incs.size()/2+1; j<sequence_chain.get_tasks_per_threads()[i].size(); ++j ){
			auto task = sequence_chain.get_tasks_per_threads()[i][j];

			// Si la tâche ne contient pas de socket inout => pas la peine de continuer à verifier les autres tâche qui viennent après
			// L'hypothèse est vrai dans le cas où le parcours respecte l'ordre du bind
			if (task->get_n_inout_sockets() == 0)
				break;
			for (auto socket : task->sockets){
				if (socket.get()->get_type() == socket_t::SINOUT){
					liste_fwd.push_back(socket.get());
				}
			}
		}
	}


	std::cout << "Le nombre de FWD trouvé est : " << liste_fwd.size() << std::endl; 

	/*----------------------------------------------------------------------------------------------------------------------*/


	std::atomic<unsigned int> counter(0);
	auto t_start = std::chrono::steady_clock::now();
	if (!step_by_step)
	{
		// execute the sequence (multi-threaded)       
		sequence_chain.exec([&counter, n_exec]() { return ++counter >= n_exec; });
	}
	else
	{
		do
			for (size_t tid = 0; tid < n_threads; tid++)
				while (sequence_chain.exec_step(tid));
		while (++counter < (n_exec / n_threads));
	}

	std::chrono::nanoseconds duration = std::chrono::steady_clock::now() - t_start;

	auto elapsed_time = duration.count() / 1000.f / 1000.f;
	std::cout << "Sequence elapsed time: " << elapsed_time << " ms" << std::endl;

	size_t chain_sleep_time = 0;
	for (auto &inc : incs)
		chain_sleep_time += inc->get_ns();

	auto theoretical_time = (chain_sleep_time * n_exec * n_inter_frames) / 1000.f / 1000.f / n_threads;
	std::cout << "Sequence theoretical time: " << theoretical_time << " ms" << std::endl;

	// verification of the sequence execution
	bool tests_passed = true;
	tid = 0;
	
	for (auto cur_finalizer : sequence_chain.get_cloned_modules<module::Finalizer<uint8_t>>(finalizer))
	{
		for (size_t f = 0; f < n_inter_frames; f++)
		{
			const auto &final_data = cur_finalizer->get_final_data()[f];
			for (size_t d = 0; d < final_data.size(); d++)
			{
				auto expected = (int)(incs.size()+incs_io.size() + (tid * n_inter_frames +f));
				expected = expected % 256;
				if (final_data[d] != expected)
				{
					std::cout << "# expected = " << +expected << " - obtained = "
					          << +final_data[d] << " (d = " << d << ", tid = " << tid << ")" << std::endl;
					tests_passed = false;
				}
			}
		}
		tid++;
	}
	
	if (tests_passed)
		std::cout << "# " << rang::style::bold << rang::fg::green << "Tests passed!" << rang::style::reset << std::endl;
	else
		std::cout << "# " << rang::style::bold << rang::fg::red << "Tests failed :-(" << rang::style::reset << std::endl;

	unsigned int test_results = !tests_passed;

	// display the statistics of the tasks (if enabled)
	if (print_stats)
	{
		std::cout << "#" << std::endl;
		tools::Stats::show(sequence_chain.get_modules_per_types(), true, false);
	}

	// sockets unbinding
	sequence_chain.set_n_frames(1);
	if (!subseq)
	{
		(*incs[0])[module::inc::sck::increment::in].unbind(initializer[module::ini::sck::initialize::out]);
		for (size_t s = 0; s < incs.size()/2 -1; s++)
			(*incs[s+1])[module::inc::sck::increment::in].unbind((*incs[s])[module::inc::sck::increment::out]);
		finalizer[module::fin::sck::finalize::in].unbind((*incs[incs.size()-1])[module::inc::sck::increment::out]);
	}
	else
	{
		/*for (size_t s = 0; s < incs.size() -1; s++)
			(*incs[s+1])[module::inc_io::sck::increment_io::inout].unbind((*incs[s])[module::inc_io::sck::increment_io::inout]);
		(*subsequence)[module::ssq::tsk::exec    ][ 0].unbind(initializer   [module::ini::sck::initialize::out]);
		finalizer     [module::fin::sck::finalize::in].unbind((*subsequence)[module::ssq::tsk::exec      ][  1]);*/
	}

	return test_results;
}