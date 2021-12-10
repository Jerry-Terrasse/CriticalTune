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

typedef std::function<void(const std::vector<int>&)> Callback;

class Widget
{
public:
    std::string name;
    Widget(const std::string&);
    virtual std::string serial() = 0;
    //virtual int parse(const std::string&);
};
class NumberWidget: public Widget
{
    int upper, lower;
public:
    NumberWidget(const std::string&, int, int=0);
    std::string serial();
};
class CheckWidget: public Widget
{
public:
    CheckWidget(const std::string&);
    std::string serial();
};
class OptionWidget: public Widget
{
    std::vector<std::string> options;
public:
    OptionWidget(const std::string&, const std::vector<std::string>&);
    std::string serial();
};

typedef std::shared_ptr<Widget> pW;
typedef std::shared_ptr<NumberWidget> pNW;
typedef std::shared_ptr<CheckWidget> pCW;
typedef std::shared_ptr<OptionWidget> pOW;

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
public:
    Tuner(const std::string&, Callback, bool=true);
    void addWidgets(pW);
    template<typename ...Args>
    void addWidgets(pW, Args...);
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
static size_t WriteCallback(void*, size_t, size_t, void*);
//TEST
void cb(const std::vector<int> &dat)
{
    for(auto v: dat) {
        std::cout<<v<<' ';
    }
    std::cout<<std::endl;
    return;
}

int main()
{
    Tuner tuner("test", cb);
    tuner.addWidgets(CW("tttt0"), NW("ttt1", 255), OW("ttt2", "123", "fff"));
    tuner.start();
    std::this_thread::sleep_for(std::chrono::seconds(100000));
    return 0;
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}
template<typename ...Args>
pOW OW(const std::string &name, Args ...args)
{
    std::vector<std::string> options;
    (options.push_back(args), ...);
    pOW res(new OptionWidget(name, options));
    return res;
}
pCW CW(const std::string &name)
{
    pCW res(new CheckWidget(name));
    return res;
}
pNW NW(const std::string &name, int upper, int lower)
{
    pNW res(new NumberWidget(name, upper, lower));
    return res;
}
std::string Tuner::serial()
{
    std::string res;
    std::stringstream sio;
    sio << "[" << widgets.size();
    for(pW widget: widgets) {
        sio << "," << widget->serial();
    }
    sio << "]";
    sio >> res;
    return res;
}
std::vector<int> Tuner::parse(const std::string &res)
{
    int n=0, dat=0;
    std::vector<int> data;
    std::stringstream sio;
    sio << res;
    sio >> n >> n; //omitting the status "1"
    for(int i=0; i<n; ++i) {
        sio >> dat;
        data.push_back(dat);
    }
    return data;
}
void Tuner::loop()
{
    while(true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        const std::string &res = curlGET("/get/" + name);
        if(res[0] != '1') {
            std::cerr << "Updating Fail..." << std::endl;
            continue;
        }
        const std::vector<int> &dat = parse(res);
        if(dat.size() != widgets.size()) {
            std::cerr << "Data Length Error..." << std::endl;
            continue;
        }
        if(lazy && dat == data) {
            continue;
        }
        data = dat;
        callback(data);
    }
    return;
}
bool Tuner::start()
{
    if(started) {
        return true;
    }
    return started = start(1);
}
bool Tuner::start(int cnt)
{
    const std::string &res = curlPOST("/register/" + name, serial());
    if(res[0] != '1') {
        if(cnt > 3) {
            std::cerr << "Registration Fail." << std::endl;
            return false;
        }
        //std::cerr << std::format("Registration Fail, retrying... ({})", cnt) << std::endl;
        std::cerr << "Registration Fail, retrying... (" << cnt << ")" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(3));
        return start(cnt+1);
    }
    std::thread looper(&Tuner::loop, this);
    looper.detach();
    return true;
}
Tuner::Tuner(const std::string &name_, Callback callback_, bool lazy_):name(name_), callback(callback_), lazy(lazy_), started(false){}
template<typename ...Args>
void Tuner::addWidgets(pW widget, Args... args)
{
    widgets.push_back(widget);
    addWidgets(args...);
    return;
}
void Tuner::addWidgets(pW widget)
{
    widgets.push_back(widget);
    return;
}
std::string OptionWidget::serial()
{
    //std::format
    std::string res;
    std::stringstream sio;
    sio << "[3,\"" << name << "\"";
    for(const std::string &option: options) {
        sio << ",\"" << option << "\"";
    }
    sio << "]";
    sio >> res;
    return res;
}
std::string CheckWidget::serial()
{
    //std::format
    std::string res;
    std::stringstream sio;
    sio << "[2,\"" << name <<"\"]";
    sio >> res;
    return res;
}
std::string NumberWidget::serial()
{
    //return std::format("['{}',{},{}]", name, lower, upper);
    std::string res;
    std::stringstream sio;
    sio << "[1,\"" << name << "\"," << upper << ',' << lower << ']';
    sio >> res;
    return res;
}
OptionWidget::OptionWidget(const std::string &name_, const std::vector<std::string> &options_): Widget(name_)
{
    assert(validNames(options_));
    options = options_;
    return;
}
CheckWidget::CheckWidget(const std::string &name_): Widget(name_){}
NumberWidget::NumberWidget(const std::string &name_, int upper_, int lower_): Widget(name_), lower(lower_), upper(upper_)
{
    assert(lower_ <= upper_);
}
Widget::Widget(const std::string &name_)
{
    assert(validName(name_));
    name = name_;
}
bool validNames(const std::vector<std::string> &names)
{
    for(const std::string &name: names) {
        if(!validName(name)) {
            return false;
        }
    }
    return true;
}
bool validName(const std::string &name)
{
    for(char c: name) {
        if(!isalnum(c)) {
            std::cerr << "Invalid Name: '" << name << "'" <<  std::endl;
            return false;
        }
    }
    return true;
}
std::string curlPOST(const std::string &rurl, const std::string &data) // rurl: url relative to HOST
{
    CURL *curl = curl_easy_init();
    std::string url = HOST + rurl;
    std::string readBuffer;
    CURLcode res;
    //std::cout << std::format("[POST] send to {}: {}", rurl, data) << std::endl;
    std::cout << "[POST] send to " << url << ": " << data << std::endl;
    assert(curl != NULL);
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        if(res == CURLE_OK) {
            std::cout << "[POST] receive: " << readBuffer << std::endl;
            return readBuffer;
        }
    }
    std::cerr << "[POST] CURL Error" << std::endl;
    return "fail";
}
std::string curlGET(const std::string &rurl) // rurl: url relative to HOST
{
    CURL *curl = curl_easy_init();
    std::string url = HOST + rurl;
    std::string readBuffer;
    CURLcode res;
    std::cout << "[GET] send: " << url << std::endl;
    assert(curl != NULL);
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        if(res == CURLE_OK) {
            std::cout << "[GET] receive:" << readBuffer << std::endl;
            return readBuffer;
        }
    }
    std::cerr << "[GET] CURL Error" << std::endl;
    return "fail";
}