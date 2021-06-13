//
//  insupector.cpp
//  washstandMainCheckerForSIM
//
//  Created by 加藤祥真 on 2020/09/14.
//  Copyright © 2020 加藤祥真. All rights reserved.
//

#include "mainChecker.hpp"

#define BG_IMG_FILE_NAME "../Debug/main/back.jpg"

#define SMALL_RATIO (0.1)
//コンストラクタ
MainChecker::MainChecker() {
    struct passwd *pw = getpwuid(getuid());
    home_dir = pw->pw_dir;//ホームディレクトリまでのパスを取得
    
    this->bg_img = cv::imread(home_dir + "/config/back.jpg", 0);//グレースケールで入力
    
    if(this->bg_img.empty()){
        std::cerr << "back.jpg not found." << std::endl;
        
    }
    
    back_image = this->bg_img.clone();//保存用
    
    cv::resize(this->bg_img, this->resized_bg_img, cv::Size(), SMALL_RATIO, SMALL_RATIO); //1/10にする
    
    this->bin_threshold = 50; //二値化閾値
    //this->detection_area_threshold = 34000; //検出用の面積閾値
    this->detection_area_threshold = 25000; //検出用の面積閾値
    extraction_area_threshold  =  9800000;//目視で数値確認した上で決定
    
    this->centering_y_threshold = 50; //中央に配置されているかの閾値 (y座標のみ) (半径)
    this->is_judged = false; //流れている洗面台が判定済みかどうか
    this->area_center = cv::Point2f(-1, -1); //領域の中心座標
    
    //Debug用出力ファイルの生成
    std::string filename = home_dir + "/config/debug_message.txt";
    //this->debug_writing_file.open(filename, std::ios::out);
}

//画像の設定
void MainChecker::setSourceImage (const cv::Mat source_img) {
    //std::cout << "画像をセット" << std::endl;
    this->src_img = source_img.clone();
}

//洗面台の検出
bool MainChecker::isLavatoryDetected() {
    
    if(this->src_img.empty()){
        return false;
    }
    
    cv::Mat org_img = this->src_img.clone();
    //入力画像の縮小
    cv::resize(this->src_img, this->resized_src_img, cv::Size(), SMALL_RATIO, SMALL_RATIO);
    //グレースケール化
    cv::cvtColor(this->resized_src_img, this->resized_gray_src_img, cv::COLOR_BGR2GRAY);
    //差分
    cv::absdiff(this->resized_gray_src_img, this->resized_bg_img, this->resized_diff_img);
    
    //二値化
    cv::threshold(this->resized_diff_img, this->resized_bin_img, this->bin_threshold, 1, cv::THRESH_BINARY);
    
    //背景差分エラー監視用
    cv::imshow("reized", this->resized_src_img);
    cv::imshow("diff", this->resized_diff_img);
    
    double val_sum = sum(resized_bin_img)[0];
    
    
    if (val_sum > detection_area_threshold) {
        return (true); //検出
    }
    
    return false;
}


//洗面台領域の抽出
cv::Mat MainChecker::extractLavatory() {
    cv::RotatedRect rotate_param = getRotateParam(this->focus_contour);
    cv::Point2f roi_points[4];
    rotate_param.points(roi_points);
    
    //回転後、切り抜いた画像を生成
    this->lavatory_img = getRotateTrimImage(rotate_param);
    
    this->lavatory_img = checkSorrounding(this->lavatory_img);
    
    //本当に洗面台かどうか判定
    bool lavatory_flag = judgeLavatory(this->lavatory_img);
    if(!lavatory_flag){
        cv::Mat clear_img;
        this->lavatory_img = clear_img.clone();//空画像のクローン
    }
    
    return (this->lavatory_img);
}

cv::RotatedRect MainChecker::getRotateParam(std::vector<cv::Point> contour){
    cv::Point2f vtx[4];
    cv::RotatedRect box = cv::minAreaRect(contour);//回転角と隣接矩形の角4点
    return (box);
}

cv::Mat MainChecker::getRotateTrimImage(const cv::RotatedRect rotate_param){
    cv::RotatedRect rotate_rect = rotate_param; //コピー
    //回転の中心を指定(画像の中心)
    cv::Point2f center = cv::Point2f(static_cast<float>(this->src_img.cols/2), static_cast<float>(this->src_img.rows/2));
    double angle_deg = rotate_rect.angle;//回転角度
    //参考URL: https://qiita.com/vs4sh/items/93d65468a992af5b8f92
    //fprintf(stderr, "angle=%f\n", angle_deg); //回転角度の印字
    
    /*
     if (angle_deg < 0) {
     angle_deg = 180 + angle_deg; //反対に回転
     }
     */
    
    double w, h;
    if (fabs(angle_deg) < 8.0) {
        //angle_deg = 90.0;
        std::cerr << "-------" << std::endl;
        angle_deg = 90 + angle_deg; //反対に回転
        w = static_cast<double>(this->src_img.rows);
        h = static_cast<double>(this->src_img.cols);
    }else{
        if (angle_deg < 0) {
            angle_deg = 180 + angle_deg; //反対に回転
        }
        w = static_cast<double>(this->src_img.cols);
        h = static_cast<double>(this->src_img.rows);
    }
    
    //洗面台領域のROIを取得
    cv::Point2f roi_points[4];
    rotate_rect.points(roi_points);
    
    double scale = 1.0;
    double angle_rad = angle_deg/180.0*M_PI;//角度をラジアンに変換
    
    int w_rot, h_rot;
    //回転後の画像サイズを計算
    if(fabs(angle_deg)<8.0){
        w_rot = int(round(h*abs(sin(angle_rad))+w*abs(cos(angle_rad))));
        h_rot = int(round(h*abs(cos(angle_rad))+w*abs(sin(angle_rad))));
    }else{
        w_rot = int(round(w*abs(sin(angle_rad))+h*abs(cos(angle_rad))));
        h_rot = int(round(w*abs(cos(angle_rad))+h*abs(sin(angle_rad))));
    }
    cv::Size rot_size = {w_rot, h_rot};//回転後のサイズ
    
    cv::Mat rotation = cv::getRotationMatrix2D(center, angle_deg, scale);//回転行列を求める
    //内挿方法
    int interpolationFlag = cv::INTER_CUBIC;
    
    cv::Mat affine = rotation.clone();
    cv::Size rotate_size;
    //アフィン行列を生成(平行移動を追加)
    if (fabs(angle_deg)<8.0) {
        affine.at<double>(0, 2) = affine.at<double>(0, 2) - this->src_img.rows/2 + w_rot/2;
        affine.at<double>(1, 2) = affine.at<double>(1, 2) - this->src_img.cols/2 + h_rot/2;
    }else{
        affine.at<double>(0, 2) = affine.at<double>(0, 2) - this->src_img.cols/2 + w_rot/2;
        affine.at<double>(1, 2) = affine.at<double>(1, 2) - this->src_img.rows/2 + h_rot/2;
    }
    
    //画像の回転を行う
    cv::Mat rotate_img, test_img;
    
    cv::warpAffine(this->src_img, rotate_img, affine, rot_size, interpolationFlag);
    this->image_size = rotate_img.size();//回転後のサイズを取得
    cv::Rect roi_rect = getRoiRect(roi_points, affine);//回転された切り抜き座標を得る
    
    cv::Mat roi_img;
    //std::cout << roi_rect << ", " << rotate_img.cols << ", " << rotate_img.rows << std::endl;
    roi_rect = dealWithErrorFromRoi(roi_rect, rotate_img.size());
    roi_img = rotate_img(roi_rect);
    //エラー対応
    //    if (roi_rect.x > 0 && roi_rect.y > 0 && roi_rect.width>0 && roi_rect.height>0 && roi_rect.x+roi_rect.width <= rotate_img.cols && roi_rect.y+roi_rect.height <= rotate_img.rows) {
    //        roi_img = rotate_img(roi_rect);
    //    }
    //this->debug_writing_file << "--getRotateTrimImage" << std::endl;
    return (roi_img);
}

cv::Rect MainChecker::getRoiRect(cv::Point2f *points, cv::Mat affine){
    
    cv::Point2f roi_point[4];//回転後の関心領域矩形の角4点
    //ROIの角を回転
    for(int i=0; i<4; i++){
        roi_point[i].x = affine.at<double>(0, 0)*points[i].x + affine.at<double>(0, 1)*points[i].y + affine.at<double>(0, 2);
        roi_point[i].y = affine.at<double>(1, 0)*points[i].x + affine.at<double>(1, 1)*points[i].y + affine.at<double>(1, 2);
    }
    
    double min_dis = this->image_size.width * image_size.height;
    double max_dis = this->image_size.width * image_size.height;
    cv::Point above_left, below_right;
    //左上の点、右下の点
    cv::Point min_point = {0, 0}, max_point = {this->image_size.width, this->image_size.height};
    
    for(cv::Point pt:roi_point){
        //左上に一番近いROI角を検出
        if(calcDis(pt, min_point) < min_dis){
            above_left = pt;
            min_dis = calcDis(pt, min_point);
        }
        //右下に一番近いROI角を検出
        if(calcDis(pt, max_point) < max_dis){
            below_right = pt;
            max_dis = calcDis(pt, max_point);
        }
    }
    
    //roiのrectを設定
    cv::Rect roi_rect(above_left.x, above_left.y, below_right.x-above_left.x, below_right.y-above_left.y);
    return(roi_rect);
}

//コピペ
double MainChecker::calcDis(cv::Point pt1, cv::Point pt2){
    return(sqrt(pow(pt1.x-pt2.x,2) + pow(pt1.y-pt2.y,2)));
}

//領域が中央なのかどうか
bool MainChecker::isCentered() {
    this->lavatory_img.release(); //まずは初期化 (不要かも)
    
    //エラー対応
    if(this->resized_gray_src_img.empty()){
        return false;
    }
    //グレースケール画像から洗面台の候補の抽出
    cv::Mat white_region_img; //白色領域の画像
    //二値化 (0, 1)
    cv::threshold(this->resized_gray_src_img, white_region_img, this->bin_threshold, 1, cv::THRESH_BINARY);
    
    //論理積
    cv::Mat and_img;
    cv::bitwise_and(this->resized_bin_img, white_region_img, and_img);
    cv::Mat enhanced_and_img; //二値画像の強調
    cv::threshold(and_img, enhanced_and_img, 0, 255, cv::THRESH_BINARY);
    
    cv::Mat element = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3));
    cv::Mat eroded_img; //収縮画像
    cv::Mat dilated_img; //膨張画像
    cv::dilate(enhanced_and_img, dilated_img, element, cv::Point(-1, -1), 2);//膨張処理
    cv::erode(dilated_img, eroded_img, element, cv::Point(-1, -1), 4);//収縮処理
    cv::dilate(eroded_img, dilated_img, element, cv::Point(-1, -1), 2); //膨張処理
    
    cv::Mat including_lavatory_img; //洗面台を含むフルサイズ画像
    //拡大
    cv::resize(dilated_img, including_lavatory_img, cv::Size(), 1. / SMALL_RATIO, 1. / SMALL_RATIO);
    
    //不適切領域の対応---------------------
    //----------------------------------
    
    //ここからコピペ detectRoiArea
    std::vector<std::vector<cv::Point>> contours;//輪郭点郡のvector
    std::vector<cv::Vec4i> hierarchy;//画像のトポロジーに関する情報を含む出力ベクトル
    //最も外側の輪郭のみを抽出
    cv::findContours(including_lavatory_img, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    int min_diff = including_lavatory_img.cols * including_lavatory_img.rows;//閾値との最小面積差の保存
    
    for(auto contour = contours.begin(); contour != contours.end(); contour++){
        std::vector<cv::Point> approx;//輪郭を近似した点郡列
        //輪郭(点郡)を直線近似する
        cv::approxPolyDP(cv::Mat(*contour), approx, 0.01 * cv::arcLength(*contour, true), true);
        //近似した点郡内の内包領域を算出
        double area = cv::contourArea(approx);
        if(abs(area - this->extraction_area_threshold) < min_diff){
            this->focus_contour = approx;//コピー
            min_diff = abs(area - this->extraction_area_threshold);//更新
        }
    }
    
    //重心の取得
    cv::Moments mu = cv::moments(this->focus_contour);
    this->area_center = cv::Point2f(mu.m10/mu.m00, mu.m01/mu.m00); //重心の保存
    std::cerr << this->area_center << std::endl;
    
    //this->debug_writing_file << "--isCentered" << std::endl;
    if (abs(this->src_img.rows/2 - this->area_center.y) < this->centering_y_threshold + 20) {
        //std::cerr << "中心です!!!!!!!!!!!!!!" << std::endl;
        return true; //中心
    }
    
    return false; //中心から離れている
    
}
//洗面台が存在するかどうか
std::pair<cv::Mat, cv::Mat> MainChecker::ErrorCheck(cv::Mat src_img, cv::Mat side_img, std::string kind_number){
    
    std::string kind_char = kind_number.c_str();
    current_type = kind_char;//現在の型として保存
    if(src_img.empty() || side_img.empty()){
        std::pair<cv::Mat, cv::Mat> tmp = std::make_pair(cv::Mat::zeros(30, 30, CV_8UC3), cv::Mat::zeros(30, 30, CV_8UC3));
    }
    //画像のコピー
    cv::Mat main_dst_img = src_img.clone();//pylon
    cv::Mat side_dst_img = side_img.clone();//側面
    
    //点滅用画像
    cv::Mat main_flash_img = src_img.clone();
    cv::Mat side_flash_img = src_img.clone();
    
    error_number.clear();
    
    std::string side_string = "4";
    std::pair<cv::Mat, cv::Mat> dst_image_pair;
    
    //エラーの種類に
    if (kind_char == "1"){
        
        error_number = checker1.checkLavatory(src_img);//エラーの取得
        main_dst_img = checker1.getRegistImage();//エラー矩形の表示された画像を取得
        main_flash_img = checker1.getFlashImage();//点滅演出用画像
        side_string = side_checker1.sideCheck(side_img);//側面のエラーの取得
        side_dst_img = side_checker1.getRegistImage();//エラー矩形の表示された画像を取得
        side_flash_img = side_checker1.getFlashImage();//点滅演出用画像
        result_rect_flag.clear();//表示用リストの初期化
        
        //エラー文言の結合
        error_number = joinErrorStrings(error_number, side_string);//文字列の結合
        dst_image_pair = std::make_pair(main_dst_img, side_dst_img);//エラー画像ペアの生成
        flash_images = std::make_pair(main_flash_img, side_flash_img);
        current_type = "1";
        
    }else if (kind_char == "2"){
        
        error_number = checker2.checkLavatory(src_img);//エラーの取得
        main_dst_img = checker2.getRegistImage();//エラー矩形の表示された画像を取得
        main_flash_img = checker2.getFlashImage();//点滅演出用画像
        side_string = side_checker2.sideCheck(side_img);//側面のエラーの取得
        side_dst_img = side_checker2.getRegistImage();//エラー矩形の表示された画像を取得
        side_flash_img = side_checker2.getFlashImage();//点滅演出用画像
        result_rect_flag.clear();//表示用リストの初期化
        
        //エラー文言の結合
        error_number = joinErrorStrings(error_number, side_string);//文字列の結合
        dst_image_pair = std::make_pair(main_dst_img, side_dst_img);//エラー画像ペアの生成
        flash_images = std::make_pair(main_flash_img, side_flash_img);
        current_type = "2";
        
    }else if (kind_char == "6"){
        
        error_number = checker6.checkLavatory(src_img);//エラーの取得
        main_dst_img = checker6.getRegistImage();//エラー矩形の表示された画像を取得
        main_flash_img = checker6.getFlashImage();//点滅演出用画像
        side_string = side_checker6.sideCheck(side_img);//側面のエラーの取得
        side_dst_img = side_checker6.getRegistImage();//エラー矩形の表示された画像を取得
        side_flash_img = side_checker6.getFlashImage();//点滅演出用画像
        result_rect_flag.clear();//表示用リストの初期化
        
        //エラー文言の結合
        error_number = joinErrorStrings(error_number, side_string);//文字列の結合
        dst_image_pair = std::make_pair(main_dst_img, side_dst_img);//エラー画像ペアの生成
        flash_images = std::make_pair(main_flash_img, side_flash_img);
        current_type = "6";
        
    }else if (kind_char == "8"){
        
        error_number = checker8.checkLavatory(src_img);//エラーの取得
        main_dst_img = checker8.getRegistImage();//エラー矩形の表示された画像を取得
        main_flash_img = checker8.getFlashImage();//点滅演出用画像
        side_string = side_checker8.sideCheck(side_img);//側面のエラーの取得
        side_dst_img = side_checker8.getRegistImage();//エラー矩形の表示された画像を取得
        side_flash_img = side_checker8.getFlashImage();//点滅演出用画像
        result_rect_flag.clear();//表示用リストの初期化
        
        //エラー文言の結合
        error_number = joinErrorStrings(error_number, side_string);//文字列の結合
        dst_image_pair = std::make_pair(main_dst_img, side_dst_img);//エラー画像ペアの生成
        flash_images = std::make_pair(main_flash_img, side_flash_img);
        current_type = "8";
        
    }else if (kind_char == "9"){
        
        error_number = checker9.checkLavatory(src_img);//エラーの取得
        main_dst_img = checker9.getRegistImage();//エラー矩形の表示された画像を取得
        main_flash_img = checker9.getFlashImage();//点滅演出用画像
        side_string = side_checker9.sideCheck(side_img);//側面のエラーの取得
        side_dst_img = side_checker9.getRegistImage();//エラー矩形の表示された画像を取得
        side_flash_img = side_checker9.getFlashImage();//点滅演出用画像
        result_rect_flag.clear();//表示用リストの初期化
        
        //エラー文言の結合
        error_number = joinErrorStrings(error_number, side_string);//文字列の結合
        dst_image_pair = std::make_pair(main_dst_img, side_dst_img);//エラー画像ペアの生成
        flash_images = std::make_pair(main_flash_img, side_flash_img);
        current_type = "9";
        
    }else if (kind_char == "3"){
        
        error_number = checker3.checkLavatory(src_img);//エラーの取得
        main_dst_img = checker3.getRegistImage();//エラー矩形の表示された画像を取得
        main_flash_img = checker3.getFlashImage();//点滅演出用画像
        side_string = side_checker3.sideCheck(side_img);//側面のエラーの取得
        side_dst_img = side_checker3.getRegistImage();//エラー矩形の表示された画像を取得
        side_flash_img = side_checker3.getFlashImage();//点滅演出用画像
        
        
        error_number = joinErrorStrings(error_number, side_string);//文字列の結合
        dst_image_pair = std::make_pair(main_dst_img, side_dst_img);//エラー画像ペアの生成
        flash_images = std::make_pair(main_flash_img, side_flash_img);
        current_type = "3";
        
    }else if (kind_char == "4"){
        
        error_number = checker4.checkLavatory(src_img);//エラーの取得
        main_dst_img = checker4.getRegistImage();//エラー矩形の表示された画像を取得
        main_flash_img = checker4.getFlashImage();//点滅演出用画像
        side_string = "0";//側面のエラーの取得
        side_dst_img = side_img.clone();//エラー矩形の表示された画像を取得
        side_flash_img = side_img.clone();//点滅演出用画像
        
        error_number = joinErrorStrings(error_number, side_string);//文字列の結合
        dst_image_pair = std::make_pair(main_dst_img, side_dst_img);//エラー画像ペアの生成
        flash_images = std::make_pair(main_flash_img, side_flash_img);
        current_type = "4";
        
    }else if (kind_char == "5"){
        
        error_number = checker5.checkLavatory(src_img);//エラーの取得
        main_dst_img = checker5.getRegistImage();//エラー矩形の表示された画像を取得
        main_flash_img = checker5.getFlashImage();//点滅演出用画像
        side_string = "0";//側面のエラーの取得
        side_dst_img = side_img.clone();//エラー矩形の表示された画像を取得
        side_flash_img = side_img.clone();//点滅演出用画像
        
        error_number = joinErrorStrings(error_number, side_string);//文字列の結合
        dst_image_pair = std::make_pair(main_dst_img, side_dst_img);//エラー画像ペアの生成
        flash_images = std::make_pair(main_flash_img, side_flash_img);
        current_type = "5";
        
    }else if (kind_char == "7"){
        
        error_number = checker7.checkLavatory(src_img);//エラーの取得
        main_dst_img = checker7.getRegistImage();//エラー矩形の表示された画像を取得
        main_flash_img = checker7.getFlashImage();//点滅演出用画像
        side_string = side_checker7.sideCheck(side_img);//側面のエラーの取得
        side_dst_img = side_checker7.getRegistImage();//エラー矩形の表示された画像を取得
        side_flash_img = side_checker7.getFlashImage();//点滅演出用画像
        
        error_number = joinErrorStrings(error_number, side_string);//文字列の結合
        dst_image_pair = std::make_pair(main_dst_img, side_dst_img);//エラー画像ペアの生成
        flash_images = std::make_pair(main_flash_img, side_flash_img);
        current_type = "7";
        
    }else {
        std::cerr << "Type is failed." << std::endl;
    }
    
    //点滅用画像の登録
    
    
    previous_type = kind_char;//ひとつ前としてコピ-
    //this->debug_writing_file << "--ErrorCheck" << std::endl;
    return (dst_image_pair);
}

//背景画像を確認する関数．なかったらtrue
bool MainChecker::bgImageEmpty() {
    if (this->bg_img.empty()) {
        return true;
    }
    
    return false;
}

std::string MainChecker::joinErrorStrings(std::string main_error, std::string side_error){
    
    std::string error;
    //メイン文字列の追加
    if(main_error.size() != 0){
        error = main_error;//メインエラーの追加
    }
    
    if(error == "0" && side_error != "0"){
        error = side_error;//サイドエラーで上書き
    }else if (error != "0" && side_error != "0"){
        error = error + "," + side_error;//結合
    }
    
    return (error);
}

//各エラー検出のon.offを切り替える
void MainChecker::setOnOffParam(){
    for(int num = 1; num <= 9; num++){
        std::ostringstream ss;
        ss << home_dir << "/switch/" << num << ".csv";
        readOnOffCsv(ss.str());
    }
}

void MainChecker::readOnOffCsv(std::string file_path){
    constexpr int COLUMNS = 2;
    
    std::string name;
    int mode;
    
    std::vector<std::pair<std::string, int>> param;
    //csvパラメータの読み込み
    if(std::__fs::filesystem::exists(file_path)){
        
        io::CSVReader<COLUMNS> in(file_path);
        in.read_header(io::ignore_extra_column, "name", "mode");
        
        while(in.read_row(name, mode)){
            param.push_back(std::make_pair(name, mode));//追加
        }
        
    }else{
        //万が一csvがなかった時
        for(int i=0; i<=20; i++){
            param.push_back(std::make_pair("emp", 1));
        }
        
    }
}

cv::Mat MainChecker::checkSorrounding(cv::Mat source_img){
    //エラー処理
    if(source_img.empty()){
        return cv::Mat(cv::Size(100, 100), CV_8UC3, cv::Scalar(0, 0, 0));
    }
    cv::Mat original = source_img.clone();
    
    cv::Mat gray_img, bin_img;
    cv::cvtColor(original, gray_img, cv::COLOR_BGR2GRAY);
    cv::threshold(gray_img, bin_img, 130, 255, cv::THRESH_BINARY);
    cv::bitwise_not(bin_img, bin_img);
    
    //横方向の余白をカットしていく
    cv::Rect roi_horizon = {0, 0, original.cols, 1};
    for(int i=0; i<200; i++){
        roi_horizon = {0, 0, original.cols, 1};
        cv::Mat tmp_img = bin_img(roi_horizon).clone();
        if(cv::countNonZero(tmp_img) < tmp_img.cols/2){
            break;
        }
        cv::Rect rect = {0, 1, original.cols, original.rows-1};
        rect = dealWithErrorFromRoi(rect, original.size());
        original = original(rect).clone();
        bin_img = bin_img(rect).clone();
    }
    
    //縦方向の余白をカットしていく
    cv::Rect roi_vertical = {0, 0, 1, original.rows};
    for(int i=0; i<200; i++){
        roi_vertical = {0, 0, 1, original.rows};
        cv::Mat tmp_img = bin_img(roi_vertical).clone();
        if(cv::countNonZero(tmp_img) < tmp_img.rows/2){
            break;
        }
        cv::Rect rect = {1, 0, original.cols-1, original.rows};
        rect = dealWithErrorFromRoi(rect, original.size());
        original = original(rect).clone();
        bin_img = bin_img(rect).clone();
    }
    
    cv::Mat rotate_img, M, rotate_M;
    cv::Point center = {original.cols/2, original.rows/2};
    cv::Rect roi_left_a = {0, 0, 5, original.rows/2};
    cv::Rect roi_left_b = {0, original.rows/2, 5, original.rows/2};
    
    double angle = 0.0;
    for(int i=0; i<=5; i++){
        M = cv::getRotationMatrix2D(center, angle, 1.0);
        cv::warpAffine(bin_img, bin_img, M, original.size(), cv::INTER_CUBIC);
        cv::Mat roi1_img = bin_img(roi_left_a).clone();//roi(上)
        cv::Mat roi2_img = bin_img(roi_left_b).clone();//roi(下)
        //        std::cout << cv::countNonZero(roi1_img) << ", " <<  cv::countNonZero(roi2_img) << std::endl;
        //上下黒画素差分を計測
        if(cv::countNonZero(roi1_img) < 1500 && cv::countNonZero(roi2_img) < 1500){
            rotate_M = M.clone();
            break;
        }
        
        if(cv::countNonZero(roi1_img) > cv::countNonZero(roi2_img)){
            angle += 0.3;//右回転
        }else{
            angle -= 0.3;//左回転
        }
    }
    
    if(!rotate_M.empty()){
        cv::warpAffine(original, original, rotate_M, original.size(), cv::INTER_CUBIC);
        cv::warpAffine(bin_img, bin_img, rotate_M, bin_img.size(), cv::INTER_CUBIC);
    }else{
        std::cout << "微調整なし" << std::endl;
    }
    
    //横方向の余白をカットしていく
    for(int i=0; i<100; i++){
        roi_horizon = {0, 0, original.cols, 1};
        cv::Mat tmp_img = bin_img(roi_horizon).clone();
        if(cv::countNonZero(tmp_img) < tmp_img.cols/2){
            break;
        }
        cv::Rect rect = {0, 1, original.cols, original.rows-1};
        rect = dealWithErrorFromRoi(rect, original.size());
        original = original(rect).clone();
        bin_img = bin_img(rect).clone();
    }
    
    //縦方向の余白をカットしていく
    for(int i=0; i<100; i++){
        roi_vertical = {0, 0, 1, original.rows};
        cv::Mat tmp_img = bin_img(roi_vertical).clone();
        if(cv::countNonZero(tmp_img) < tmp_img.rows/2){
            break;
        }
        cv::Rect rect = {1, 0, original.cols-1, original.rows};
        rect = dealWithErrorFromRoi(rect, original.size());
        original = original(rect).clone();
        bin_img = bin_img(rect).clone();
    }
    
    //縦方向(右)の余白をカットしていく
    cv::Rect roi_right = {original.cols, 0, 1, original.rows};
    for(int i=1; i<=100; i++){
        roi_right = {original.cols-i, 0, 1, original.rows};
        roi_right = dealWithErrorFromRoi(roi_right, original.size());
        cv::Mat tmp_img = bin_img(roi_right).clone();
        if(cv::countNonZero(tmp_img) < tmp_img.rows/2){
            break;
        }
        cv::Rect rect = {0, 0, original.cols-i, original.rows};
        rect = dealWithErrorFromRoi(rect, original.size());
        original = original(rect).clone();
        bin_img = bin_img(rect).clone();
    }
    
    //    cv::imshow("origin", original);
    //    cv::waitKey();
    
//        original = cv::imread("/Users/kato/Downloads/20210224データ抜き取り/20210212/NG/main_debug/20210212_114849.jpg", cv::IMREAD_COLOR);

    return (original);
}

bool MainChecker::judgeLavatory(cv::Mat src_img){
    cv::Mat gray_img, bin_img;
    cv::cvtColor(src_img, gray_img, cv::COLOR_BGR2GRAY);
    
    //判定
    if((int)cv::mean(gray_img)[0] < judge_threshold){
        return false;
    }else{
        return true;
    }
}

cv::Rect2d MainChecker::dealWithErrorFromRoi(cv::Rect2d roi_rect, cv::Size image_s){
    cv::Size image_size = image_s;//画像サイズを取得
    cv::Rect2d dst_rect = roi_rect;//コピー
    //xが0以下
    if(dst_rect.x < 0){
        std::cout << "書き換え1" << std::endl;
        dst_rect.x = 0;
    }
    //yが0以下
    if(dst_rect.y < 0){
        std::cout << "書き換え2" << std::endl;
        dst_rect.y = 0;
    }
    //幅が0以下の時
    if(dst_rect.width <= 0){
        std::cout << "書き換え3" << std::endl;
        dst_rect.width = image_size.width/2;
    }
    //高さが0以下の時
    if(dst_rect.height <= 0){
        std::cout << "書き換え4" << std::endl;
        dst_rect.height = image_size.height/2;
    }
    //幅が画面サイズ以上
    if(dst_rect.x + dst_rect.width > image_size.width){
        std::cout << "書き換え5" << std::endl;
        dst_rect.width = image_size.width - dst_rect.x;
    }
    //高さが画面サイズ以上
    if(dst_rect.y + dst_rect.height > image_size.height){
        std::cout << dst_rect.y << ", " << dst_rect.height << ", " << image_size.height << std::endl;
        std::cout << "書き換え6" << std::endl;
        dst_rect.height = image_size.height - dst_rect.y;
    }
    
    return (dst_rect);
}
