#include <not_implemented.h>
#include <map>
#include <set>
#include <iostream>
#include <fstream>
#include "../include/client_logger.h"
#include <regex>

std::map<std::string, std::pair<std::ofstream*, size_t>> client_logger::_streams = std::map<std::string, std::pair<std::ofstream*, size_t>>();

client_logger::client_logger(
        std::map<std::string, std::set<logger::severity>> const &configuration,
        std::string const &format)
{
    for (auto iter = configuration.begin(); iter != configuration.end(); ++iter)
    {
        const std::string &file_path = iter->first;
        const std::set<logger::severity> &severities = iter->second;

        if (file_path == CONSOLE_STREAM)
        {
            _streams_local[CONSOLE_STREAM] = std::make_pair(&std::cout, severities);
            continue;
        }

        if (!_streams.count(file_path))
        {
            std::ofstream *stream = nullptr;

            try
            {
                if(file_path != CONSOLE_STREAM) {
                    stream = new std::ofstream(file_path);

                    if (!stream->is_open()) {
                        throw std::runtime_error("File error: " + file_path);
                    }
                }
            }
            catch (const std::exception&)
            {
                delete stream;

                for (auto del_iter = configuration.begin(); del_iter != iter; ++del_iter)
                {
                    remove_reference(del_iter->first);
                }

                throw;
            }

            _streams[file_path] = std::make_pair(stream, size_t(0));
        }

        _streams_local[file_path] = std::make_pair(_streams[file_path].first, severities);
        ++_streams[file_path].second;
        _format_out = format;
    }
}

client_logger::client_logger(
    client_logger const &other) :
    _format_out(other._format_out),
    _streams_local(other._streams_local)
{
    //дублирование ссылок, везде +1
   for(auto x : _streams_local)
   {
       _streams[x.first].second++;
   }
}

client_logger &client_logger::operator=(
    client_logger const &other)
{
    if (this == &other)
    {
        return *this;
    }

    for (auto record : _streams)
    {
        remove_reference(record.first);
    }

    _format_out = other._format_out;
    _streams_local = other._streams_local;

    for (auto record : _streams_local)
    {
        _streams[record.first].second++;
    }

    return *this;
}

client_logger::client_logger(client_logger &&other) noexcept :
    _format_out(std::move(other._format_out)),
    _streams_local(std::move(other._streams_local))
{

}

client_logger &client_logger::operator=(
    client_logger &&other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    for (auto record : _streams_local)
    {
        remove_reference(record.first);
    }

    _format_out = std::move(other._format_out);
    _streams_local = std::move(other._streams_local);
    return *this;
}

client_logger::~client_logger() noexcept
{
    for (auto record : _streams_local)
    {
        remove_reference(record.first);
    }
}

logger const *client_logger::log(
    const std::string &text,
    logger::severity severity) const noexcept
{

        std::string result = _format_out;
        //--------------
        std::time_t now = std::time(nullptr);
        char date[11];
        char time[9];
        std::strftime(date, sizeof(date), "%d.%m.%Y", std::localtime(&now));
        std::strftime(time, sizeof(time), "%H:%M:%S", std::localtime(&now));
        std::string d(date);
        std::string t(time);
        //--------------
        result = std::regex_replace(result, std::regex("\\%d"), d);
        result = std::regex_replace(result, std::regex("\\%t"), t);
        result = std::regex_replace(result, std::regex("\\%s"), logger::severity_to_string(severity));
        result = std::regex_replace(result, std::regex("\\%m"), text);


        for (auto &[path,value] : _streams_local)
        //ostream*, logger
        {

            auto &stream = path == CONSOLE_STREAM ? std::cout : *(value.first);
            auto &sevs = value.second;
            if(sevs.count(severity))
            {
                stream << result << std::endl;
            }
        }
        return this;
}

void client_logger::remove_reference(const std::string &key)
{
    if(key.empty())
        return;
    auto i = _streams.find(key);
    if(i != _streams.end())
    {
        auto stream_value = i->second;
        stream_value.second--;
        if(stream_value.second == 0)
        {
            stream_value.first->flush();
            //delete
            delete stream_value.first;
            _streams.erase(i);
        }
    }
}