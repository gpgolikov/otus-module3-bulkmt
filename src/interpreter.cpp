#include "interpreter.h"

#include <string>
#include <vector>
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <fstream>

#include <range/v3/all.hpp>

#include "reader.h"
#include "reader_subscriber.h"
#include "statement.h"

namespace griha {

namespace {

struct Worker : ReaderSubscriber {

    struct Metrics {
        size_t nblocks;
        size_t nstatements;
    };
    
    std::vector<Metrics> thread_metrics;
    std::vector<std::thread> thread_pool;
    std::mutex guard;
    std::condition_variable cv_bulks;
    std::list<StatementContainer> bulks;
    bool stopped { false };

    template <typename Job>
    Worker(size_t nthreads, Job&& job) 
        : thread_metrics(nthreads, {0, 0}) {
        thread_pool.reserve(nthreads);
        for (auto i = 0u; i < nthreads; ++i) {
            thread_pool.push_back(std::thread {
                std::ref(*this), 
                std::forward<Job>(job), std::ref(thread_metrics[i])
            });
        }
    }

    ~Worker() {
        join();
    }

    template <typename Job>
    void operator ()(Job&& job, Metrics& metrics) {
        auto exit = false;
        while (!exit) {
            std::unique_lock<std::mutex> l { guard };
            cv_bulks.wait(l, [this] {
                return stopped || !bulks.empty();
            });

            exit = stopped;
            std::list<StatementContainer> bulks_local;
            std::swap(bulks, bulks_local);

            l.unlock();

            for (auto& stms : bulks_local) {
                job(stms);

                // calculate metrics
                ++metrics.nblocks;
                metrics.nstatements += stms.size();
            }
        }
    }


    void send(StatementContainer stms) {
        {
            std::lock_guard<std::mutex> l { guard };
            bulks.push_back(std::move(stms));
        }
        cv_bulks.notify_one();
    }

    void stop() {
        {
            std::lock_guard<std::mutex> l { guard };
            stopped = true;
        }
        cv_bulks.notify_all();
    }

    void join() {
        for (auto& t : thread_pool)
            if (t.joinable())
                t.join();
    }

    void on_block(const StatementContainer& stms) override {
        send(stms);
    }

    void on_unexpected_eof(const StatementContainer&) override {

    }

};

void log_job(const StatementContainer& stms) {
    using namespace std;
    using namespace ranges;

    struct Logger : Executer {
        ostream_joiner<std::string> osj { cout, ", " };
        void execute(const SomeStatement &stm) override {
            *osj = stm.value();
        }
    } logger;

    cout << "bulk: ";
    for (auto& stm : stms)
        stm->execute(logger);
    endl(cout);
}

void file_job(const StatementContainer& stms) {
    using namespace std;
    
    struct Printer : Executer {
        ofstream output;
        void execute(const SomeStatement &stm) override {
            output << stm.value() << endl;
        }
    };
    
    const auto now = chrono::system_clock::now();
    const auto now_ns = chrono::duration_cast<chrono::nanoseconds>(now.time_since_epoch());
    const auto filename = "bulk"s + to_string(now_ns.count()) + ".log"s;

    Printer printer;

    printer.output.open(filename);

    for (auto& stm : stms)
        stm->execute(printer);

    printer.output.flush();
    printer.output.close();
}

} // unnamed namespace

void Interpreter::run(std::istream& input, size_t block_size, size_t nthreads) {
    using WorkerPtr = std::shared_ptr<Worker>;

    Reader reader { block_size };

    WorkerPtr log_worker = std::make_shared<Worker>(1u, log_job);
    WorkerPtr file_worker = std::make_shared<Worker>(nthreads, file_job);

    reader.subscribe(log_worker);
    reader.subscribe(file_worker);
    
    auto reader_metrics = reader.run(input);

    // stop workers
    log_worker->stop();
    file_worker->stop();
    // wait for completing
    log_worker->join();
    file_worker->join();

    // print metrics
    std::clog << "Metrics" << std::endl;
    std::clog << "\tReader:" << std::endl;
    std::clog
        << "\t\tlines - " << reader_metrics.nlines
        << "; statements - " << reader_metrics.nstatements
        << "; blocks - " << reader_metrics.nblocks
        << std::endl;
    
    std::clog << "\tLog:" << std::endl;
    std::clog
        << "\t\tblocks - " << log_worker->thread_metrics[0].nblocks
        << "; statements - " << log_worker->thread_metrics[0].nstatements
        << std::endl;

    std::clog << "\tFiles:" << std::endl;
    for (auto i = 0u; i < file_worker->thread_metrics.size(); ++i) {
        auto &m = file_worker->thread_metrics[i];
        std::clog
            << "\t#" << i
            << "\tblocks - " << m.nblocks
            << "; statements - " << m.nstatements
            << std::endl;
    }
}

} // namespace griha