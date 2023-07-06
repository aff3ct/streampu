/*!
 * \file
 * \brief AFF3CT-core main header file, include all the other headers.
 *
 * \section LICENSE
 * This file is under MIT license (https://opensource.org/licenses/MIT).
 */

#ifndef AFF3CT_CORE_HPP
#define AFF3CT_CORE_HPP

#ifndef ADAPTOR_1_TO_N_HPP_
#include <Module/Adaptor/Adaptor_1_to_n.hpp>
#endif
#ifndef ADAPTOR_HPP_
#include <Module/Adaptor/Adaptor.hpp>
#endif
#ifndef ADAPTOR_N_TO_1_HPP_
#include <Module/Adaptor/Adaptor_n_to_1.hpp>
#endif
#ifndef BINARYOP_HPP_
#include <Module/Binaryop/Binaryop.hpp>
#endif
#ifndef CONTROLLER_CYCLIC_HPP_
#include <Module/Controller/Controller_cyclic/Controller_cyclic.hpp>
#endif
#ifndef CONTROLLER_HPP_
#include <Module/Controller/Controller.hpp>
#endif
#ifndef CONTROLLER_LIMIT_HPP_
#include <Module/Controller/Controller_limit/Controller_limit.hpp>
#endif
#ifndef CONTROLLER_STATIC_HPP_
#include <Module/Controller/Controller_static/Controller_static.hpp>
#endif
#ifndef DELAYER_HPP_
#include <Module/Delayer/Delayer.hpp>
#endif
#ifndef FINALIZER_HPP_
#include <Module/Finalizer/Finalizer.hpp>
#endif
#ifndef INCREMENTER_HPP_
#include <Module/Incrementer/Incrementer.hpp>
#endif
#ifndef INCREMENTERIO_HPP_
#include <Module/Incrementer_io/Incrementer_io.hpp>
#endif
#ifndef INITIALIZER_HPP_
#include <Module/Initializer/Initializer.hpp>
#endif
#ifndef ITERATOR_HPP_
#include <Module/Iterator/Iterator.hpp>
#endif
#ifndef MODULE_HPP_
#include <Module/Module.hpp>
#endif
#ifndef PROBE_LATENCY_HPP_
#include <Module/Probe/Latency/Probe_latency.hpp>
#endif
#ifndef PROBE_OCCURRENCE_HPP_
#include <Module/Probe/Occurrence/Probe_occurrence.hpp>
#endif
#ifndef PROBE_HPP_
#include <Module/Probe/Probe.hpp>
#endif
#ifndef PROBE_THROUGHPUT_HPP_
#include <Module/Probe/Throughput/Probe_throughput.hpp>
#endif
#ifndef PROBE_TIME_HPP_
#include <Module/Probe/Time/Probe_time.hpp>
#endif
#ifndef PROBE_TIMESTAMP_HPP_
#include <Module/Probe/Timestamp/Probe_timestamp.hpp>
#endif
#ifndef PROBE_VALUE_HPP_
#include <Module/Probe/Value/Probe_value.hpp>
#endif
#ifndef REDUCER_HPP_
#include <Module/Reducer/Reducer.hpp>
#endif
#ifndef RELAYER_HPP_
#include <Module/Relayer/Relayer.hpp>
#endif
#ifndef RELAYERIO_HPP_
#include <Module/Relayer_io/Relayer_io.hpp>
#endif
#ifndef SINK_NO_HPP
#include <Module/Sink/NO/Sink_NO.hpp>
#endif
#ifndef SINK_HPP
#include <Module/Sink/Sink.hpp>
#endif
#ifndef SINK_USER_BINARY_HPP
#include <Module/Sink/User/Sink_user_binary.hpp>
#endif
#ifndef SLEEPER_HPP_
#include <Module/Sleeper/Sleeper.hpp>
#endif
#ifndef SOURCE_AZCW_HPP_
#include <Module/Source/AZCW/Source_AZCW.hpp>
#endif
#ifndef SOURCE_RANDOM_HPP_
#include <Module/Source/Random/Source_random.hpp>
#endif
#ifndef SOURCE_HPP_
#include <Module/Source/Source.hpp>
#endif
#ifndef SOURCE_USER_BINARY_HPP_
#include <Module/Source/User/Source_user_binary.hpp>
#endif
#ifndef SOURCE_USER_HPP_
#include <Module/Source/User/Source_user.hpp>
#endif
#ifndef STATELESS_HPP_
#include <Module/Stateless/Stateless.hpp>
#endif
#ifndef SUBSEQUENCE_HPP_
#include <Module/Subsequence/Subsequence.hpp>
#endif
#ifndef SWITCHER_HPP_
#include <Module/Switcher/Switcher.hpp>
#endif
#ifndef UNARYOP_HPP_
#include <Module/Unaryop/Unaryop.hpp>
#endif
#ifndef PIPELINE_HPP_
#include <Runtime/Pipeline/Pipeline.hpp>
#endif
#ifndef SEQUENCE_HPP_
#include <Runtime/Sequence/Sequence.hpp>
#endif
#ifndef SOCKET_HPP_
#include <Runtime/Socket/Socket.hpp>
#endif
#ifndef TASK_HPP_
#include <Runtime/Task/Task.hpp>
#endif
#ifndef BIT_PACKER_HPP_
#include <Tools/Algo/Bit_packer/Bit_packer.hpp>
#endif
#ifndef DIGRAPH_NODE_HPP_
#include <Tools/Algo/Digraph/Digraph_node.hpp>
#endif
#ifndef COMPUTE_BYTES_H_
#include <Tools/compute_bytes.h>
#endif
#ifndef RANG_FORMAT_H_
#include <Tools/Display/rang_format/rang_format.h>
#endif
#ifndef STATISTICS_HPP_
#include <Tools/Display/Statistics/Statistics.hpp>
#endif
#ifndef TERMINAL_DUMP_HPP_
#include <Tools/Display/Terminal/Dump/Terminal_dump.hpp>
#endif
#ifndef TERMINAL_STD_HPP_
#include <Tools/Display/Terminal/Standard/Terminal_std.hpp>
#endif
#ifndef TERMINAL_HPP_
#include <Tools/Display/Terminal/Terminal.hpp>
#endif
#ifndef CANNOT_ALLOCATE_HPP_
#include <Tools/Exception/cannot_allocate/cannot_allocate.hpp>
#endif
#ifndef DOMAIN_ERROR_HPP_
#include <Tools/Exception/domain_error/domain_error.hpp>
#endif
#ifndef EXCEPTION_HPP_
#include <Tools/Exception/exception.hpp>
#endif
#ifndef INVALID_ARGUMENT_HPP_
#include <Tools/Exception/invalid_argument/invalid_argument.hpp>
#endif
#ifndef LENGTH_ERROR_HPP_
#include <Tools/Exception/length_error/length_error.hpp>
#endif
#ifndef LOGIC_ERROR_HPP_
#include <Tools/Exception/logic_error/logic_error.hpp>
#endif
#ifndef OUT_OF_RANGE_HPP_
#include <Tools/Exception/out_of_range/out_of_range.hpp>
#endif
#ifndef OVERFLOW_ERROR_HPP_
#include <Tools/Exception/overflow_error/overflow_error.hpp>
#endif
#ifndef PROCESSING_ABORTED_HPP_
#include <Tools/Exception/processing_aborted/processing_aborted.hpp>
#endif
#ifndef RANGE_ERROR_HPP_
#include <Tools/Exception/range_error/range_error.hpp>
#endif
#ifndef RUNTIME_ERROR_HPP_
#include <Tools/Exception/runtime_error/runtime_error.hpp>
#endif
#ifndef UNDERFLOW_ERROR_HPP_
#include <Tools/Exception/underflow_error/underflow_error.hpp>
#endif
#ifndef UNIMPLEMENTED_ERROR_HPP_
#include <Tools/Exception/unimplemented_error/unimplemented_error.hpp>
#endif
#ifndef WAITING_CANCELED_HPP_
#include <Tools/Exception/waiting_canceled/waiting_canceled.hpp>
#endif
#ifndef HELP_HPP_
#include <Tools/Help/Help.hpp>
#endif
#ifndef INTERFACE_CLONE_HPP__
#include <Tools/Interface/Interface_clone.hpp>
#endif
#ifndef INTERFACE_GET_SET_N_FRAMES_HPP__
#include <Tools/Interface/Interface_get_set_n_frames.hpp>
#endif
#ifndef INTERFACE_IS_DONE_HPP__
#include <Tools/Interface/Interface_is_done.hpp>
#endif
#ifndef INTERFACE_RESET_HPP__
#include <Tools/Interface/Interface_reset.hpp>
#endif
#ifndef INTERFACE_SET_SEED_HPP__
#include <Tools/Interface/Interface_set_seed.hpp>
#endif
#ifndef INTERFACE_WAITING_HPP__
#include <Tools/Interface/Interface_waiting.hpp>
#endif
#ifndef BINARYOP_H
#include <Tools/Math/binaryop.h>
#endif
#ifndef UNARYOP_H
#include <Tools/Math/unaryop.h>
#endif
#ifndef MATH_UTILS_H
#include <Tools/Math/utils.h>
#endif
#ifndef REPORTER_PROBE_HPP_
#include <Tools/Reporter/Probe/Reporter_probe.hpp>
#endif
#ifndef REPORTER_HPP__
#include <Tools/Reporter/Reporter.hpp>
#endif
#ifndef THREAD_PINNING_HPP
#include <Tools/Thread_pinning/Thread_pinning.hpp>
#endif

#endif