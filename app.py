import time
from flask import Flask, request, render_template, json
from flask.wrappers import Response
from typing import Generator, Union, cast
import cv2
from collections import deque
import hashlib

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

class VideoWidget(Widget):
    def __init__(self, name: str) -> None:
        super().__init__(name)
        self.type = 4
        self.val = vmg.register(name)
    def setVal(self, val: int) -> None:
        return # omit operation

class VideoManager:
    def __init__(self) -> None:
        self.fps = 60
        self.interval = 1 / self.fps
        self.default_img: bytes = cv2.imencode(".jpeg", cv2.imread("temp/34.jpg"))[1].tobytes()
        self.queues: dict[int, deque] = {}
        self.times: dict[int, float] = {}
    @staticmethod
    def token(name: str) -> int:
        #return hashlib.md5(name.encode('utf-8')).hexdigest()
        return hash(name) % 2**31 # int32 # TODO: hash collision?
    def register(self, name: str) -> int:
        token = VideoManager.token(name)
        if token not in self.queues:
            q = deque()
            q.append(self.default_img)
            self.queues[token] = q
            self.times[token] = float()
            print(f"Video Registration: {name} to {token}")
        return token
    def release(self, token: int) -> None:
        del self.queues[token]
        del self.times[token]
    def push(self, token: int, data: bytes) -> None:
        self.queues[token].append(data)
        return
    def pop(self, token: int) -> bytes:
        q = self.queues[token]
        self.times[token] = time.time()
        data = q[0] if len(q) == 1 else q.popleft()
        tm = time.time()
        if tm - self.times[token] < self.interval:
            time.sleep(self.interval + self.times[token] - tm)
        self.times[token] = tm
        return data

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
    elif obj[0] == 4:
        assert isinstance(obj[1], str)
        return VideoWidget(obj[1])
    else:
        raise ValueError("UnknownWidgetType")


vmg = VideoManager()

table: dict[str, list[Widget]] = {
    'test': [NumberWidget('t1', 255), CheckWidget('t2'), OptionWidget('t3', ['a', 'b'])],
    'another': [],
    'vtest': [VideoWidget("v1")]
}
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
        data = request.data
        data = data.decode()
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
        data = request.data
        data = data.decode()
        data = json.loads(data)
        print(data)
        n = int(data[0])
        widgets = [parse(obj) for obj in data[1:]]
        assert n == len(widgets)
        table[name] = widgets
        return get(name) # success
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

@app.route('/stream/<token>')
def stream(token: str) -> Response:
    try:
        return Response(get_frame(int(token)), mimetype='multipart/x-mixed-replace; boundary=frame')
        #return Response(data, mimetype="image/jpeg")
    except Exception as e:
        print(e)
        return Response(None, 404)

@app.route('/publish/<token>', methods=['POST'])
def publish(token: str) -> str:
    '''
    c++ user push image to server
    '''
    try:
        data = request.data
        vmg.push(int(token), data)
        return '1'
    except Exception as e:
        print(e)
        return '0'

def get_frame(token: int) -> Generator:
    while True:
        yield b'--frame\r\nContent-Type: image/jpeg\r\n\r\n%b\r\n' % (vmg.pop(token))

def add(name: str, widgets: list[Widget]) -> None:
    table[name] = widgets
    return

def run():
    app.run(debug=True)

if __name__ == '__main__':
    run()