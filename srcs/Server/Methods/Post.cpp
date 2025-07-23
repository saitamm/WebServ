#include "../../../Includes/Response.hpp"

int SupportUpload(Response &resp)
{
    if (resp.getRequest()->getLocation()->getUp_store().empty())
        return (1);
    return (0);
}

void handlePost(Response &resp)
{
    if (SupportUpload(resp))
    {
        setCodeStatus(resp, 403);
        return;
    }
    string filename = resp.getRequest()->getFilename();
    ifstream file(filename.c_str());
    std::string buffer((std::istreambuf_iterator<char>(file)),
    std::istreambuf_iterator<char>());
    setCodeStatus(resp, 200);
    srand(time(0));
    stringstream ll;
    string type = resp.getRequest()->getHeadvalue("Content-Type").substr(resp.getRequest()->getHeadvalue("Content-Type").find('/')+1);
    ll << rand();
    string f = ll.str() +"."+ type;
    string Up = resp.getRequest()->getConfigFile().getRoot() + resp.getRequest()->getLocation()->getUp_store() + "/" + f;
    cout << Up << endl;
    std::ofstream out(Up.c_str());
    out << buffer;
}