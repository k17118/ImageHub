const imgWidth = 250;
var imgHeight = 0;
var codeArray = {};//ソースコード格納用配列

class CodeToImage{

	static previewImage(obj)
	{

		const img = document.getElementById('preview');
		const reader = new FileReader();
		const imgReader = new Image();

		reader.onloadend = () => {
			imgReader.onload = () => {
				//文字列の切り出し
				const imgType = imgReader.src.substring(5, imgReader.src.indexOf(';'));
				imgHeight = Math.round(imgReader.height * (imgWidth / imgReader.width));//画像の高さを計算
				const canvas = document.createElement('canvas');
				canvas.width = imgWidth;
				canvas.height = imgHeight;
				const ctx = canvas.getContext('2d');
				ctx.drawImage(imgReader,0,0,imgWidth,imgHeight);

				img.src = canvas.toDataURL(imgType);

				//ソースコード読み込みボタンの表示
				const srcCode = document.getElementById("src-code");
				const inSrcCode = document.getElementById("code");
				if(srcCode.style.display!="block"){
					srcCode.style.display = "block";
					inSrcCode.style.display = "block";
				}
			}
			imgReader.src = reader.result;
		}
		reader.readAsDataURL(obj.files[0]);
	}

	//ソースコード取得用
	static previewCode(obj)
	{
		const inputCode = document.getElementById('code');// 要素を取得
		var file = inputCode.files[0];//ファイル読み込み(1番目のファイル)

		// FileReaderを生成
		var reader = new FileReader();

		// 読み込み完了時
		reader.onload = () => {
			var codeText = reader.result;
			codeText = codeText.replace(/\r?\n/g,"△");//改行を置換
			codeText = codeText.replace(/ /g,"×");//空白を置換

			//グローバル変数codeArray配列に置換結果を格納
			codeArray =  Array.from(codeText);
			//console.log(codeArray);

			//変換及びコピーボタンの表示
			const changeButton = document.getElementById("change");
			if(changeButton.style.display!="block"){
				changeButton.style.display = "block";
			}
		}

		reader.readAsText(file);//ファイル読み込み
	}

	static showTileText(){
		function productTileImage(width, height, pixel_data) {
			var tileString = "";
			var codeCount = 0; //コード配列カウント用配列

			const tileElement = document.querySelector('#tile');

			var alphabet = "abcdefghijklmnopqrstuvwxyz0123456789+-*";//文字数不足補間用
			var al = alphabet.length;

			for (let h=0; h<height; h++){
				for (let w=0; w<width; w++){

					let color = pixel_data[w + h*width];
					let r = color[0], g = color[1], b = color[2];
					if(codeArray[codeCount] == undefined){//ソースコードの長さが足りない時，別の文字で埋める
						var support_ch = alphabet[Math.floor(Math.random()*al)];
						tileString += `<span style=\"color:rgb(${r}, ${g}, ${b})\">` + support_ch + "</span>";
					}else{
						tileString += `<span style=\"color:rgb(${r}, ${g}, ${b})\">` + codeArray[codeCount] + "</span>";
					}
					codeCount++;//文字列カウント
				}
				tileString += "<br/>"
			}

			tileElement.innerHTML = tileString;
			//最終ボタンの位置を調整
			let imageSize = height * 5.0;
			//コンテンツの幅を指定（これがないと下の余白がバグ）
			document.getElementById('tile-image').style.height = `${imageSize}px`;

		}

		function getPixelData(src, dst, width, height) {
			var pixel_data = [];
			for (var i = 0; i < height; i++) {
				for (var j = 0; j < width; j++) {
					var idx = (j + i * width) * 4;
					var gray = (src[idx] + src[idx + 1] + src[idx + 2]) / 3;
					pixel_data.push([src[idx], src[idx+1], src[idx+2]]);
					dst[idx] = src[idx];
					dst[idx + 1] = src[idx+1];
					dst[idx + 2] = src[idx+2];
					dst[idx + 3] = src[idx+3];
				}
			}
			console.log(height)
			return (pixel_data);
		};

		const image = document.getElementById('preview');
		const canvas = document.createElement('canvas');
		canvas.width = imgWidth;
		canvas.height = imgHeight;
		const ctx = canvas.getContext('2d');
		ctx.drawImage(image,0,0,imgWidth,imgHeight);

		//画像処理 pixelデータの取得
		var srcData = ctx.getImageData(0, 0, imgWidth, imgHeight);
		var dstData = ctx.createImageData(imgWidth, imgHeight);
		var src = srcData.data;
		var dst = dstData.data;

		const pixel_data = getPixelData(src, dst, imgWidth, imgHeight);
		ctx.putImageData(dstData, 0, 0);
		productTileImage(imgWidth, imgHeight, pixel_data);

		// ボタンの表示
		const copyButton = document.getElementById("copy");

		if(copyButton.style.display!="block"){
			copyButton.style.display = "block";
		}

		//POST用に値をセッティング
		document.getElementById('image-w').value = imgWidth;
		document.getElementById('image-h').value = imgHeight;
		document.getElementById('image-code').value = codeArray;
		document.getElementById('image-color').value = pixel_data;
	}

	static copyProgram()
	{
		const programText = document.getElementById("tile").textContent;

		// テキストエリアを用意する
		var copyFrom = document.createElement("textarea");
		// テキストエリアへ値をセット
		copyFrom.textContent = programText;

		// bodyタグの要素を取得
		var bodyElm = document.getElementsByTagName("body")[0];
		// 子要素にテキストエリアを配置
		bodyElm.appendChild(copyFrom);

		// テキストエリアの値を選択
		copyFrom.select();
		// コピーコマンド発行
		var retVal = document.execCommand('copy');
		// 追加テキストエリアを削除
		bodyElm.removeChild(copyFrom);
		// 処理結果を返却
		return retVal;
	}

}