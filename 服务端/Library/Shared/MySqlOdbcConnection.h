#pragma once

#include <windows.h>
#include <stdio.h>

inline void BuildMySqlOdbcConnectionString(const char* database_alias,
                                           const char* user_name,
                                           const char* password,
                                           const char* ini_path,
                                           char* output,
                                           unsigned int output_size)
{
    const char* database_name = database_alias;
    if (_stricmp(database_alias, "DragonRajaDB") == 0)
        database_name = "dragonraja";
    else if (_stricmp(database_alias, "TotalDB") == 0)
        database_name = "dragonraja_total";
    else if (_stricmp(database_alias, "ChrLogDB") == 0)
        database_name = "dragonraja_log";
    else if (_stricmp(database_alias, "NgcDB") == 0)
        database_name = "ngcdb";

    char driver_name[128] = "MySQL ODBC 9.7 Unicode Driver";
    char server_name[64] = "127.0.0.1";
    char server_port[16] = "3306";
    char driver_options[64] = "3";
    GetPrivateProfileStringA("mysql_conn", "driver", driver_name, driver_name,
                             sizeof(driver_name), ini_path);
    GetPrivateProfileStringA("mysql_conn", "server", server_name, server_name,
                             sizeof(server_name), ini_path);
    GetPrivateProfileStringA("mysql_conn", "port", server_port, server_port,
                             sizeof(server_port), ini_path);
    GetPrivateProfileStringA("mysql_conn", "option", driver_options,
                             driver_options, sizeof(driver_options), ini_path);
    _snprintf_s(output, output_size, _TRUNCATE,
                "DRIVER={%s};SERVER=%s;PORT=%s;DATABASE=%s;UID=%s;PWD=%s;OPTION=%s;",
                driver_name, server_name, server_port, database_name, user_name,
                password, driver_options);
}
