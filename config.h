#ifndef CONFIG_H
#define CONFIG_H
#pragma once // reads the variables at comp time
#include <QCoreApplication>

namespace Config
{
    /*
     * declare every variable that is needed at comp time (we use docker to deploy it)
     * you need to set them all in your docker build command if you don't it will fail
     */
    #ifdef ENV_USER_DATA_PATH
        static constexpr const char* USER_DATA_PATH = ENV_USER_DATA_PATH;
    #else
        static constexpr const char* USER_DATA_PATH = "Z:/TEMP";
    #endif
    #ifdef ENV_DEFAULT_PATH_SRC
            static constexpr const char* DEFAULT_PATH_SRC = ENV_DEFAULT_PATH_SRC;
    #else
            static constexpr const char* DEFAULT_PATH_SRC = "Z:/copy";
    #endif
    #ifdef ENV_DEFAULT_JSON_PATH
        static constexpr const char* DEFAULT_JSON_PATH = ENV_DEFAULT_JSON_PATH;
    #else
        static constexpr const char* DEFAULT_JSON_PATH = "C:/temps/App-Directory-Source";
    #endif
    #ifdef ENV_DEFAULT_IMAGES_PATH
            static constexpr const char* DEFAULT_IMAGES_PATH = ENV_DEFAULT_IMAGES_PATH;
    #else
            static constexpr const char* DEFAULT_IMAGES_PATH = "C:/temps/App-Directory-Source";
    #endif
    #ifdef ENV_CUSTOM_DATA_PATH
        static constexpr const char* CUSTOM_DATA_PATH = ENV_CUSTOM_DATA_PATH;
    #else
        static constexpr const char* CUSTOM_DATA_PATH = "Z:/TEMP";
    #endif

    #ifdef ENV_USER_GUIDE_PATH
            static constexpr const char* USER_GUIDE_PATH = ENV_USER_GUIDE_PATH;
    #else
        static constexpr const char* USER_GUIDE_PATH = "C:/Users/local_user/Documents/Guide/index.html";
    #endif

    static constexpr const char* CUSTOM_FILENAME = "custom.json";
    static constexpr const char* DEFAULT_FILENAME = "default.json";

    #ifdef ENV_SPECIFIC_FONT
        #pragma message("ENV_SPECIFIC_FONT is defined: " ENV_SPECIFIC_FONT)
        static constexpr const bool HAS_CUSTOM_FONT = true;
        static constexpr const char* SPECIFIC_FONT = ENV_SPECIFIC_FONT;
    #else
        #pragma message("ENV_SPECIFIC_FONT is NOT defined")
        static constexpr const bool HAS_CUSTOM_FONT = false;
        static constexpr const char* SPECIFIC_FONT = "Liberation Sans";
    #endif
}

#endif // CONFIG_H
