#include"ConfigFile.hpp"

int main(int ac, char **av)
{
    if (ac != 2)
    {
        cout << "Error: ./Webserv <file.conf> \n";
        return 1;
    }
    ConfigFile config;
    if(config.ParseConfigFile(av[1]) == 1)
        cout << "Config File is incorrect! \n";
}
