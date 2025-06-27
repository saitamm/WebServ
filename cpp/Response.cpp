#include "../hpp/Response.hpp"
#include <sys/stat.h>


Response::Response(){}
Response::~Response(){}
int allowMethod(Location loc, string method)
{
    vector<string> vec = loc.getMethods();
    cout << "***********\n";
    for(size_t idx = 0;idx < vec.size();idx++)
    {
        if (vec[idx] == method)
            return (1);
    }
    return (0);
}

void handleDelete(Response &resp)
{
    string     file = resp.getRequest().getUri();
    cout << "file is  = "<< file <<endl;
    struct stat Buff;
    cout << stat(file.c_str(), &Buff) << endl;
    if (access(file.c_str(), F_OK))
        resp.setStatus("404");
}

void MakeResponce(Request &req, Response &resp)
{
    resp.setRequest(req);
    if (!allowMethod(*req.getLocation(), req.getMethod()))
    {
        cout << "=="<<endl;
        resp.setStatus("405");
    }
    if (req.getMethod() == "DELETE")
        handleDelete(resp);
}


string &Response::getStatus(void){return (_status);}
size_t &Response::getContentLength(void){return(_ContentLength);}
string &Response::getContenttype(void){return (_ContentType);}
Request &Response::getRequest(void){return (_req);}

//setters
void Response::setContentLength(size_t size){_ContentLength = size;}
void Response::setContentType(string content){_ContentType = content;}
void Response::setRequest(Request &req){_req = req;}
void Response::setStatus(string stat){_status = stat;}


