import cv2
import datetime
import numpy as np
from PIL import Image
import random, string
from re import split

def splitColorText(color_str):
    color_list = color_str.split(',')#カンマで分割
    rgb_list = []
    
    #rgb値の登録
    for i in range(0, len(color_list), 3):
        rgb_list.append([color_list[i], color_list[i+1], color_list[i+2]])
    
    return rgb_list

def splitCodeText(code_str):
    str_list = []
    for i in range(0, len(code_str), 1):
        if i%2 == 0:
            str_list.append(code_str[i])
    return str_list

def productRandomText(width, height):
    length = int(width)*int(height)
    # 画素数分のrandomリストを作成
    randlst = [random.choice(string.ascii_letters + string.digits) for i in range(length)]
    return randlst

def getImageName():
    time_str = []
    dt_now = datetime.datetime.now()#現在の時間を取得
    time_data = str(dt_now)#文字列に変換
    
    day_str = time_data[:10]#日付
    time_str = time_data[11:19]#時間
    image_name = day_str + time_str
    
    return (image_name)

def cutSpace(pil_img):
    
    # PIL -> OpenCV
    new_image = np.array(pil_img, dtype=np.uint8)
    new_image = cv2.cvtColor(new_image, cv2.COLOR_RGB2BGR)
    
    #切り抜き処理
    height, width = new_image.shape[:2]
    
    new_gray = cv2.cvtColor(new_image, cv2.COLOR_BGR2GRAY)
    new_bin = cv2.threshold(new_gray, 253, 255, cv2.THRESH_BINARY)[1]
    
    left = None; right = None;
    for i in range(width-1):
        roi_image = new_bin[0:height, i:i+1]#幅1で切り出し
        if left == None and cv2.countNonZero(roi_image) != height:
            left = i-1
            continue
        
        if left != None and cv2.countNonZero(roi_image) == height:
            right = i
            break
    
    # エラー処理
    if left == None:
        left = 0
    if right == None:
        right = width
    
    
    top = None; bottom = None;
    for i in range(height-1):
        roi_image = new_bin[i:i+1, 0:width]#幅1で切り出し
        if top == None and cv2.countNonZero(roi_image) != width:
            top = i-1
            continue
        
        if top != None and cv2.countNonZero(roi_image) == width:
            bottom = i
            break
    
    crop_image = new_image[top:bottom, left:right]
    
    #opencv -> PIL
    dst_image = crop_image.copy()
    dst_image = cv2.cvtColor(dst_image, cv2.COLOR_BGR2RGB)
    dst_image = Image.fromarray(dst_image)
    
    return(dst_image)