
#ifndef NOVA_LOGGER_HPP
#define NOVA_LOGGER_HPP

#include <string>
#include <string_view>
#include <sstream>
#include <vector>
#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>
#include <iostream>
#include <chrono>
#include <iomanip>

namespace nova_sim::log {

enum class level : int {
    trace,
    debug,
    info,
    warn,
    err,
    off
};

// Simple internal formatter to replace fmt::format
namespace detail {
    inline void format_helper(std::ostringstream& oss, std::string_view fmt) {
        oss << fmt;
    }

    template<typename T, typename... Args>
    void format_helper(std::ostringstream& oss, std::string_view fmt, T&& first, Args&&... args) {
        size_t pos = fmt.find("{}");
        if (pos != std::string_view::npos) {
            oss << fmt.substr(0, pos);
            oss << first;
            format_helper(oss, fmt.substr(pos + 2), std::forward<Args>(args)...);
        } else {
            oss << fmt;
        }
    }
}

template<typename... Args>
std::string format(std::string_view fmt, Args&&... args) {
    std::ostringstream oss;
    detail::format_helper(oss, fmt, std::forward<Args>(args)...);
    return oss.str();
}

class NovaLogger {
public:
    static NovaLogger& instance() {
        static NovaLogger instance;
        return instance;
    }

    void set_level(level lvl) {
        level_ = lvl;
    }

    void log(level lvl, std::string_view msg) {
        if (lvl < level_ || lvl == level::off) return;

        std::string formatted_msg = prepare_message(lvl, msg);
        
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(std::move(formatted_msg));
        }
        cv_.notify_one();
    }

    void flush() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return queue_.empty(); });
    }

private:
    NovaLogger() : level_(level::info), stop_(false) {
        worker_ = std::thread(&NovaLogger::process_queue, this);
    }

    ~NovaLogger() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stop_ = true;
        }
        cv_.notify_one();
        if (worker_.joinable()) {
            worker_.join();
        }
    }

    std::string prepare_message(level lvl, std::string_view msg) {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::ostringstream oss;
        oss << "[" << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S") 
            << "." << std::setfill('0') << std::setw(3) << ms.count() << "] ";
        
        oss << "[nova] ";

        // Reset fill to space for level string padding
        oss << std::setfill(' ');

        // Add level string and color codes
        const char* color_start = "";
        const char* color_end = "\033[0m";
        const char* level_str = "unknown";

        switch (lvl) {
            case level::trace: level_str = "[trace]"; color_start = "\033[37m"; break; // White
            case level::debug: level_str = "[debug]"; color_start = "\033[36m"; break; // Cyan
            case level::info:  level_str = "[info]";  color_start = "\033[32m"; break; // Green
            case level::warn:  level_str = "[warning]"; color_start = "\033[33m"; break; // Yellow
            case level::err:   level_str = "[error]"; color_start = "\033[31m"; break; // Red
            default: break;
        }

        oss << color_start << std::setw(9) << std::left << level_str << color_end << " " << msg;
        return oss.str();
    }

    void process_queue() {
        while (true) {
            std::string msg;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                cv_.wait(lock, [this] { return !queue_.empty() || stop_; });
                
                if (stop_ && queue_.empty()) return;
                
                msg = std::move(queue_.front());
                queue_.pop();
            }
            std::cout << msg << std::endl;
            if (queue_.empty()) {
                cv_.notify_all(); // For flush()
            }
        }
    }

    level level_;
    std::queue<std::string> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::thread worker_;
    bool stop_;
};

class Stopwatch {
public:
    Stopwatch() : start_(std::chrono::high_resolution_clock::now()) {}
    std::chrono::duration<double> elapsed() const {
        return std::chrono::high_resolution_clock::now() - start_;
    }
    void reset() {
        start_ = std::chrono::high_resolution_clock::now();
    }
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

} // namespace nova_sim::log

#endif // NOVA_LOGGER_HPP
