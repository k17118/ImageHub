import datetime
from flask import Flask, render_template, request, redirect, session, url_for
from flask_sqlalchemy import SQLAlchemy

import base64
from io import BytesIO
from PIL import Image

from py_method import method

# instance
app = Flask(__name__)

app.config['SECRET_KEY'] = 'secret_key'


code_data = None
color_data = None
base64_input = None

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
    code = db.Column(db.String(720000), nullable=False)
    color = db.Column(db.String(1440000), nullable=False)
    title = db.Column(db.String(128), nullable=False)
    path = db.Column(db.String(128), nullable=False)
    created_at = db.Column(db.String(100), default=datetime.datetime.now)
    updated_at = db.Column(db.String(100), default=datetime.datetime.now, onupdate=datetime.datetime.now)
    
@app.route("/")#url
def top():
    return render_template("top.html")

@app.route("/regist")
def regist():
    return render_template("index.html")

@app.route("/confirm")
def confirm():
    if 'token' in session:
        width = session['width']
        height = session['height']
        
        #文字列をリストに分割
        code_list  = method.splitCodeText(code_data)
        color_list = method.splitColorText(color_data)
        random_list = method.productRandomText(width, height)
        
        #セッションを破棄
        # session.pop('token')
        
        return render_template("confirm.html", width=width, height=height, \
            code_text=code_list, pixel_data=color_list, random_text=random_list)
    else:
        return redirect(url_for('regist'))

@app.route("/registed")
def registed():
    if 'token' in session:
        #セッションを破棄
        session.pop('token')
        
        return render_template("registed.html")
    else:
        return redirect(url_for('regist'))
    
@app.route("/goRegist", methods=["POST"])
def go_to_regist_view():
    return redirect(url_for('regist'))
    
@app.route("/Confirm", methods=["POST"])
def regist_confirmation():
    code_width  = request.form['image-w'] #タイル幅
    code_height = request.form['image-h'] #タイル高さ
    code_text   = request.form['image-code'] #プログラム
    code_color  = request.form['image-color'] #色情報
    input_image = request.form['input'] #入力画像
    
    #保存用にグローバルデータとして保存
    global code_data; global color_data; global base64_input;
    code_data = code_text #保存
    color_data = code_color #保存
    base64_input = input_image
    
    #セッションで配置
    session['width'] = code_width
    session['height'] = code_height
    session['token'] = 'token'
    
    return redirect(url_for('confirm'))

@app.route("/Regist", methods=["POST"])
def regist_data():
    
    title = request.form['name']
    
    #画像にエンコード(tile画像)
    enc_data = request.form['picture']
    dec_data = base64.b64decode(enc_data.split(',')[1])
    dec_img = Image.open(BytesIO(dec_data))
    
    # tileの保存
    image_name = method.getImageName()#保存用の名前を生成
    path_name = 'static/tile_img/' + title + '_' + image_name + '.png'
    dec_img = method.cutSpace(dec_img)#余白の削除
    dec_img.save(path_name)#画像の書き出し
    
    #画像にエンコード(入力画像)
    dec_input = base64.b64decode(base64_input.split(',')[1])
    input_img = Image.open(BytesIO(dec_input))
    
    # 入力画像の保存
    path_name = 'static/img/' + title + '_' + image_name + '.png'
    input_img.save(path_name)#画像の書き出し
    
    regist_db = ProductDB()#クラスをインスタンス化
    regist_db.user_name = 'guest'
    regist_db.code = code_data
    regist_db.color = color_data
    regist_db.title = title
    regist_db.path = path_name
    
    session['token'] = 'token'
    
    db.session.add(regist_db)#regist_dbをセッションに追加(この時点ではデータベースに追加されていない)
    db.session.commit()#データベースに登録
    return redirect(url_for('registed'))

if __name__ == '__main__':
    db.create_all()#データベースの作成
    app.run(debug=True,  host='0.0.0.0', port=1017) # ポートの変更