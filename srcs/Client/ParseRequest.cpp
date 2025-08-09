#include "../../Includes/Request.hpp"

void trim(string &str, string tr)
{

    std::string::size_type pos = str.find_first_not_of(tr);
    str.erase(0, pos);
    pos = str.find_last_not_of(tr);
    str.erase(pos + 1);
}
void split(string str, char c, vector<string> &resul)
{
    if (str.find(c) == string::npos)
    {
        resul.push_back(str);
        return;
    }
    resul.push_back(str.substr(0, str.find(c)));
    str.erase(0, str.find(c) + 1);
    resul.push_back(str);
}
Location *matchLocation(const std::string &uri, const std::vector<Location> &locations)
{
    Location *bestMatch = NULL;
    size_t maxMatchLength = 0;
    for (size_t i = 0; i < locations.size(); ++i)
    {
        std::string path = locations[i].getPath();
        if (uri.find(path) != string::npos && path.length() > maxMatchLength)
        {
            bestMatch = new Location(locations[i]);
            maxMatchLength = path.length();
        }
    }
    return bestMatch;
}


