// var CanvasResize = function (canvas, width, height) {
// 	var img = new Image();
// 	img.onload = function () {
// 		// canvas.width = width;
// 		// canvas.height = height;
// 		// var ctx = canvas.getContext('2d');
// 		// ctx.drawImage(img, 0, 0, width, height);
// 	}
// 	img.src = canvas.toDataURL();
// }

// window.addEventListener("DOMContentLoaded", function() {
// 	var ofd = document.getElementById("selectfile");
// 	ofd.addEventListener("change", function(evt){
// 		//画像データの入力先
// 		var img = null
// 		var canvas = document.getElementById("canvas");

// 		var file = evt.target.files;
// 		var reader = new FileReader();

// 		//dataURL形式でファイルを読み込む
// 		reader.readAsDataURL(file[0]);

// 		//ファイルの読込が終了した時の処理
// 		reader.onloadend = function(){
// 			img = new Image();
// 			img.onload = function(){
// 				/*画像加工*/
// 				var context = canvas.getContext('2d');
// 				var width = img.width
// 				var height = img.height
// 				canvas.width = width;
// 				canvas.height = height;
// 				context.drawImage(img, 0, 0);
				
// 				CanvasResize(canvas, 300, 300)//リサイズ
// 			}
// 			img.src = reader.result
// 		}
// 	},false);
// });
// document.getElementById('button').onclick = () => {
// 	var text = document.getElementById('image_size').value;
// 	navigator.clipboard.writeText(text).then(e => {
// 		alert('コピーできました');
// 	});
// };

class CodeToImage{

	static previewImage(obj)
	{
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
			document.getElementById('tile-image').style.height = `${imageSize}px`;
			console.log(height);
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
			// console.log(width, height, pixel_data.length)
			return (pixel_data);
		};

		const img = document.getElementById('preview');
		const reader = new FileReader();
		const imgReader = new Image();
		const imgWidth = 300;

		reader.onloadend = () => {
			imgReader.onload = () => {
				//文字列の切り出し
				const imgType = imgReader.src.substring(5, imgReader.src.indexOf(';'));
				const imgHeight = Math.round(imgReader.height * (imgWidth / imgReader.width));//画像の高さを計算
				const canvas = document.createElement('canvas');
				canvas.width = imgWidth;
				canvas.height = imgHeight;
				const ctx = canvas.getContext('2d');
				ctx.drawImage(imgReader,0,0,imgWidth,imgHeight);

				//画像処理
				var srcData = ctx.getImageData(0, 0, imgWidth, imgHeight);
				var dstData = ctx.createImageData(imgWidth, imgHeight);
				var src = srcData.data;
				var dst = dstData.data;
				
				const pixel_data = getPixelData(src, dst, imgWidth, imgHeight);
				ctx.putImageData(dstData, 0, 0);
				
				img.src = canvas.toDataURL(imgType);
				productTileImage(imgWidth, imgHeight, pixel_data);
			}
			imgReader.src = reader.result;
		}
		reader.readAsDataURL(obj.files[0]);
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