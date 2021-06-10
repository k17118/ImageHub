const imgWidth = 90;
var imgHeight = 0;

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
				const srccode = document.getElementById("srccode");
				const insrccode = document.getElementById("insrccode");
				if(srccode.style.display!="block"){
					srccode.style.display = "block";
					insrccode.style.display = "block";
				}
			}
			imgReader.src = reader.result;
		}
		reader.readAsDataURL(obj.files[0]);
	}

	//ソースコード取得用
	static previewCode(obj)
	{
		const reader = new FileReader();
		const codeReader = new File();

		//ソースコードの表示(確認用)
		//code.src = canvas.toDataURL();

		reader.onloadend = () => {
			codeReader.onload = () => {
				//変換・コピーボタンの表示
				const changeButton = document.getElementById("change");
				//const copyButton = document.getElementById("copy");

				if(changeButton.style.display!="block"){
					changeButton.style.display = "block";
					//copyButton.style.display = "none";
				}
			}
			//codeReader.src = reader.result;
		}
		//reader.readAsDataURL(obj.files[0]);
	}

	static showTileText(){
		function productTileImage(width, height, pixel_data) {
			var tileString = "";
			const tileElement = document.querySelector('#tile');
			for (let h=0; h<height; h++){
				for (let w=0; w<width; w++){
					let color = pixel_data[w + h*width];
					let r = color[0], g = color[1], b = color[2];
					tileString += `<span style=\"color:rgb(${r}, ${g}, ${b})\">` + "あ" + "</span>";
					// console.log(color);
				}
				tileString += "<br/>"
			}
			tileElement.innerHTML = tileString;
			let imageSize = height * 2.5;
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