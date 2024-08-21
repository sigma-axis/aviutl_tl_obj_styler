# TLオブジェクト描画拡張

拡張編集のタイムラインの描画をカスタマイズできるプラグインです．[ダウンロードはこちら．](https://github.com/sigma-axis/aviutl_tl_obj_styler/releases)

![表示例](https://github.com/user-attachments/assets/4367bfe3-f2c9-4877-9996-5f4cfb406423)

蛇色様の[エディットボックス最適化プラグイン](https://github.com/hebiiro/AviUtl-Plugin-OptimizeEditBox)のライムライン描画機能や，[アルティメットプラグイン](https://github.com/hebiiro/anti.aviutl.ultimate.plugin)の[『拡張編集微調整』アドイン](https://github.com/hebiiro/anti.aviutl.ultimate.plugin/wiki/exedit_tweaker)を機能拡張したものです．これらのプラグイン/アドインとは競合するので，利用している場合は無効化してください．


## 動作要件

- AviUtl 1.10 + 拡張編集 0.92

  http://spring-fragrance.mints.ne.jp/aviutl
  - 拡張編集 0.93rc1 等の他バージョンでは動作しません．

- Visual C++ 再頒布可能パッケージ（\[2015/2017/2019/2022\] の x86 対応版が必要）

  https://learn.microsoft.com/ja-jp/cpp/windows/latest-supported-vc-redist

- **(推奨)** patch.aul

  https://github.com/nazonoSAUNA/patch.aul
  - タイムラインオブジェクトに表示されるフォントやグラデーションの色を変更できます．
  - patch.aul の設定による階段グラデーションの設定 (`patch.aul.json` の `"fast_exeditwindow"` 内の `"step"` の値) は反映されなくなります．代わりに[こちらのプラグインの設定](#gradient_back)で階段グラデーションを指定してください．

## 導入方法

以下のフォルダのいずれかに `tl_obj_styler.auf` と `tl_obj_styler.ini` をコピーしてください．

1. `aviutl.exe` のあるフォルダ
1. (1) のフォルダにある `plugins` フォルダ
1. (2) のフォルダにある任意のフォルダ


## 機能紹介

### 枠線を表示

![枠線の表示例](https://github.com/user-attachments/assets/83b57345-c432-421e-84ed-dc085653e27c)

オブジェクトに追加で枠線を表示できます．オブジェクトの選択中・非選択中に応じて，枠線の表示・非表示状態や太さなどを異なる設定に指定できます．

ついでに，既存の点線の枠線を非表示にすることもできます．

[[設定方法]](#borderselected--borderdeselected)

### 背景に画像を表示

![パターン画像の表示例](https://github.com/user-attachments/assets/8b8271a0-2d59-45b3-b740-fef09f212534) ![ハイライト画像の表示例](https://github.com/user-attachments/assets/04f6a9dc-7fe4-4545-99cf-f79c03e907de)

オブジェクトの背景に画像を半透明で表示できます．オブジェクトの選択中・非選択中に応じて，異なる画像や透明度などに指定できます．

[[設定方法]](#overlayselected--overlaydeselected)

### テキストのフォントを変更

![フォント変更の表示例](https://github.com/user-attachments/assets/559b697d-9411-4920-9999-834eafb909e9)

オブジェクトに表示されるテキストのフォントを変更できます．オブジェクトの選択中・非選択中に応じて異なるフォント名やフォントサイズを指定できます．

[[設定方法]](#title_font--title_fontselected)

### レイヤーの上下左右の枠線の色を変更

![レイヤー枠線の色変更](https://github.com/user-attachments/assets/fba2ff49-f3d0-45a7-b358-a0766ff7e318)

レイヤーの仕切り線の色を変更できます．

- 蛇色様の[エディットボックス最適化プラグイン](https://github.com/hebiiro/AviUtl-Plugin-OptimizeEditBox)のライムライン描画機能や，[アルティメットプラグイン](https://github.com/hebiiro/anti.aviutl.ultimate.plugin)の[『拡張編集微調整』アドイン](https://github.com/hebiiro/anti.aviutl.ultimate.plugin/wiki/exedit_tweaker)にある機能と同等です．
- この色変更は，ダークモード系プラグインを利用している場合は反映されません．
- 高 DPI 環境で「システム（拡張）」で実行している場合，現在フレームの赤線移動による「ゴミ」が残りにくくなります．

  赤線移動による「ゴミ」の例:

  ![高DPIで発生する「ゴミ」の例](https://github.com/user-attachments/assets/35fb5d4c-7ef0-4bcd-8d6d-0db5b47e0f77)

[[設定方法]](#layer_border)

### 時間範囲選択の色設定を変更

![時間範囲の表示色変更](https://github.com/user-attachments/assets/c7fe2b65-5516-4d33-bdbd-0e147342d3c6)

時間範囲選択時に表示されるマーカーの色を変更できます．

- 蛇色様の[エディットボックス最適化プラグイン](https://github.com/hebiiro/AviUtl-Plugin-OptimizeEditBox)のライムライン描画機能や，[アルティメットプラグイン](https://github.com/hebiiro/anti.aviutl.ultimate.plugin)の[『拡張編集微調整』アドイン](https://github.com/hebiiro/anti.aviutl.ultimate.plugin/wiki/exedit_tweaker)にある機能と同等です．
- この色変更は，ダークモード系プラグインを利用している場合は反映されません．

[[設定方法]](#time_selection)


## 設定方法

`tl_obj_styler.ini` ファイルをテキストエディタで編集して設定します．ファイル内のコメントにも説明を記述してあるため併せて参照してください．

### `[gradient_back]`

背景グラデーションの設定です．patch.aul による動作を上書きするため，patch.aul で階段グラデーションを設定していた場合などはこちらを同様の設定にしてください．

- `type` で通常のグラデーションか，階段グラデーションかを選びます．
  - `0` で通常のグラデーション，`1` で階段グラデーションになります．

- `steps` で階段グラデーションの段数を指定します．

初期状態は以下の通り:
```ini
[gradient_back]
type=0
steps=4
```
通常の（階段でない）グラデーションです．

- 階段グラデーションに関しては patch.aul のものと微妙に異なります．色の変化が少し緩やかになっている代わりに，中間点の境界が判別しやすくなっています．

  | このプラグイン | patch.aul |
  |:---:|:---:|
  |![TLオブジェクト描画拡張での階段グラデーション](https://github.com/user-attachments/assets/9e65762d-f8f8-4be4-b15e-80265b94291a)|![patch.aulによる階段グラデーション](https://github.com/user-attachments/assets/1398ffcf-845b-48f2-ae86-0733159f40ac)|


### `[overlay.selected]` / `[overlay.deselected]`

[背景に表示する画像](#背景に画像を表示)に関する設定です．`[overlay.selected]` で選択中オブジェクト，`[overlay.deselected]` で未選択オブジェクトの設定をします．

- `path` で画像ファイルへのパスを指定します．**日本語文字等が含まれている場合は `tl_obj_styler.ini` を UTF-8 で保存してください．**
- `alpha` で画像の不透明度 (`0` -- `255`) を指定します．
- `align_h` で画像の水平方向の配置を， `align_v` で垂直方向の配置を調整します．
  - `0` の場合（タイムラインの 0 フレーム / レイヤー 1 を基準）

    ![タイムラインの 0F/1L 基準の表示例](https://github.com/user-attachments/assets/078e798f-1ea9-4e3c-abcc-cc28587c59ef)

  - `1`, `2`, `3` の場合（ウィンドウの左上，中央，右下が基準; 画像は `2` の例）

    ![タイムラインの中央基準の表示例](https://github.com/user-attachments/assets/f156747e-e696-4f20-a1ff-98fecf3babdd)

  - `4`, `5`, `6` の場合（オブジェクトの左上，中央，右下が基準; 画像は `5` の例）

    ![オブジェクトの中央基準の表示例](https://github.com/user-attachments/assets/694c9d8d-4857-4af4-b717-6f7e3d2b1e3d)

- `margin` で画像の上下左右の余白を指定します．
  - `1,2,3,4` と指定して，「左は `1` ピクセル，上は `2` ピクセル，右は `3` ピクセル，下は `4` ピクセル」など上下左右個別に指定することもできます．

- `connect_midpoint` は `margin` で指定した左右の余白を，中間点の前後に適用するかどうかを指定します．
  - `0` 指定で余白を適用するようになり，中間点で画像に余白が空くようになります．
  - `1` 指定で余白を適用しないようになり，中間点で画像が途切れません．

初期状態は以下の通り:
```ini
[overlay.selected]
path=
alpha=0
align_h=0
align_v=0
margin=0
connect_midpoint=1

[overlay.deselected]
path=
alpha=0
align_h=0
align_v=0
margin=0
connect_midpoint=1
```
画像を表示しない設定です．

- `path` が空欄，または `alpha` が `0` の場合は画像を表示しません．


### `[border.selected]` / `[border.deselected]`

[追加で表示する枠線](#枠線を表示)に関する設定です．2重の枠線の色と太さ，余白を指定します．`[border.selected]` で選択中オブジェクト，`[border.deselected]` で未選択オブジェクトの設定をします．

- `outer.color` で外側の枠線の色を指定します．
- `outer.margin` で外側の枠線の余白を指定します．
  - [画像の余白](#overlayselected--overlaydeselected)設定の `margin` と同様に，上下左右個別に指定することもできます．

- `outer.thick` で外側の枠線の太さを指定します．
  - こちらも上下左右個別に指定できます．
- `inner.color` / `inner.margin` / `inner.thick` で内側の枠線の指定をします．
- `hide_dot_border` (`[border.selected]` のみ) で既存の点線の枠線を非表示にできます．

初期状態は以下の通り:
```ini
[border.selected]
outer.color=0xffffff
outer.margin=1
outer.thick=1
inner.color=-1
inner.margin=0
inner.thick=0
hide_dot_border=1

[border.deselected]
outer.color=-1
outer.margin=0
outer.thick=0
inner.color=-1
inner.margin=0
inner.thick=0
```
選択中オブジェクトは白い1ピクセルの枠線を外側から1ピクセルだけ空けて表示，未選択オブジェクトは枠線の表示なし．既存の点線は非表示に．


### `[title_font]` / `[title_font.selected]`

[オブジェクトに表示されるテキストのフォント](#テキストのフォントを変更)に関する設定です．`[title_font.selected]` で選択中オブジェクト，`[title_font]` で未選択オブジェクトの設定をします．`[title_font.selected]` で無指定だった場合，選択中・非選択中で同じフォント設定になります．

- `name` でフォント名を指定します．**日本語文字等が含まれている場合は `tl_obj_styler.ini` を UTF-8 で保存してください．**
- `size` でフォントサイズを指定します．

初期状態は以下の通り:
```ini
[title_font]
name=
size=0

[title_font.selected]
name=
size=0
```
フォントの変更なしの設定です．

- `name` が空欄，または `size` が `0` の場合はフォントを変更しません．
- 実行環境に依存するかもしれませんが，拡張編集のデフォルト状態は `name=Yu Gothic UI`, `size=-12` に相当します．


### `[layer_border]`

[レイヤーの仕切り線の色](#レイヤーの上下左右の枠線の色を変更)に関する設定です．

- `left` / `top` / `right` / `bottom` で上下左右の色を指定します．
  - `left` は `separator` に隠れて実質見えません．

- `separator` でレイヤーの左側，レイヤー名を表示したボタンとの境界線の色を指定します．

初期状態は以下の通り:
```ini
[layer_border]
left=-1
top=-1
right=-1
bottom=-1
separator=-1
```
色指定無しの設定です．

- 拡張編集の既定値は以下の設定に相当します:
  ```ini
  left=0x000000
  top=0x000000
  right=0xffffff
  bottom=0xffffff
  separator=0x000000
  ```

- 既定値と同じ色を指定しても実は無意味ではなくて，高 DPI 設定で「システム（拡張）」を選んでいる場合，現在フレームの赤線移動による「ゴミ」が残りにくくなります．ただし線は太く見えるようになります．
- ダークモード化プラグインを利用時は，この設定は反映されません．


### `[time_selection]`

[時間範囲を選択時のマーカーの色](#時間範囲選択の色設定を変更)に関する設定です．

- `tip` で選択範囲の左右端に表示される色を指定します．
- `interior` で選択範囲の内側の色を指定します．
- `exterior` で選択範囲の外側の色を指定します．

初期状態は以下の通り:
```ini
[time_selection]
tip=-1
interior=-1
exterior=-1
```
色指定無しの設定です．

- 拡張編集の既定値は以下の設定です:
  ```ini
  tip=0x6a7bfc
  interior=0x96b4fb
  exterior=0xcccccc
  ```

- ダークモード化プラグインを利用時は，この設定は反映されません．


## 既知の問題

1.  [フォントの設定](#title_font--title_fontselected)で選択中・非選択中で異なる設定をしていた場合，中間点の境界でフォントが切り替わって表示されます．この挙動を変更して「選択中の中間点区間がある場合は，選択中のフォント設定」にするためには，再描画するオブジェクトの選別手順を書き換えなければならず，想定よりも大幅なコード解析と改変が必要と判断したため保留にしています．

1.  [画像の表示](#overlayselected--overlaydeselected)や[枠線の表示](#borderselected--borderdeselected)での余白に負の値を指定することもできますが，その場合オブジェクトの表示領域を超えてしまって正しく表示されないことがあります．


## 改版履歴

- **v1.00** (2024-08-??)

  - 初版．

## ライセンス

このプログラムの利用・改変・再頒布等に関しては MIT ライセンスに従うものとします．

---

The MIT License (MIT)

Copyright (C) 2024 sigma-axis

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

https://mit-license.org/


#  Credits

##  aviutl_exedit_sdk

https://github.com/ePi5131/aviutl_exedit_sdk

---

1条項BSD

Copyright (c) 2022
ePi All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
THIS SOFTWARE IS PROVIDED BY ePi “AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL ePi BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

##  AviUtl プラグイン - エディットボックス最適化

https://github.com/hebiiro/AviUtl-Plugin-OptimizeEditBox

---

MIT License

Copyright (c) 2021 hebiiro

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.


#  連絡・バグ報告

- GitHub: https://github.com/sigma-axis
- Twitter: https://twitter.com/sigma_axis
- nicovideo: https://www.nicovideo.jp/user/51492481
- Misskey.io: https://misskey.io/@sigma_axis
- Bluesky: https://bsky.app/profile/sigma-axis.bsky.social
