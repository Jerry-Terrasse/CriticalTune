import time
from flask import Flask, request, render_template, json
from flask.wrappers import Response
from typing import Union, cast

class Widget:
    def __init__(self, name: str) -> None:
        self.name = name
        self.type = 0
        self.val = 0
    def setVal(self, val: int) -> None:
        self.val = val

class NumberWidget(Widget):
    def __init__(self, name: str, upper: int, lower: int = 0) -> None:
        super().__init__(name)
        self.type = 1
        self.lower = lower
        self.upper = upper

class CheckWidget(Widget):
    def __init__(self, name: str) -> None:
        super().__init__(name)
        self.type = 2

class OptionWidget(Widget):
    def __init__(self, name: str, options: list[str]) -> None:
        super().__init__(name)
        self.type = 3
        self.options = options

def parse(obj: list[Union[str, int]]) -> Widget:
    if obj[0] == 1:
        assert isinstance(obj[1], str)
        assert isinstance(obj[2], int)
        assert isinstance(obj[3], int)
        return NumberWidget(obj[1], obj[2], obj[3])
    elif obj[0] == 2:
        assert isinstance(obj[1], str)
        return CheckWidget(obj[1])
    elif obj[0] == 3:
        assert all(isinstance(option, str) for option in obj[1:])
        options = cast(list[str], obj[1:])
        return OptionWidget(options[0], options[1:])
    raise ValueError("UnknownWidgetType")

table: dict[str, list[Widget]] = {'test': [NumberWidget('t1', 255), CheckWidget('t2'), OptionWidget('t3', ['a', 'b'])], 'another': []}
app = Flask("Critical Tune")


@app.route('/')
@app.route('/update')
def hello_world() -> str:
    return render_template('home.jinja', table=table.keys())

@app.route('/update/<name>', methods=['GET'])
def update(name: str) -> str:
    if name in table:
        return render_template('update.jinja', name=name, widgets=table[name], table=table.keys())
    else:
        return render_template('error.jinja', info='Unknown Name')
    #return render_template('error.jinja', info='Invalid Method')

@app.route('/submit/<name>', methods=['POST'])
def submit(name: str) -> Response:
    try:
        data, = request.form.to_dict().keys() # this dict only have one key
        data = json.loads(data)
        assert len(data) == len(table[name])
        [widget.setVal(val) for val, widget in zip(data, table[name])]
        return Response(json.dumps({'success': True, 'time': time.asctime()}), 200, mimetype='application/json')
    except Exception as e:
        print(e)
        return Response(json.dumps({'success': False, 'errorinfo': str(e)}), 200, mimetype='application/json')

@app.route('/register/<name>', methods=['POST'])
def register(name: str) -> str:
    try:
        data, = request.form.to_dict().keys() # this dict only have one key
        data = json.loads(data)
        print(data)
        n = int(data[0])
        widgets = [parse(obj) for obj in data[1:]]
        assert n == len(widgets)
        table[name] = widgets
        return '1' # success
    except Exception as e:
        print(e)
        return '0'

@app.route('/get/<name>', methods=['GET'])
def get(name: str) -> str:
    try:
        widgets = table[name]
        res = ['1', str(len(widgets))]
        [res.append(str(widget.val)) for widget in widgets]
        res = ' '.join(res)
        return res # success
    except Exception as e:
        print(e)
        return '0'

def add(name: str, widgets: list[Widget]) -> None:
    table[name] = widgets
    return

def run():
    app.run(debug=True)

run()