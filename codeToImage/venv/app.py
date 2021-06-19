import datetime
from flask import Flask, render_template, request, redirect, session, url_for, make_response
from flask_sqlalchemy import SQLAlchemy

import base64
from io import BytesIO
from PIL import Image

from py_method import method

import pyperclip

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
    thumbnail = db.Column(db.String(128), nullable=False)
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
    
@app.route("/view")
def show():
        
    # 表示するページ番号の指定
    page_number = 0
    if 'page_num' in session:
        page_number = session['page_num']
        session.pop('page_num')
    else:
        page_number = 1
    page_number = int(page_number)
    
    print(page_number)
    
    data = ProductDB.query.all()
    data_size = len(data)
    
    image_path = []# 画像のパスリスト
    title_path = []# タイトルのリスト
    time_list = []# 登録時刻
    author_list = []# 著者のリスト
    
    for x in range(data_size-1, -1, -1):
        image_path.append(data[x].thumbnail)
        title_path.append(data[x].title)
        time_list.append((data[x].created_at)[0:10])
        author_list.append(data[x].user_name)
    
    return render_template("viewer.html", image_path=image_path, title_path=title_path, \
                            time_list=time_list, author_list=author_list, page_number=page_number)

# viewerページ遷移用関数---------------------------
@app.route("/page1", methods=["POST"])
def page1():
    session['id'] = request.form['page1']
    print(session['id'])
    return redirect(url_for('show_img'))

@app.route("/page2", methods=["POST"])
def page2():
    session['id'] = request.form['page2']
    print(session['id'])
    return redirect(url_for('show_img'))

@app.route("/page3", methods=["POST"])
def page3():
    session['id'] = request.form['page3']
    print(session['id'])
    return redirect(url_for('show_img'))

@app.route("/page4", methods=["POST"])
def page4():
    session['id'] = request.form['page4']
    print(session['id'])
    return redirect(url_for('show_img'))

@app.route("/page5", methods=["POST"])
def page5():
    session['id'] = request.form['page5']
    print(session['id'])
    return redirect(url_for('show_img'))

@app.route("/page6", methods=["POST"])
def page6():
    session['id'] = request.form['page6']
    print(session['id'])
    return redirect(url_for('show_img'))

@app.route("/page7", methods=["POST"])
def page7():
    session['id'] = request.form['page7']
    print(session['id'])
    return redirect(url_for('show_img'))

@app.route("/page8", methods=["POST"])
def page8():
    session['id'] = request.form['page8']
    print(session['id'])
    return redirect(url_for('show_img'))

@app.route("/page9", methods=["POST"])
def page9():
    session['id'] = request.form['page9']
    print(session['id'])
    return redirect(url_for('show_img'))

@app.route("/page10", methods=["POST"])
def page10():
    session['id'] = request.form['page10']
    print(session['id'])
    return redirect(url_for('show_img'))

@app.route("/page11", methods=["POST"])
def page11():
    session['id'] = request.form['page11']
    print(session['id'])
    return redirect(url_for('show_img'))

@app.route("/page12", methods=["POST"])
def page12():
    session['id'] = request.form['page12']
    print(session['id'])
    return redirect(url_for('show_img'))

@app.route("/page13", methods=["POST"])
def page13():
    session['id'] = request.form['page13']
    print(session['id'])
    return redirect(url_for('show_img'))

@app.route("/page14", methods=["POST"])
def page14():
    session['id'] = request.form['page14']
    print(session['id'])
    return redirect(url_for('show_img'))

@app.route("/page15", methods=["POST"])
def page15():
    session['id'] = request.form['page15']
    print(session['id'])
    return redirect(url_for('show_img'))
# -----------------------------------------------


@app.route("/pageTransitionPrev", methods=["POST"])
def transitionPrev():
    session['page_num'] = request.form['page-num1']
    return redirect(url_for('show'))

@app.route("/pageTransitionNext", methods=["POST"])
def transitionNext():
    session['page_num'] = request.form['page-num2']
    return redirect(url_for('show'))
    
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
    thumbnail_name = 'static/img/' + title + '_' + image_name + '.png'
    input_img = method.productThumbnail(input_img)# サムネイル用に切り抜き
    input_img.save(thumbnail_name)#画像の書き出し
    
    regist_db = ProductDB()#クラスをインスタンス化
    regist_db.user_name = 'guest'
    regist_db.code = code_data
    regist_db.color = color_data
    regist_db.title = title
    regist_db.path = path_name
    regist_db.thumbnail = thumbnail_name
    
    session['token'] = 'token'
    
    db.session.add(regist_db)#regist_dbをセッションに追加(この時点ではデータベースに追加されていない)
    db.session.commit()#データベースに登録
    return redirect(url_for('registed'))
    
@app.route("/show-img", methods=["GET", "POST"])
def show_img():
    usr_id = 1
    # id
    if 'id' in session:
        session['id'] = session['id']
        print('id = ', session['id'])
    else:
        print('None')
    usr_id = session['id']
    # session['id'] = usr_id
    # moe_pics = ProductDB.query.all()
    moe_pics = ProductDB.query.filter_by(id = int(usr_id)).all()
    # moe_pics = ProductDB.query.get(usr_id)
    width = 200

    moe_pics[0].color = method.splitColorText(moe_pics[0].color)
    moe_pics[0].code = method.splitCodeText(moe_pics[0].code)
    height = (len(moe_pics[0].color) / width)
    random_list = method.productRandomText(width, height)
    print(moe_pics[0].title)

    return render_template("show-img.html", width=width, height=height, img_title=moe_pics[0].title, \
            code_text=moe_pics[0].code, pixel_data=moe_pics[0].color, random_text=random_list)

@app.route("/img_download", methods=["GET", "POST"])
def download_img():
    # usr_id = 1
    usr_id = session["id"]
    XLSX_MIMETYPE = 'application/vnd.openxmlformats-officedocument.spreadsheetml.sheet'
    moe_pic = ProductDB.query.filter_by(id = int(usr_id)).all()
    img_path = moe_pic[0].path
    img_title = moe_pic[0].title
    response = make_response()
    
    # print(img_path)
    response.data = open(img_path, "rb").read()

    downloadFileName = img_title + '.png'
    response.headers['Content-Disposition'] = 'attachment; filename=' + downloadFileName


    response.mimetype = XLSX_MIMETYPE
    return response

@app.route("/text_download", methods=["GET", "POST"])
def download_text():
    # usr_id = 1
    usr_id = session["id"]
    moe_pic = ProductDB.query.filter_by(id = int(usr_id)).all()
    img_code = moe_pic[0].code
    img_code = method.splitCodeText(img_code)
    raw_code = ''
    for text in img_code:
        if text == '×':
            text = ' '
        if text == '△':
            text = '\n'
        raw_code = raw_code + text


    # print(str(raw_code))
    pyperclip.copy(raw_code)
    # download_test()
    return ('', 204)


if __name__ == '__main__':
    db.create_all()#データベースの作成
    app.run(debug=True,  host='0.0.0.0', port=1017) # ポートの変更
