#ifndef CRITICAL_TUNER
#define CRITICAL_TUNER 1

#include<iostream>
#include<sstream>
#include<thread>
#include<memory>
#include<cstring>
#include<vector>
#include<functional>
#include<curl/curl.h>
#include<cstdlib>
#include<assert.h>

#ifndef HOST
    #define HOST ("127.0.0.1:5000")
#endif


class Widget
{
    Widget();
public:
    std::string name;
    Widget(const std::string&);
    virtual std::string serial() = 0;
    //virtual int parse(const std::string&);
};
class NumberWidget: public Widget
{
    int lower, upper;
    NumberWidget();
public:
    NumberWidget(const std::string&, int, int=0);
    std::string serial();
};
class CheckWidget: public Widget
{
    CheckWidget();
public:
    CheckWidget(const std::string&);
    std::string serial();
};
class OptionWidget: public Widget
{
    std::vector<std::string> options;
    OptionWidget();
public:
    OptionWidget(const std::string&, const std::vector<std::string>&);
    std::string serial();
};

typedef std::shared_ptr<Widget> pW;
typedef std::shared_ptr<NumberWidget> pNW;
typedef std::shared_ptr<CheckWidget> pCW;
typedef std::shared_ptr<OptionWidget> pOW;


typedef std::function<void(const std::vector<int>&)> Callback;

class Tuner
{
    std::string name;
    Callback callback;
    std::vector<pW> widgets;
    std::vector<int> data;
    bool started, lazy;
    bool start(int);
    void loop();
    std::vector<int> parse(const std::string&);
    Tuner();
public:
    Tuner(const std::string&, Callback, bool=true);
    template<typename ...Args>
    void addWidgets(pW, Args...);
    void addWidgets(pW);
    bool start();
    std::string serial();
};

bool validName(const std::string&);
bool validNames(const std::vector<std::string>&);

std::string curlGET(const std::string&);
std::string curlPOST(const std::string&, const std::string&);

pNW NW(const std::string&, int, int=0);
pCW CW(const std::string&);
template<typename ...Args>
pOW OW(const std::string&, Args...);

#endif /* CRITICAL_TUNER */