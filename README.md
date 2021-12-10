# Critical Tune

## Example

终端：

```shell
pip install -r requirements.txt --user
python app.py
```

新建终端：

```shell
mkdir build && cd build
cmake ..
make
cd ..
./example
```

## Usage

先启动`app.py`服务端服务，然后在C++代码中包含`tuner.h`，即可使用调参器接口。

用户需自行定义回调函数，原型参照`void callback(const std::vector<int>&) `，调参器会自动调用回调函数并传回每个组件的返回值。

### 组件 Widget

每个待调参数抽象为一个`Widget`（虚类），其派生为`NumberWidget` `CheckWidget` `OptionWidget`三类，分别对应离散数值型参数、布尔型参数、选项型参数。

上述三类组件分别会在网页端产生滑条、开关、下拉菜单。

组件返回值均为`int`类型，其中`NumberWidget`的返回值属于$[lower, upper] \cap \mathbb{Z}$，其中`lower`和`upper`在构造时定义；`CheckWidget`的返回值为`1`对应`true`、`0`对应`false`；`OptionsWidget`返回值为所选项的序号，下标从`0`开始计。

### 接口

```cpp
class Tuner(const std::string &name, Callback callback, bool lazy=true); //构造Tuner
/* 其中Callback是一个接收std::vector<int>作为参数、返回值为void的函数
   如果lazy为true，则只在参数变更时调用callback，否则每隔1s调用一次
*/
void Tuner::addWidgets(Args... Widgets);//添加Widgets，参数为Widget*，支持任意数目参数
void Tuner::start();//启动Tuner，每隔1s检测参数是否更改

//便捷地构造三种组件，直接返回指针便于传递给Tuner的构造函数
pW NW(const std::string &name, int upper, int lower=0);
pW CW(const std::string &name);
pW OW(const std::string &name, Args... options);//options类型为std::string，支持任意数目参数
```

虽然事实上有更多的接口暴露，但是上述几个已经足够。

### Tuner 调参器类

咕咕咕
