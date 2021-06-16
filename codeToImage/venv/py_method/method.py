import numpy as np
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
    