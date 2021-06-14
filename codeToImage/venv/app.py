import datetime
from flask import Flask, render_template, request, redirect, url_for
from flask_sqlalchemy import SQLAlchemy

from py_method import method

# instance
app = Flask(__name__)
#データベースの種類とファイル名を指定
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///work.db'
#ここは基本無効
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
#Trueにすると実行される
app.config['SQLALCHEMY_ECHO'] = True
#Flaskのインスタンスを引数に渡すことでFlaskでSQLAlchemyを利用してデータベースを扱える
db = SQLAlchemy(app)

#DBの形式を定義
class ProductDB(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    user_name = db.Column(db.String(128), nullable=False)
    code = db.Column(db.String(150000), nullable=False)
    color = db.Column(db.String(600000), nullable=False)
    title = db.Column(db.String(128), nullable=False)
    created_at = db.Column(db.String(100), default=datetime.datetime.now)
    updated_at = db.Column(db.String(100), default=datetime.datetime.now, onupdate=datetime.datetime.now)
    
@app.route("/")#url
def regist():
    return render_template("index.html")

@app.route("/Confirm", methods=["POST"])
def regist_confirmation():
    code_width = request.form['image-w'] 
    code_height = request.form['image-h'] 
    code_text = request.form['image-code'] 
    code_color = request.form['image-color'] 
    
    code_text = method.splitCodeText(code_text)
    code_color = method.splitColorText(code_color)
    
    print(code_text)
    return render_template("confirm.html", width=code_width, height=code_height, \
        code_text=code_text, pixel_data=code_color)

@app.route("/top")
def top():
    return render_template("top.html")

if __name__ == '__main__':
    db.create_all()#データベースの作成
    app.run(debug=True,  host='0.0.0.0', port=1017) # ポートの変更