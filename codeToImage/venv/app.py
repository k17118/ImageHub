from flask import Flask, render_template

# instance
app = Flask(__name__)

@app.route("/")#url
def hello_world():
    return render_template("index.html")

if __name__ == '__main__':
    app.run(debug=True,  host='0.0.0.0', port=1017) # ポートの変更