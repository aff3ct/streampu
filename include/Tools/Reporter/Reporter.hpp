/*!
 * \file
 * \brief Class tools::Reporter.
 */
#ifndef REPORTER_HPP__
#define REPORTER_HPP__

#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "Tools/Interface/Interface_reset.hpp"

namespace aff3ct
{
namespace tools
{
class Reporter : public Interface_reset
{
  public:
    using title_t = std::tuple<std::string, std::string, size_t>;
    using group_title_t = title_t;
    using column_titles_t = std::vector<title_t>;
    using group_t = std::pair<group_title_t, column_titles_t>;

    using report_t = std::vector<std::vector<std::string>>;

  protected:
    std::vector<group_t> cols_groups;

  public:
    virtual ~Reporter() = default;

    const std::vector<group_t>& get_groups() const;

    /*
     * Report a vector as long as the number of groups (cols_groups.size())
     * with each sub vector as long as the number of columns of the matching group (cols_groups[i].second.size())
     */
    virtual report_t report(bool final = false) = 0;

    virtual void init(); // do nothing by default

    virtual void reset();
};
}
}

#endif // REPORTER_HPP__