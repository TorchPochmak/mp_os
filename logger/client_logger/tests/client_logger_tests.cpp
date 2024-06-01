#include <gtest/gtest.h>

#include <client_logger.h>

#include <fstream>
#include <set>
#include <filesystem>


int main(
    int argc,
    char *argv[])
{
    std::string config_path = "E:/GitHub/mp_os/logger/client_logger/tests/log_config.json";

    logger_builder *builder = new client_logger_builder("%d %t - [%s] %m");


//    logger *lg = builder->clear()
//            ->add_console_stream(logger::severity::trace)
//            ->add_file_stream("./debug", logger::severity::debug)
//            ->add_file_stream("./debug", logger::severity::information)
//            ->add_file_stream("./debug", logger::severity::debug)
//            ->add_file_stream("./HELPME", logger::severity::critical)
//            ->build();


    logger *lg = builder
        ->transform_with_configuration(config_path, "log_config")
        ->build();

    //logger *lg2 = new client_logger(*lg);

    lg->trace("TRACE!!!");
    lg->debug("DEBUG!!!");
    lg->information("INFORMATION!!!");
    lg->critical("CRIT");
    lg->critical("TOO MUCH");
    delete builder;
    delete lg;
    //delete lg2;
}