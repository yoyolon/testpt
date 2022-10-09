## ナイーブなパストレーシング

### 概要

オブジェクト指向プログラミングやCGの勉強のために[Raytracing in one weekend](https://raytracing.github.io/)のソースコードを参考にC++によるナイーブなパストレーサーを実装しました．実行すると三角ポリゴンと球で構成されたシーン(コーネルボックス)をレンダリングします．

### 開発環境 

Windows 10  
Microsoft Visual Studio 2019  
C++17

### 実行方法

testpt.exeを実行するかソースコードをビルドしてください．  
次のコマンドを実行すると，1ピクセルあたり1000サンプルのパストレーシングにより下図のような画像が生成されます．
```bash
testpt.exe 1000
```

<div align="center">
  <img src="imgs/img1.png" width=40% />
</div>

### 機能

Raytracing in one weekendにない機能として次の項目を実装しました．

- 三角ポリゴンとレイの交差判定
- Wavefront OBJ形式のモデルの読み込み
- マイクロファセットモデルや薄膜干渉のマテリアル
- 簡単なイメージベーストライティング

これらを用いると下図のような画像を生成することが可能です(別途，三次元形状モデルと環境マップが必要です)．


<div align="center">
  <img src="imgs/img2.png" width=40% />
  <img src="imgs/img3.png" width=40%/>
</div>

### その他

画像入出力に[stbライブラリ](https://github.com/nothings/stb)を使用しています．  
環境マップと三次元形状モデルは[Poly Heaven](https://polyhaven.com/)のものを使用しています．

### 今後の課題

- 変換行列の導入
- 光源を考慮した多重重点的サンプリングやBVHなどの空間データ構造による計算の高速化
- スペクトラルレンダリングへの対応
- フィルタ処理等によるデノイズ

### 参考文献

[MT97] T.Moller, B.Trumbore. "Fast, minimum storage ray-triangle intersection" 1997.  
[PR08] P.Shirley, R.K.Morley. "Realistic Ray Tracing" 2008.  
[PJH16] M.Pharr, W.Jakob, G.Humphre. "Physically Based Rendering: From Theory To Implementation" 2016.  
[Shi20] P.Shirley. "Raytracing in one weekend" 2020.  
