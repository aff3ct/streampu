#include <algorithm>
#include <cassert>
#include <cmath>
#include <iomanip>
#include <ios>

#include "Tools/Display/Terminal/Standard/Terminal_std.hpp"
#include "Tools/Display/rang_format/rang_format.h"
#include "Tools/Exception/exception.hpp"

using namespace aff3ct;
using namespace aff3ct::tools;

const char aff3ct::tools::Terminal_std::line_separator = '-';
const std::string aff3ct::tools::Terminal_std::col_separator = "|";
const std::string aff3ct::tools::Terminal_std::group_separator = "||";
const std::string aff3ct::tools::Terminal_std::spaced_scol_separator = " |";
const std::string aff3ct::tools::Terminal_std::spaced_dcol_separator = " ||";
const std::string aff3ct::tools::Terminal_std::data_tag = "  ";
const rang::style aff3ct::tools::Terminal_std::legend_style = rang::style::bold;
const rang::style aff3ct::tools::Terminal_std::report_style = rang::style::bold;

#ifdef _WIN32
const size_t aff3ct::tools::Terminal_std::def_column_width = 11;
#else
const size_t aff3ct::tools::Terminal_std::def_column_width = 10;
#endif

Terminal_std::Terminal_std(const std::vector<tools::Reporter*>& reporters)
  : Terminal()
  , reporters(reporters)
{
}

Terminal_std::Terminal_std(const std::vector<std::unique_ptr<tools::Reporter>>& reporters)
  : Terminal()
  , reporters()
{
    for (auto const& r : reporters)
        this->reporters.push_back(r.get());
}

std::vector<tools::Reporter*>&
Terminal_std::get_reporters()
{
    return this->reporters;
}

void
Terminal_std::legend(std::ostream& stream) const
{
    std::vector<const Reporter::group_t*> cols_groups;

    for (auto& r : this->reporters)
        if (r != nullptr)
            for (auto& g : r->get_groups())
                cols_groups.push_back(&g);
        else
            throw tools::runtime_error(__FILE__, __LINE__, __func__, "'this->reporters' contains null pointer.");

    std::ios::fmtflags f(stream.flags());

    // clang-format off
    // stream << "# " << "----------------------------------------------||---------------------------------" << std::endl; // line 1
    // stream << "# " << "          cols_groups[0].first.first          ||   cols_groups[1].first.first    " << std::endl; // line 2
    // stream << "# " << "          cols_groups[0].first.second         ||   cols_groups[1].first.second   " << std::endl; // line 3
    // stream << "# " << "----------------------------------------------||---------------------------------" << std::endl; // line 4
    // stream << "# " << "----------|-----------|-----------|-----------||----------|----------|-----------" << std::endl; // line 5
    // stream << "# " << "   (1.1)  |    (2.1)  |    (3.1)  |    (4.1)  ||   (5.1)  |   (6.1)  |   (7.1)   " << std::endl; // line 6
    // stream << "# " << "   (1.2)  |    (2.2)  |    (3.2)  |    (4.2)  ||   (5.2)  |   (6.2)  |   (7.2)   " << std::endl; // line 7
    // stream << "# " << "----------|-----------|-----------|-----------||----------|----------|-----------" << std::endl; // line 8
    // note (1.1) is "cols_groups[0].second[0].first"
    // note (1.2) is "cols_groups[0].second[0].second"
    // note (2.1) is "cols_groups[0].second[1].first"
    // note (2.2) is "cols_groups[0].second[1].second"
    // note (3.1) is "cols_groups[0].second[2].first"
    // note (3.2) is "cols_groups[0].second[2].second"
    // note (4.1) is "cols_groups[0].second[3].first"
    // note (4.2) is "cols_groups[0].second[3].second"
    // note (5.1) is "cols_groups[1].second[0].first"
    // note (5.2) is "cols_groups[1].second[0].second"
    // note (6.1) is "cols_groups[1].second[1].first"
    // note (6.2) is "cols_groups[1].second[1].second"
    // note (7.1) is "cols_groups[1].second[2].first"
    // note (7.2) is "cols_groups[1].second[2].second"
    // clang-format on

    assert(!cols_groups.empty());

    // print line 1 of the table
    stream << rang::tag::comment;
    for (unsigned i = 0; i < cols_groups.size(); i++)
    {
        assert(!cols_groups[i]->second.empty());

        const unsigned group_width = get_group_width(*cols_groups[i]);
        const auto n_separators = group_width + extra_spaces(cols_groups[i]->first, group_width);

        stream << legend_style << std::string(n_separators, line_separator) << rang::style::reset;

        if (i < (cols_groups.size() - 1)) // print group separator except for last
            stream << legend_style << group_separator << rang::style::reset;
    }
    stream << std::endl;

    // print line 2 and 3 of the table (group title lines)
    for (auto l = 0; l < 2; l++)
    {
        stream << rang::tag::comment;
        for (unsigned i = 0; i < cols_groups.size(); i++)
        {
            const auto& text = l == 0 ? std::get<0>(cols_groups[i]->first) : std::get<1>(cols_groups[i]->first);

            const unsigned group_width = get_group_width(*cols_groups[i]);
            int n_spaces = (int)group_width - (int)text.size();

            if (text.size() !=
                std::max(std::get<0>(cols_groups[i]->first).size(), std::get<1>(cols_groups[i]->first).size()))
                n_spaces += extra_spaces(cols_groups[i]->first, group_width);

            const unsigned n_spaces_left = (n_spaces >= 0) ? (unsigned)n_spaces / 2 : 0;
            const unsigned n_spaces_right = (n_spaces >= 0) ? n_spaces - n_spaces_left : 0;

            stream << legend_style << std::string(n_spaces_left, ' ') << rang::style::reset;
            stream << legend_style << text << rang::style::reset;
            stream << legend_style << std::string(n_spaces_right, ' ') << rang::style::reset;

            if (i < (cols_groups.size() - 1)) // print group separator except for last
                stream << legend_style << group_separator << rang::style::reset;
        }
        stream << std::endl;
    }

    // print line 4 of the table
    stream << rang::tag::comment;
    for (unsigned i = 0; i < cols_groups.size(); i++)
    {
        const unsigned group_width = get_group_width(*cols_groups[i]);
        const auto n_separators = group_width + extra_spaces(cols_groups[i]->first, group_width);

        stream << legend_style << std::string(n_separators, line_separator) << rang::style::reset;

        if (i < (cols_groups.size() - 1)) // print group separator except for last
            stream << legend_style << group_separator << rang::style::reset;
    }
    stream << std::endl;

    // print line 5 of the table
    stream << rang::tag::comment;
    for (unsigned i = 0; i < cols_groups.size(); i++)
    {
        const unsigned group_width = get_group_width(*cols_groups[i]);
        const auto n_extra = extra_spaces(cols_groups[i]->first, group_width);

        for (unsigned j = 0; j < cols_groups[i]->second.size(); j++)
        {
            auto n_separators =
              (std::get<2>(cols_groups[i]->second[j])) ? std::get<2>(cols_groups[i]->second[j]) : def_column_width;
            if (j == 0) n_separators += n_extra;

            stream << legend_style << std::string(n_separators, line_separator) << rang::style::reset;
            if (j < (cols_groups[i]->second.size() - 1)) // print column separator except for last
                stream << legend_style << col_separator << rang::style::reset;
        }

        if (i < (cols_groups.size() - 1)) // print group separator except for last
            stream << legend_style << group_separator << rang::style::reset;
    }
    stream << std::endl;

    // print line 6 and 7 of the table (column title lines)
    for (auto l = 0; l < 2; l++)
    {
        stream << rang::tag::comment;
        for (unsigned i = 0; i < cols_groups.size(); i++)
        {
            const unsigned group_width = get_group_width(*cols_groups[i]);
            const auto n_extra = extra_spaces(cols_groups[i]->first, group_width);

            for (unsigned j = 0; j < cols_groups[i]->second.size(); j++)
            {
                const auto& text =
                  l == 0 ? std::get<0>(cols_groups[i]->second[j]) : std::get<1>(cols_groups[i]->second[j]);
                int n_spaces = (int)((std::get<2>(cols_groups[i]->second[j])) ? std::get<2>(cols_groups[i]->second[j])
                                                                              : def_column_width) -
                               (int)text.size() - 1;

                if (j == 0) n_spaces += n_extra;

                if (n_spaces > 0) stream << legend_style << std::string(n_spaces, ' ') << rang::style::reset;

                stream << legend_style << text + " " << rang::style::reset;

                if (j < (cols_groups[i]->second.size() - 1)) // print column separator except for last
                    stream << legend_style << col_separator << rang::style::reset;
            }

            if (i < (cols_groups.size() - 1)) // print group separator except for last
                stream << legend_style << group_separator << rang::style::reset;
        }
        stream << std::endl;
    }

    // print line 8 of the table
    stream << rang::tag::comment;
    for (unsigned i = 0; i < cols_groups.size(); i++)
    {
        const unsigned group_width = get_group_width(*cols_groups[i]);
        const auto n_extra = extra_spaces(cols_groups[i]->first, group_width);

        for (unsigned j = 0; j < cols_groups[i]->second.size(); j++)
        {
            auto n_separators =
              (std::get<2>(cols_groups[i]->second[j])) ? std::get<2>(cols_groups[i]->second[j]) : def_column_width;
            if (j == 0) n_separators += n_extra;

            stream << legend_style << std::string(n_separators, line_separator) << rang::style::reset;
            if (j < (cols_groups[i]->second.size() - 1)) // print column separator except for last
                stream << legend_style << col_separator << rang::style::reset;
        }

        if (i < (cols_groups.size() - 1)) // print group separator except for last
            stream << legend_style << group_separator << rang::style::reset;
    }
    stream << std::endl;

    stream.flags(f);
}

void
Terminal_std::report(std::ostream& stream, bool final)
{
    std::ios::fmtflags f(stream.flags());

    stream << data_tag;

    for (unsigned r = 0; r < this->reporters.size(); r++)
        if (this->reporters[r] != nullptr)
        {
            auto report = this->reporters[r]->report(final);
            auto& groups = this->reporters[r]->get_groups();

            assert(report.size() == groups.size());

            for (unsigned g = 0; g < groups.size(); g++)
            {
                assert(report[g].size() == groups[g].second.size());

                stream << report_style << std::string(extra_spaces(groups[g]), ' ') << rang::style::reset;

                for (unsigned c = 0; c < report[g].size(); c++)
                {
                    auto& text = report[g][c];

                    size_t column_with =
                      std::get<2>(groups[g].second[c]) ? std::get<2>(groups[g].second[c]) : def_column_width;
                    if (text.size() < (size_t)column_with)
                    {
                        text += " ";
                        text.insert(0, column_with - text.size(), ' ');
                    }

                    stream << text;

                    if (c != (report[g].size() - 1)) stream << report_style << col_separator << rang::style::reset;
                }

                if (g != (groups.size() - 1)) stream << report_style << group_separator << rang::style::reset;
            }

            if (r != (this->reporters.size() - 1)) stream << report_style << group_separator << rang::style::reset;
        }
        else
            throw tools::runtime_error(__FILE__, __LINE__, __func__, "'this->reporters' contains null pointer.");

    if (final)
    {
        stream << "  " << std::endl;
    }
    else
    {
        stream << rang::style::bold << rang::fg::green << (real_time_state++ < 2 ? " *" : "  ") << rang::style::reset;
        stream << "\r"; // put the carrier return character only on temporary reports
        real_time_state %= (uint8_t)4;
    }

    stream.flags(f);
    stream.flush();
}

// get extra spaces if text is too long for the given group width
unsigned
Terminal_std::extra_spaces(const Reporter::title_t& text, const unsigned group_width)
{
    const unsigned longest_text = (unsigned)std::max(std::get<0>(text).size(), std::get<1>(text).size());
    return (longest_text > group_width) ? longest_text - group_width : 0;
}

unsigned
Terminal_std::extra_spaces(const Reporter::group_t& group)
{
    return extra_spaces(group.first, get_group_width(group));
}

unsigned
Terminal_std::get_group_width(const Reporter::group_t& group)
{
    size_t group_width = 0;
    for (size_t j = 0; j < group.second.size(); j++)
        group_width += (std::get<2>(group.second[j]) ? std::get<2>(group.second[j]) : def_column_width) + 1;
    return --group_width;
}
