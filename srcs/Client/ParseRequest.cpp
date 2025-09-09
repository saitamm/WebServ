#include "../../Includes/Request.hpp"

void trim(string &str, string tr)
{

    string::size_type pos = str.find_first_not_of(tr);
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
Location &matchLocation(const string &uri,  vector<Location> &locations)
{
    size_t maxMatchLength = 0;
    int bestMatch = -1;
    for (size_t i = 0; i < locations.size(); ++i)
    {
        string path = locations[i].getPath();
        if (uri.find(path) != string::npos && path.length() > maxMatchLength)
        {
            bestMatch = i;
            maxMatchLength = path.length();
        }
    }
    if (bestMatch == -1)
        throw NotFoundException();
    return locations[bestMatch];
}


    