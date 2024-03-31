#include <utility>
#include <iomanip>
#include <sstream>

#include "Module/Probe/Probe.hpp"

#include "Tools/Reporter/Probe/Reporter_probe.hpp"

using namespace aff3ct;
using namespace aff3ct::tools;

Reporter_probe
::Reporter_probe(const std::string &group_name, const std::string &group_description)
: Reporter(), n_frames(1), mtx(100)
{
	if (group_name.empty())
	{
		std::stringstream message;
		message << "'group_name' can't be empty.";
		throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
	}

	this->cols_groups.push_back(std::make_pair(std::make_tuple(group_name, group_description, 0),
	                            std::vector<Reporter::title_t>()));
}

Reporter_probe
::Reporter_probe(const std::string &group_name)
: Reporter_probe(group_name, "")
{
}

void Reporter_probe
::probe(const module::AProbe& probe, const void *data, const size_t frame_id)
{
	std::string col_name = probe.get_col_name();
	const int col = this->name_to_col[col_name];
	// bool can_push = false;
	     if (this->datatypes[col] == typeid(double  )) /* can_push = */ this->push<double  >(col, ( double* )data);
	else if (this->datatypes[col] == typeid(float   )) /* can_push = */ this->push<float   >(col, ( float*  )data);
	else if (this->datatypes[col] == typeid( int64_t)) /* can_push = */ this->push< int64_t>(col, ( int64_t*)data);
	else if (this->datatypes[col] == typeid(uint64_t)) /* can_push = */ this->push<uint64_t>(col, (uint64_t*)data);
	else if (this->datatypes[col] == typeid( int32_t)) /* can_push = */ this->push< int32_t>(col, ( int32_t*)data);
	else if (this->datatypes[col] == typeid(uint32_t)) /* can_push = */ this->push<uint32_t>(col, (uint32_t*)data);
	else if (this->datatypes[col] == typeid( int16_t)) /* can_push = */ this->push< int16_t>(col, ( int16_t*)data);
	else if (this->datatypes[col] == typeid(uint16_t)) /* can_push = */ this->push<uint16_t>(col, (uint16_t*)data);
	else if (this->datatypes[col] == typeid( int8_t )) /* can_push = */ this->push< int8_t >(col, ( int8_t* )data);
	else if (this->datatypes[col] == typeid(uint8_t )) /* can_push = */ this->push<uint8_t >(col, (uint8_t* )data);
	else
	{
		std::stringstream message;
		message << "Unsupported type.";
		throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
	}
}

void Reporter_probe
::reset()
{
	std::fill(this->head.begin(), this->head.end(), 0);
	std::fill(this->tail.begin(), this->tail.end(), 0);
	for (size_t p = 0; p < this->probes.size(); p++)
		this->probes[p]->reset();
}

template <typename T>
bool Reporter_probe
::format_values(const int col, std::stringstream &temp_stream)
{
	std::vector<T> buff(this->data_sizes[col]);
	const auto can_pull = this->pull<T>(col, buff.data());
	if (this->data_sizes[col] > 1 && can_pull) temp_stream << "[";
	for (size_t v = 0; v < this->data_sizes[col] && can_pull; v++)
	{
		const std::string s = std::string((v != 0) ? ", " : "") + std::string((buff[v] >= 0) ? " " : "");
		temp_stream << std::setprecision(this->precisions[col]) << s << +buff[v];
	}
	if (this->data_sizes[col]> 1 && can_pull) temp_stream << "]";
	return can_pull;
}

Reporter::report_t Reporter_probe
::report(bool final)
{
	if (this->cols_groups[0].second.size() == 0)
	{
		std::stringstream message;
		message << "'cols_groups[0].second.size()' has to be greater than 0 ('cols_groups[0].second.size()' = "
		        << this->cols_groups[0].second.size() << ").";
		throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
	}

	Reporter::report_t the_report(this->cols_groups.size());
	auto& probe_report = the_report[0];
	if (final)
	{
		bool can_pull;
		do
		{
			can_pull = false;
			std::vector<std::stringstream> streams(this->buffer.size());
			for (size_t col = 0; col < this->buffer.size(); col++)
			{
				std::stringstream temp_stream;
				temp_stream.flags(this->format_flags[col]);
				bool c_pull = false;
				     if (this->datatypes[col] == typeid(double  )) c_pull = format_values<double  >(col, temp_stream);
				else if (this->datatypes[col] == typeid(float   )) c_pull = format_values<float   >(col, temp_stream);
				else if (this->datatypes[col] == typeid( int64_t)) c_pull = format_values< int64_t>(col, temp_stream);
				else if (this->datatypes[col] == typeid(uint64_t)) c_pull = format_values<uint64_t>(col, temp_stream);
				else if (this->datatypes[col] == typeid( int32_t)) c_pull = format_values< int32_t>(col, temp_stream);
				else if (this->datatypes[col] == typeid(uint32_t)) c_pull = format_values<uint32_t>(col, temp_stream);
				else if (this->datatypes[col] == typeid( int16_t)) c_pull = format_values< int16_t>(col, temp_stream);
				else if (this->datatypes[col] == typeid(uint16_t)) c_pull = format_values<uint16_t>(col, temp_stream);
				else if (this->datatypes[col] == typeid( int8_t )) c_pull = format_values< int8_t >(col, temp_stream);
				else if (this->datatypes[col] == typeid(uint8_t )) c_pull = format_values<uint8_t >(col, temp_stream);
				else
				{
					std::stringstream message;
					message << "Unsupported type.";
					throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
				}

				streams[col] << std::setprecision(this->precisions[col] +1) << temp_stream.str();

				can_pull = can_pull || c_pull;
			}

			if (can_pull)
				for (auto &s : streams)
					probe_report.push_back(s.str());
		}
		while (can_pull);

		return the_report;
	}
	else
	{
		for (size_t f = 0; f < this->get_n_frames(); f++)
		{
			for (size_t col = 0; col < this->buffer.size(); col++)
			{
				std::stringstream stream, temp_stream;
				temp_stream.flags(this->format_flags[col]);
				     if (this->datatypes[col] == typeid(double  )) format_values<double  >(col, temp_stream);
				else if (this->datatypes[col] == typeid(float   )) format_values<float   >(col, temp_stream);
				else if (this->datatypes[col] == typeid( int64_t)) format_values< int64_t>(col, temp_stream);
				else if (this->datatypes[col] == typeid(uint64_t)) format_values<uint64_t>(col, temp_stream);
				else if (this->datatypes[col] == typeid( int32_t)) format_values< int32_t>(col, temp_stream);
				else if (this->datatypes[col] == typeid(uint32_t)) format_values<uint32_t>(col, temp_stream);
				else if (this->datatypes[col] == typeid( int16_t)) format_values< int16_t>(col, temp_stream);
				else if (this->datatypes[col] == typeid(uint16_t)) format_values<uint16_t>(col, temp_stream);
				else if (this->datatypes[col] == typeid( int8_t )) format_values< int8_t >(col, temp_stream);
				else if (this->datatypes[col] == typeid(uint8_t )) format_values<uint8_t >(col, temp_stream);
				else
				{
					std::stringstream message;
					message << "Unsupported type.";
					throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
				}

				stream << std::setprecision(this->precisions[col] +1) << temp_stream.str();
				probe_report.push_back(stream.str());
			}
		}

		return the_report;
	}
}

size_t B_from_datatype(const std::type_index &type)
{
	if (type == typeid(double  )) return 8;
	if (type == typeid(float   )) return 4;
	if (type == typeid(int64_t )) return 8;
	if (type == typeid(uint64_t)) return 8;
	if (type == typeid(int32_t )) return 4;
	if (type == typeid(uint32_t)) return 4;
	if (type == typeid(int16_t )) return 2;
	if (type == typeid(uint16_t)) return 2;
	if (type == typeid(int8_t  )) return 1;
	if (type == typeid(uint8_t )) return 1;

	std::stringstream message;
	message << "Unsupported type.";
	throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
}

void Reporter_probe
::create_probe_checks(const std::string &name)
{
	if (name_to_col.count(name))
	{
		std::stringstream message;
		message << "'name' already exist in this reporter ('name' = " << name << ").";
		throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
	}

	if (this->buffer.size() > 100)
	{
		std::stringstream message;
		message << "'buffer.size()' has to be smaller than 'mtx.size()' ('buffer.size()' = " << this->buffer.size()
		        << ", 'mtx.size()' = " << this->mtx.size() << ").";
		throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
	}
}

void Reporter_probe
::register_probe(module::AProbe& probe, const size_t data_size, const std::type_index data_type,
                 const std::string &unit, const size_t buffer_size, const std::ios_base::fmtflags ff,
                 const size_t precision)
{
	if (probe.get_col_name().empty())
	{
		std::stringstream message;
		message << "'probe->get_col_name()' cannot be empty.";
		throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
	}

	if (buffer_size == 0)
	{
		std::stringstream message;
		message << "'buffer_size' has to be higher than 0.";
		throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
	}

	for (auto p : this->probes)
		if (p == &probe)
		{
			std::stringstream message;
			message << "'probe' has already been registered to this reporter ('probe.get_col_name()' = "
			        << probe.get_col_name() << ").";
			throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
		}

	probe.set_n_frames(this->get_n_frames());
	this->probes               .push_back(&probe);
	this->head                 .push_back(0);
	this->tail                 .push_back(0);
	this->buffer               .push_back(std::vector<std::vector<int8_t>>(this->get_n_frames() * buffer_size,
	                                      std::vector<int8_t>(data_size * B_from_datatype(data_type))));
	this->datatypes            .push_back(data_type);
	this->format_flags         .push_back(ff);
	this->precisions           .push_back(precision);
	this->data_sizes           .push_back(data_size);
	this->cols_groups[0].second.push_back(std::make_tuple(probe.get_col_name(), unit, 0));
	this->name_to_col[probe.get_col_name()] = this->buffer.size() -1;
	this->col_to_name[this->buffer.size() -1] = probe.get_col_name();
}

void Reporter_probe
::set_n_frames(const size_t n_frames)
{
	const size_t old_n_frames = this->get_n_frames();
	if (n_frames != old_n_frames)
	{
		this->n_frames = n_frames;
		for (size_t p = 0; p < this->probes.size(); p++)
		{
			this->probes[p]->set_n_frames(n_frames);
			auto buffer_size = this->buffer[p].size() / old_n_frames;
			this->buffer[p].resize(n_frames * buffer_size,
			                       std::vector<int8_t>(this->data_sizes[p] * B_from_datatype(this->datatypes[p])));
		}
	}
}

size_t Reporter_probe
::get_n_frames() const
{
	return this->n_frames;
}

size_t Reporter_probe
::get_probe_index(const module::AProbe &prb)
{
	for (size_t p = 0; p < probes.size(); p++)
		if (probes[p] == &prb)
			return p;

	std::stringstream message;
	message << "Current probe is not contained in the reporter ('prb.get_name()' = " << prb.get_name()
	        << ", 'Reporter_probe.name' = " << std::get<0>(this->cols_groups[0].first) << ").";
	throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
}

void Reporter_probe
::set_col_unit(const std::string &unit, const module::AProbe &prb)
{
	size_t p = this->get_probe_index(prb);
	std::string name = std::get<0>(this->cols_groups[0].second[p]);
	size_t col_size = std::get<2>(this->cols_groups[0].second[p]);
	this->cols_groups[0].second[p] = std::make_tuple(name, unit, col_size);
}

void Reporter_probe
::set_cols_unit(const std::string &unit)
{
	for (auto &p : this->probes)
		this->set_col_unit(unit, *p);
}

void Reporter_probe
::set_col_buff_size(const size_t buffer_size, const module::AProbe &prb)
{
	size_t p = this->get_probe_index(prb);
	this->buffer[p].resize(this->get_n_frames() * buffer_size,
		                   std::vector<int8_t>(this->data_sizes[p] * B_from_datatype(this->datatypes[p])));
}

void Reporter_probe
::set_cols_buff_size(const size_t buffer_size)
{
	for (auto &p : this->probes)
		this->set_col_buff_size(buffer_size, *p);
}

void Reporter_probe
::set_col_fmtflags(const std::ios_base::fmtflags ff, const module::AProbe &prb)
{
	size_t p = this->get_probe_index(prb);
	this->format_flags[p] = ff;
}

void Reporter_probe
::set_cols_fmtflags(const std::ios_base::fmtflags ff)
{
	for (auto &p : this->probes)
		this->set_col_fmtflags(ff, *p);
}

void Reporter_probe
::set_col_prec(const size_t precision, const module::AProbe &prb)
{
	size_t p = this->get_probe_index(prb);
	this->precisions[p] = precision;
}

void Reporter_probe
::set_cols_prec(const size_t precision)
{
	for (auto &p : this->probes)
		this->set_col_prec(precision, *p);
}

void Reporter_probe
::set_col_size(const size_t col_size, const module::AProbe &prb)
{
	size_t p = this->get_probe_index(prb);
	std::string name = std::get<0>(this->cols_groups[0].second[p]);
	std::string unit = std::get<1>(this->cols_groups[0].second[p]);
	this->cols_groups[0].second[p] = std::make_tuple(name, unit, col_size);
}

void Reporter_probe
::set_cols_size(const size_t col_size)
{
	for (auto &p : this->probes)
		this->set_col_size(col_size, *p);
}

void Reporter_probe
::register_probes(const std::initializer_list<module::AProbe*> &probes)
{
	for (auto p : probes)
	{
		if (p == nullptr)
		{
			std::stringstream message;
			message << "'p' cannot be nullptr.";
			throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
		}
		p->register_reporter(this);
	}
}
