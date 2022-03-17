from crypt import methods
from flask import Flask, render_template, request, redirect
import paho.mqtt.client as mqtt

app = Flask(__name__)
client = mqtt.Client("p1")
broker_address = "192.168.0.58"

client.connect(broker_address)

client = mqtt.Client("P1")
broker = 'localhost'
port = 1883

client.connect(broker, port, 60)



@app.route("/", methods=["POST", "GET"])
def index():
    return render_template("index.html")


@app.route('/call1', methods=["POST", "GET"])
def call1():
    client.publish("inDoor", "1")
    client.publish("outDoor", "1")
    return "open door"


@app.route("/call2", methods=["POST", "GET"])
def call2():
    client.publish("inDoor", "0")
    client.publish("outDoor", "o")

    return "close door"


if __name__ == "__main__":
    app.run(debug=True, host="0.0.0.0")




