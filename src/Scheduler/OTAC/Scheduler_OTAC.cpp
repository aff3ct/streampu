#include "Scheduler/OTAC/Scheduler_OTAC.hpp"
#include "Tools/Exception/exception.hpp"
#include <cmath>
#include <iostream>
#include <limits>
#include <sstream>

using namespace spu;
using namespace spu::sched;

/*#define VERBOSE*/

Scheduler_OTAC::Scheduler_OTAC(runtime::Sequence& sequence, const size_t R)
  : Scheduler(&sequence)
  , R(R)
  , P(std::numeric_limits<double>::infinity())
{
}

Scheduler_OTAC::Scheduler_OTAC(runtime::Sequence* sequence, const size_t R)
  : Scheduler(sequence)
  , R(R)
  , P(std::numeric_limits<double>::infinity())
{
}

// USEFUL FUNCTIONS
// Weight of a sub-sequence (stage)
double
weight(const std::vector<runtime::Task*>& s, const unsigned int r)
{
    if (r == 0)
    {
        return std::numeric_limits<double>::infinity();
    }
    else
    {
        double sum = 0.;
        for (auto& t : s)
        {
            sum = sum + (t->get_duration_avg()).count();
        }
        return sum / r;
    }
}

double
weight_t(const std::vector<task_desc_t>& s, const unsigned int r)
{
    if (r == 0)
    {
        return std::numeric_limits<double>::infinity();
    }
    else
    {
        double sum = 0.;
        for (auto& t : s)
        {
            sum = sum + (t.tptr->get_duration_avg()).count();
        }
        return sum / r;
    }
}

// Function to find the index of an element
int
getIndex(const std::vector<task_desc_t>& chain, const task_desc_t target)
{
    int index = -1;
    for (size_t i = 0; i < chain.size(); i++)
    {
        if (chain[i].tptr == target.tptr)
        {
            index = i;
        }
    }
    return index;
}

// Is the subsequence replicable?
bool
is_replicable(const std::vector<runtime::Task*>& s)
{
    for (auto& t : s)
    {
        if (!t->is_replicable())
        {
            return false;
        }
    }
    return true;
}

// return the maximal execution time among Tf (stateful task)
double
max_stateful_weight(const std::vector<task_desc_t>& chain)
{
    double max = 0.0;

    for (auto& t : chain)
    {
        if (!t.tptr->is_replicable())
        {
            if ((t.tptr->get_duration_avg()).count() > max)
            {
                max = (t.tptr->get_duration_avg()).count();
            }
        }
    }
    return max;
}

// return the maximal execution time among the tasks
double
max_weight_t(const std::vector<task_desc_t>& tasks)
{
    double max = 0.0;
    int size = tasks.size();
    if (size >= 1)
    {
        for (auto& t : tasks)
        {
            if ((t.tptr->get_duration_avg()).count() > max)
            {
                max = (t.tptr->get_duration_avg()).count();
            }
        }
    }
    return max;
}

// PACKING FUNCTIONS
// Main loop packing (inplace)
void
main_loop_packing(const std::vector<task_desc_t>& chain, const double P, int& e, std::vector<runtime::Task*>& s, int& n)
{
    int N = chain.size();

    while ((e <= N) && (weight(s, 1) + chain[e - 1].exec_duration.count() <= P))
    {
#ifdef VERBOSE
        std::cout << "main loop packing: weight = " << weight(s, 1);
        std::cout << " new_task=" << chain[e - 1].exec_duration.count();
        std::cout << " weight+new_task= " << (weight(s, 1) + chain[e - 1].exec_duration.count());
        std::cout << " P = " << P << std::endl;
#endif
        s.push_back(chain[e - 1].tptr);
        n += 1;
        e += 1;
    }
}

// Packing if the current stage is stateless
int
stateless_packing(const std::vector<task_desc_t>& chain, const int e, std::vector<runtime::Task*>& s, int& n)
{
    int N = chain.size();
    int f = e;
    while ((f <= N) && chain[f - 1].tptr->is_replicable())
    {
        s.push_back(chain[f - 1].tptr);
        n += 1;
        f += 1;
    }
    return f;
}

// New packing with the current sub-sequence to find extra tasks (f >= e)
int
extra_tasks_packing(const std::vector<task_desc_t>& chain,
                    const double P,
                    const int f,
                    std::vector<runtime::Task*>& s,
                    int& n)
{
    std::vector<runtime::Task*> s_temp;
    s_temp.push_back(chain[f - 1].tptr);
    int e = f;
    while ((chain[e - 2].exec_duration.count() + weight(s_temp, 1)) <= P)
    {
        s.pop_back();
        n -= 1;
        s_temp.push_back(chain[e - 1].tptr);
        e -= 1;
    }
    return e;
}

// If taking tasks from the predecessor with less ressources (ri-1) is a success
void
improved_packing(const std::vector<task_desc_t>& chain,
                 const double P,
                 int& e,
                 std::vector<runtime::Task*>& s,
                 int& n,
                 int& r)
{
    r -= 1;
    int N = chain.size();
    while ((e <= N) && (weight(s, r) + chain[e - 1].exec_duration.count() / r <= P))
    {
        s.push_back(chain[e - 1].tptr);
        n += 1;
        e += 1;
    }
}

// Else, go back to the previous configuration
void
go_back_packing(const std::vector<task_desc_t>& chain, const int f, int& e, std::vector<runtime::Task*>& s, int& n)
{
    while (e != f)
    {
        s.push_back(chain[e - 1].tptr);
        n += 1;
        e += 1;
    }
}

void
print_solution(const std::vector<std::pair<size_t, size_t>>& solution, const std::string tag)
{
    std::cout << tag << ": {";
    for (auto& pair_s : solution)
    {
        std::cout << "(" << pair_s.first << ", " << pair_s.second << ")";
    }
    std::cout << "}" << std::endl;
}

bool
Scheduler_OTAC::PROBE(const std::vector<task_desc_t>& chain,
                      const size_t R,
                      double& P,
                      std::vector<std::pair<size_t, size_t>>& solution)
{
    solution.clear();
#ifdef VERBOSE
    std::cout << "--" << std::endl;
#endif
    // Initial state
    int i = 0; // current sub-sequence index
    // int b = 0; // begin index of the current sub sequence
    int e = 1; // end index of the current sub sequence
    int f;
    int n; // nunber of tasks by sub-sequence (in solution)
    int r; // number of resources by sub-sequence (in solution)
    std::pair<size_t, size_t> tmp_nr;
    std::vector<double> w;
    for (auto& t : chain)
    {
        w.push_back(t.exec_duration.count());
    }
    int N = w.size(); // number of tasks in the chain
    float maxWeight = 0;
    std::vector<std::vector<runtime::Task*>> sequence; // sub-sequence list (containing s)

    // Loop to create packing
    while (e <= N)
    {
        i = i + 1;
        // int list_index = i - 1;
        int b = e;
        e = b + 1;

        // New loop
        std::vector<runtime::Task*> s;
        s.push_back(chain[b - 1].tptr);
        r = 1;
        n = 1;

        // Add tasks to si while they fit
        main_loop_packing(chain, P, e, s, n);

        // Resources needed for a packing
        r = std::ceil(weight(s, 1) / P);
#ifdef VERBOSE
        std::cout << "r=" << r << " weight=" << weight(s, 1) << " P=" << P << std::endl;
#endif

        // All tasks do not fit with the last packing and if replicable
        if ((e <= N) && is_replicable(s))
        {
            f = stateless_packing(chain, e, s, n);
            // Processors needed for a packing
            r = std::ceil(weight(s, 1) / P);
            if (f > N)
            {
                e = f;
            }
            if (e != f)
            {
                e = extra_tasks_packing(chain, P, f, s, n);
                if (weight(s, r - 1) <= P)
                {
                    improved_packing(chain, P, e, s, n, r);
                }
                // No benefit from taking tasks
                else
                {
                    go_back_packing(chain, f, e, s, n);
                }
            }
        }
        sequence.push_back(s);
        tmp_nr.first = n; //
        tmp_nr.second = r;
        solution.push_back(tmp_nr);
        float w = weight(s, r);
        if (w >= maxWeight)
        {
            maxWeight = w;
        }
    }
#ifdef VERBOSE
    print_solution(solution, "Probe");
#endif
    size_t sum = 0;
    for (auto& nr : solution)
    {
        sum += nr.second;
    }
    if (sum <= R)
    {
        P = maxWeight;
        return true;
    }
    else
    {
        return false;
    }
}

void
Scheduler_OTAC::SOLVE(const std::vector<task_desc_t>& chain,
                      const size_t R,
                      double& P,
                      std::vector<std::pair<size_t, size_t>>& solution)
{
    if (R == 0)
    {
        std::stringstream message;
        message << "The number of ressources R has to be higher than 0!";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    if (!solution.empty())
    {
        std::stringstream message;
        message << "'solution' should be empty!";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    double maxTf = max_stateful_weight(chain);
    double maxWeight = max_weight_t(chain);
    double eps = 1 / (double)R;
    double Pmin = weight_t(chain, R) > maxTf ? weight_t(chain, R) : maxTf;
    double Pmax = Pmin + maxWeight;
    std::vector<std::pair<size_t, size_t>> solution_current;
#ifdef VERBOSE
    std::cout << "Pmin=" << Pmin << " Pmax=" << Pmax << " maxWeight=" << maxWeight << std::endl;
#endif

    if (R == 1)
    {
        P = Pmax;
        std::pair<size_t, size_t> pair_r1;
        pair_r1.first = chain.size();
        pair_r1.second = R;
        solution.push_back(pair_r1);
    }
    else
    {
        P = (Pmax + Pmin) / 2;
        std::vector<std::pair<size_t, size_t>> solution_tmp;
        while ((Pmax - Pmin) > eps && P > maxTf) //
        {
            bool is_there_a_solution = this->PROBE(chain, R, P, solution_tmp);
            if (is_there_a_solution == true)
            {
                Pmax = P;
                solution_current = solution_tmp;
#ifdef VERBOSE
                print_solution(solution_tmp, "Pmax " + std::to_string(Pmax) + " Pmin " + std::to_string(Pmin));
#endif
            }
            else
            {
                Pmin = P;
            }
            P = (Pmax + Pmin) / 2;
        }
        solution = solution_current;
    }
    // print_solution(solution, "# Solution stages {(n,r)}");
    /*return new std::vector<std::pair<size_t, size_t>>(solution); // solution_returned*/;
}

void
Scheduler_OTAC::schedule()
{
    if (this->tasks_desc.empty())
    {
        std::stringstream message;
        message << "'tasks_desc' cannot be empty, you need to execute the 'Scheduler::profile()' method first!";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    if (!this->solution.empty()) this->solution.clear();
    this->SOLVE(this->tasks_desc, this->R, this->P, this->solution);
}

double
Scheduler_OTAC::get_period() const
{
    if (this->P == std::numeric_limits<double>::infinity())
    {
        std::stringstream message;
        message << "You cannot get the period before executing the 'Scheduler::schedule()' method!";
        throw tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    return this->P;
}

void
Scheduler_OTAC::reset()
{
    Scheduler::reset();
    this->P = std::numeric_limits<double>::infinity();
}
