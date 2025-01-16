#include "my_serial.hpp"
#include "process_handler.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <vector>
#include <optional>
#include <string>

namespace fs = std::filesystem;

struct ParsedData
{
    double temp = 0;
    bool is_error = false;
};

struct LoggingConfig
{
    static constexpr const char* LOG_DIR = "logs";
    static constexpr const char* LOG_SEC_NAME = "/second.log";
    static constexpr const char* LOG_HOUR_NAME = "/hour.log";
    static constexpr const char* LOG_DAY_NAME = "/day.log";

    static constexpr int64_t HOUR_SEC = 10;
    static constexpr int64_t DAY_SEC = HOUR_SEC * 24;
    static constexpr int64_t MONTH_SEC = DAY_SEC * 30;
    static constexpr int64_t YEAR_SEC = DAY_SEC * 365;
};

struct TrackingData
{
    int64_t last_hour_gather = 0;
    int64_t last_day_gather = 0;
};

int64_t GetUNIXTimeNow()
{
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

std::vector<std::string> SplitString(const std::string &str, char delimiter = ' ')
{
    std::vector<std::string> split;
    std::istringstream iss(str);
    std::string s;
    while (std::getline(iss, s, delimiter))
    {
        split.push_back(s);
    }
    return split;
}

std::string GetLastLine(std::ifstream &fi)
{
    fi.seekg(-1, std::ios_base::end);
    if (fi.peek() == '\n')
    {
        fi.seekg(-1, std::ios_base::cur);
        for (int i = fi.tellg(); i > 0; --i)
        {
            if (fi.peek() == '\n')
            {
                fi.get();
                break;
            }
            fi.seekg(i, std::ios_base::beg);
        }
    }
    std::string last_line;
    std::getline(fi, last_line);
    return last_line;
}

void CreateFileIfNotExists(const std::string &name)
{
    if (!fs::exists(name))
    {
        std::ofstream(name).close();
    }
}

class Logger
{
public:
    explicit Logger(const std::string &log_dir)
    {
        fs::create_directory(log_dir);
        CreateFileIfNotExists(log_dir + LoggingConfig::LOG_SEC_NAME);
        CreateFileIfNotExists(log_dir + LoggingConfig::LOG_HOUR_NAME);
        CreateFileIfNotExists(log_dir + LoggingConfig::LOG_DAY_NAME);
    }

    void WriteTempToFile(const std::string &file_name, double temp, int64_t now, int64_t lim_sec)
    {
        std::string temp_name = std::string(LoggingConfig::LOG_DIR) + "/temp.log";
        if (fs::exists(temp_name))
        {
            fs::remove(temp_name);
        }

        fs::copy_file(file_name, temp_name);
        std::ofstream of_log(file_name);
        std::ifstream temp_log(temp_name);

        std::string line;
        while (std::getline(temp_log, line))
        {
            auto split = SplitString(line);
            if (split.empty())
            {
                continue;
            }

            int64_t time = std::stoll(split[0]);
            if (now - time < lim_sec)
            {
                break;
            }
        }

        if (!temp_log.eof())
        {
            of_log << line << '\n';
        }
        while (std::getline(temp_log, line))
        {
            of_log << line << '\n';
        }
        of_log << now << " " << temp << std::endl;

        of_log.close();
        temp_log.close();
        fs::remove(temp_name);
    }
};

class TemperatureParser
{
public:
    static ParsedData ParseTemperature(const std::string &str)
    {
        int len = str.size();
        int pos_f = 0;
        int pos_r = len - 1;

        for (int i = 0; i < len / 2 + 1; ++i)
        {
            if (str[i] != '$')
            {
                pos_f = i;
                break;
            }
        }

        for (int i = len - 1; i > len / 2 - 1; --i)
        {
            if (str[i] != '$')
            {
                pos_r = i;
                break;
            }
        }

        if (pos_f >= pos_r)
        {
            return {0, true};
        }

        auto str_temp = str.substr(pos_f, pos_r - pos_f);
        try
        {
            return {std::stod(str_temp), false};
        }
        catch (const std::invalid_argument &)
        {
            return {0, true};
        }
    }
};

class Tracker
{
public:
    explicit Tracker(const std::string &log_dir)
    {
        logger_ = std::make_shared<Logger>(log_dir);
    }

    void TrackData()
    {
        auto current_time = GetUNIXTimeNow();
        if (current_time - tracking_data_.last_hour_gather >= LoggingConfig::HOUR_SEC)
        {
            double hour_mean = GetMeanTemp(LoggingConfig::LOG_SEC_NAME, current_time, LoggingConfig::HOUR_SEC);
            logger_->WriteTempToFile(LoggingConfig::LOG_HOUR_NAME, hour_mean, current_time, LoggingConfig::MONTH_SEC);
            tracking_data_.last_hour_gather = current_time;
        }
        if (current_time - tracking_data_.last_day_gather >= LoggingConfig::DAY_SEC)
        {
            double day_mean = GetMeanTemp(LoggingConfig::LOG_HOUR_NAME, current_time, LoggingConfig::DAY_SEC);
            logger_->WriteTempToFile(LoggingConfig::LOG_DAY_NAME, day_mean, current_time, LoggingConfig::YEAR_SEC);
            tracking_data_.last_day_gather = current_time;
        }
    }

    void WriteTempToFile(const std::string &file_name, double temp, int64_t now, int64_t lim_sec) {
        logger_->WriteTempToFile(file_name, temp, now, lim_sec);
    }


private:
    std::shared_ptr<Logger> logger_;
    TrackingData tracking_data_;

    double GetMeanTemp(const std::string &file_name, int64_t now, int64_t diff_sec)
    {
        std::ifstream log(file_name);
        double mean = 0;
        uint32_t count = 0;
        std::string line;

        while (std::getline(log, line))
        {
            auto split = SplitString(line);
            if (split.empty())
            {
                continue;
            }

            int64_t time = std::stoll(split[0]);
            if (now - time < diff_sec)
            {
                mean += std::stod(split[1]);
                ++count;
            }
        }

        return count > 0 ? mean / count : 0;
    }
};

int main(int argc, char **argv)
{
    std::string port = argc > 1 ? argv[1] : "COM4";
    cplib::SerialPort serial_port(port, cplib::SerialPort::BAUDRATE_115200);

    if (!serial_port.IsOpen())
    {
        std::cerr << "Failed to open port: " << port << std::endl;
        return -2;
    }
    std::cout << "Listening to port: " << port << std::endl;

    Tracker tracker(LoggingConfig::LOG_DIR);

    std::string input;
    serial_port.SetTimeout(5.0);

    while (true)
    {
        serial_port >> input;
        auto parsed = TemperatureParser::ParseTemperature(input);
        if (parsed.is_error)
        {
            std::cerr << "Parsing error -- Received: " << input << std::endl;
            continue;
        }
        auto now_time = GetUNIXTimeNow();
        tracker.TrackData();

        tracker.WriteTempToFile(LoggingConfig::LOG_SEC_NAME, parsed.temp, now_time, LoggingConfig::DAY_SEC);
    }

    return 0;
}
