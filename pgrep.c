// Michal Kryštof xmicha64
#include <vector>
#include <regex>
#include <thread>
#include <mutex>
#include <utility>
#include <iostream>

long line_score = 0L;
long parameters_count = 0L;
long pat_count = 0L;

bool end = false;

std::mutex cout_mutex;
std::mutex score_mutex;
std::mutex driver_mutex;

void find_match(const unsigned id, const std::string& line, const std::regex& pat, const long pat_score, std::vector<std::mutex>& threads_mutex)
{
    while (!end)
    {
        cout_mutex.lock();

        if (--pat_count <= 0)
            driver_mutex.unlock();

        cout_mutex.unlock();
        threads_mutex[id].lock();

        if (regex_match(line, pat))
        {
            std::unique_lock<std::mutex> lck {score_mutex};
            line_score += pat_score;
        }
    }
}

void control(const long score_min, const std::vector<std::regex>& patterns, const std::vector<long>& pat_scores, std::vector<std::mutex>& threads_mutex)
{
    pat_count = parameters_count;

    // need locked mutex first
    driver_mutex.lock();

    std::vector<std::thread>    threads;
    std::string                 line;

    bool                        no_line {true};

    for (unsigned i = 0; i != parameters_count; i++)
    {
        threads_mutex[i].lock();
        threads.push_back(std::thread {find_match,i, std::ref(line), std::ref(patterns[i]), std::ref(pat_scores[i]), std::ref(threads_mutex)});
    }

    line_score = 0;
    driver_mutex.lock();

    for ( ; ; )
    {
        if (getline(std::cin, line))
            no_line = false;
        else if (!no_line)
            break;

        pat_count = parameters_count;

        for (unsigned i = 0; i != threads.size(); i++)
            threads_mutex[i].unlock();

        if (no_line)
            break;

        driver_mutex.lock();

        {
            std::unique_lock<std::mutex> lck {cout_mutex};
            if (score_min <= line_score)
                std::cout << line << " [ Score: " << line_score << " ]" << '\n'; 
            line_score = 0;
        }
    }

    end = true;

    for (unsigned i = 0; i != threads.size(); i++)
        threads_mutex[i].unlock();

    for (auto &t : threads)
        t.join();
}

std::pair<long, bool> str_to_num(char* s)
{
    long result = 0L;

    if (s != NULL)
    {
        char *endptr {NULL};

        result = strtol(s, &endptr, 10);

        if (endptr == s)
        {
            std::cerr << "Invalid number: |%s|\n" << '\n';
            return std::make_pair(result, false);
        }

    }
    return std::make_pair(result, true);
}

int main(int argc, char *argv[])
{
    if ((argc-1) < 3 || (argc - 2) % 2 != 0)
    {
        std::cerr << "Pocet vstupnich argumentu musi byt 1 + 2i !" << '\n';
        return 1;
    }

    long score_min;
   
    {
       std::pair<long, bool> res {str_to_num(argv[1])};

       if (!res.second)
           return 2;

       score_min = res.first;
    }
    parameters_count = (argc-2) / 2;

    std::vector<std::regex> patterns(parameters_count);
    std::vector<long>       pat_scores(parameters_count);
    std::vector<std::mutex> threads_mutex(parameters_count);


    for (int i = 2, j = 0; i != argc; j++, i += 2)
    {
       std::pair<long, bool> res {str_to_num(argv[i+1])};

       if (!res.second)
           return 2;

       patterns[j] = argv[i];
       pat_scores[j] = res.first;
    }

    std::thread driver {control, score_min, std::ref(patterns), std::ref(pat_scores), std::ref(threads_mutex)};

    driver.join();

    return 0;
}
